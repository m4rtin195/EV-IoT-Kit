#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstdint>
#include <string>
#include <cmath>
#include <ctime>
#include <chrono>
#include <locale>
#include <thread>

#include "serialib.h"
#include "half.hpp"

using namespace std;
//using half_float::half;
using namespace half_float;
using namespace half_float::literal;

typedef uint8_t byte;

#define thrSleep(x) std::this_thread::sleep_for(std::chrono::seconds(x))
#define is_big_endian (*(uint16_t *)"\0\xff" < 0x100)

#define briefly true
#define OK 0
#define UNKNOWN_REPLY -3
#define NO_REPLY -4

#define DEMO 60 //consider seconds as minutes /or change to 0

#if defined (_WIN32) || defined(_WIN64)
    #define SERIAL_PORT "COM6"
#endif
#ifdef __linux__
    #define SERIAL_PORT "/dev/ttyS0"
#endif

#pragma GCC diagnostic ignored "-Wformat"

#define min_voltage 600.0
#define max_voltage 700.0
#define currentEq_param_a   0.119
#define currentEq_param_c   -0.785

/// global variables

enum Conf {Empty, Demo, Load};
enum VehicleType {Null, Fuel, Electric, Hybrid, AlternativeFuel};
enum State {Off, Charging, Idle, Driving};

struct Vehicle
{
    VehicleType vehicleType = Null;
    string vendor;
    string model;
    string VIN;
    string registrationPlate;
    uint16_t yearManufactured;
    uint16_t factoryCapacity;
} vehicle;

State state;    //vehicle state
float charge, target_charge, voltage; //in percent, volts
float current, max_current;
uint16_t elapsed_time, remaining_time; //in minutes, elapsed = since charging started/finished
//float battery_resistance, battery_temp;
//uint32_t actual_capacity;
uint16_t range;
float elec_consumption;
float indoor_temp;
//string coords = "";

clock_t chargingSStimestamp; //start/stop


// system variables

serialib serial;

auto brdcstInterval = 30 * (CLOCKS_PER_SEC/1000)*1000; //input in seconds
auto wdtTimeout = 5000 * (CLOCKS_PER_SEC/1000);        //input in miliseconds

clock_t wdtMain; //in ticks
clock_t wdtSim;
clock_t wdtBrdcst;

bool simulatorRunning;
bool broadcasterAllowed;


/// function declarations

string time(void);
int broadcast(void);
int resetAT(void);
float calcCharge(void);
void recalcOthers(void);
string minsToTime(uint16_t);
void report(bool);


/// /////////////////////
/// simulator functions

int initVehicle(Conf C = Empty)
{
    switch(C)
    {
        case Empty:
        {
            vehicle.vehicleType = Electric;
            vehicle.vendor = "";
            vehicle.model = "";
            vehicle.VIN = "";
            vehicle.registrationPlate = "";
            vehicle.yearManufactured = 0;
            vehicle.factoryCapacity = 0;
        } break;

        case Demo:
        {
            vehicle.vehicleType = Electric;
            vehicle.vendor = "Volkswagen";
            vehicle.model = "e-Golf";
            vehicle.VIN = "5GZCZ43D13S812715";
            vehicle.registrationPlate = "ZA-000AB";
            vehicle.yearManufactured = 2020;
            vehicle.factoryCapacity = 50000;
        } break;

        case Load: //TODO: from file
        {
            return -1;
            break;
        }

        default:
            exit(-1);
    }

    charge = 0.0;         //percent
    target_charge = 0.0;  //percent
    voltage = 0.0;        //volts
    current = 0.0;      //amps
    max_current = 0.0;  //amps

    elapsed_time = 0;  //minutes
    remaining_time = 0;  //minutes

    //battery_resistance = 0.0;  //ohms
    //battery_temp = 0.0;  //celsius
    //actual_capacity = 0;  //Wh

    range = 0;          //km
    elec_consumption = 0.0; //kWh/100km
    indoor_temp = 0.0;  //celsius

    return 0;
}

void sim_earlyValues(void)
{
    state = Idle;
    charge = 65.0;
    target_charge = 65.0;
    voltage = 600.0;
    current = 0.0;
    max_current = 300.0;
    //battery_resistance = 0.45;
    //battery_temp = 40.0;
    range = 325;
    elec_consumption = 21.0;
    indoor_temp = 20.5;

    cout << "[i] Early data set." << endl;
}

int setState(State s, float _current = 0, float _target_charge = 0)
{
    if((state!=Charging && s==Charging) || (state==Charging && s!=Charging)) //change to or from charging
        chargingSStimestamp = clock();

    if(state==Charging && s!=Charging)
        remaining_time = 0;

    if(_current <= 300)
        current = _current;
    else
    {
        current = 300;
        cout << "[!] Charging current bigger than curves defined. Using 300Amps." << endl;
    }

    if(_target_charge>0 && _target_charge<=100)
        target_charge = _target_charge;


    state = s;

    cout << endl << "[>] State: "; if(state==0) cout << "Off"; if(state==1) cout << "Charging"; if(state==2) cout << "Idle"; if(state==3) cout << "Driving";
        if(state==Charging || state==Driving) printf("  (with current: %.1fA", current); if(state==Charging) printf("  with target charge: %.0f%)", target_charge);
    cout <<  endl << endl;

    return 0;
}


void Simulator(void)
{
    cout << "[i] Simulator thread started." << endl;

    while(simulatorRunning == true)
    {
        wdtSim = clock();

        switch(state)
        {
            case Off:
            {
                continue;
            }

            case Charging:
            {
                if(current>max_current)
                {
                    cout << "[!] Current bigger than user max current, limiting." << endl;
                    current = max_current;
                }
                if(charge < target_charge)
                {
                    charge = calcCharge();
                }
                else
                {
                    report(briefly);
                    cout << endl << "[>] Charge completed (in " << minsToTime(elapsed_time) << ").";
                    setState(Idle);
                }
                break;
            }

            case Idle:
            {
                if(charge < target_charge)
                    setState(Charging, max_current);

                break;
            }

            case Driving:
            {
                break;
            }
        }

        recalcOthers();
        thrSleep(1);
    }
}

float calcCharge(void)
{
    //Charging speed:        (a)
    //  15A      4032 min    -0,0010     1.0
    //  -linear scale-
    //  300A     130 min     -0,0350     35

    if(charge>=100) return 100;

    float a;    // exponential eq parameter
    float x;    // "time" (position) in charging process
    float y;    // charge in %

    //solve (a) parameter for actual charging current
    a = -(currentEq_param_a * current + currentEq_param_c) / 1000; // y=ax+c
    if(a>0) return charge; //for too low current


    // find position in charging process for actual charge level
    x = (1/a) * log(-((charge-100) / 100));  //inverse func of next line  // x=(1/a)*log(-((y-100)/100))

    // calc new charge level for time+1 (second/minute)
    y = 100.0 * (1 - exp(a * (x + ((float)1/60*DEMO) )));   //charging exp curve  // y=100*(1-exp(a*x))

    if(y>99.5) y=100;  //end of exp func is very long so consider charging as complete
    if(y>target_charge) y=target_charge;

    float _target_charge = (target_charge>99.5) ? 99.5 : target_charge;
    float x_full = (1/a) * log(-((_target_charge-100-0.01) / 100));  //x at target_charge
    remaining_time = (x_full - x) /60 * DEMO;

    //printf("##: x= %f  y= %f  a=%f \n", x, y, a);
    return y;
}

void recalcOthers(void) //not related only to charging
{
    //voltage
    voltage = min_voltage + ((max_voltage-min_voltage)*charge/100.0);

    //elapsed time
    elapsed_time = (float)(clock()-chargingSStimestamp) / CLOCKS_PER_SEC /60 * DEMO;

    //range
    range = (vehicle.factoryCapacity/100)*(charge/100.0);
}

void report(bool brief = false)
{
    // TODO: logging to file

    if(brief)
    {
        //cout << setprecision(5) << "* " << time() << "\t" << voltage << "V    " << charge << "%  of  " << target_charge << "%    (" << minsToTime(elapsed_time) << " / " << minsToTime(remaining_time) << ") \t" << range << "km" << endl;
        printf("* %s     %.1fV     %.1f%% of %.0f%%     (%s / %s)  \t %dkm \n", time().c_str(), voltage, charge, target_charge, minsToTime(elapsed_time).c_str(), minsToTime(remaining_time).c_str(), range);
    }

    else
    {
        printf("\n");
        printf("Vehicle status report: \n");
        printf("  charge: %.1f% \t Target charge: %.0f% \n", charge, target_charge);
        printf("  Voltage: %.1fV \t Max voltage: %.0fV \n", voltage, max_voltage);
        printf("  Current: %.1fA \t Max current: %.0fA \n", current, max_current);
        printf("  Capacity: %dWh \t Max capacity: %dWh \t Factory capacity: %dWh \t Wear: %.1f% \n", 50000, 0/*actual_capacity*/, vehicle.factoryCapacity, 0/*(actual_capacity/factory_capacity)*/);
        printf("  Charging time: %s \t Remaining time: %s \n", "1h12m", "52m");                                          ///
        printf("  Battery resistance: %.2fohm \t Battery temperature: %.1f°C \n", 0/*battery_resistance*/, 0/*battery_temp*/);
        printf("  Outdoor temperature: %.1f°C \t Indoor temperature: %.1f°C \t Desired temperature: %.1f°C \n", 0/*outdoor_temp*/, indoor_temp, 0/*desired_temp*/);
        printf("  Fuel consumption: \t %.1fl/100km \n  Elec. consumption: \t %.1fkWh/100km \n", 0.0/*fuel_consumption*/, elec_consumption);
        printf("  Fuel amount: %.1fl \t Electric-Gas ratio: %s \t range: %dkm \n", 0/*fuel_weight*/, "1:0", range);    ///
        printf("  Vehicle location: %s \n", "0.0000 0.0000");
        printf("\n");
    }
}


void Broadcaster(void)
{
    cout << "[i] Broadcaster thread started." << endl;
    int st = 0;
    clock_t lastTime;

    while(broadcasterAllowed == true)
    {
        wdtBrdcst = clock();

        if(clock() > lastTime + brdcstInterval) //broadcast now
        {
            lastTime = clock();
            st = broadcast();
            if(st == 0)
                cout << "[>] Broadcasting message (at " << time() << ") successful" << endl;
            else
                cout << "[!] Broadcasting message (at " << time() << ") failed with code: " << st << endl;
        }

        thrSleep(1);
    }
}

// return: 0=success, 1=wrong val, 2=too big val, -1,-2=serial error, -3=unknown reply, -4=no reply
int broadcast(void)
{
    uint8_t payload[12];
    memset(payload, 0, 12);
    uint32_t p1=0, p2=0, p3=0;
    void* tmpptr = 0;
    int status = INT_MAX;

    //check values are in processable bounds
    if(state > 3) return 1;
    if(charge > 100) return 1;
    if(target_charge > 100.0) return 1;
    if(current > 1023.0) return 2;
    if(elapsed_time > 8191) return 2;  //5.68 days
    if(remaining_time > 8191) return 2;
    if(range > 2047) return 2;
    if(elec_consumption);   //no need to check
    if(indoor_temp);        //no need to check

    /// build frame
    //state
    uint8_t _state = state;
    _state &= 0x0007;
    p1 |= _state << 29;

    //current charge
    uint8_t _charge = static_cast<uint8_t>(charge);
    _charge &= 0x007F;
    p1 |= _charge << 22;

    //target charge
    uint8_t _target_charge = static_cast<uint8_t>(target_charge);
    _target_charge &= 0x007F;
    p1 |= _target_charge << 15;

    //current
    uint16_t _current = static_cast<uint16_t>(current);
    _current &= 0x03FF;
    p1 |= _current << 5;

    //elapsed time
    uint16_t _elapsed_time = elapsed_time;
    _elapsed_time &= 0x1FFF;
    p1 |= (_elapsed_time >> 8) << 0;
    p2 |= (_elapsed_time & 0xFF) << 24;

    //remaining time
    uint16_t _remaining_time = remaining_time;
    _remaining_time &= 0x1FFF;
    p2 |= _remaining_time << 11;

    //range
    uint16_t _range = range;
    _range &= 0x07FF;
    p2 |= _range << 0;

    //consumption
    half _consumption = half_cast<half>(elec_consumption);
    tmpptr = &_consumption;
    uint16_t _consumption_i = *(uint16_t*)tmpptr;
    p3 |= (_consumption_i & 0xFFFF) << 16;

    //temperature
    half _indoor_temp = half_cast<half>(indoor_temp);
    tmpptr = &_indoor_temp;
    uint16_t _indoor_temp_i = *(uint16_t*)tmpptr;
    p3 |= (_indoor_temp_i & 0xFFFF) << 0;


    //swap byte order
    if(!is_big_endian)
    {
        p1 = __builtin_bswap32(p1);
        p2 = __builtin_bswap32(p2);
        p3 = __builtin_bswap32(p3);
    }

    memcpy(payload+0,&p1,4);
    memcpy(payload+4,&p2,4);
    memcpy(payload+8,&p3,4);

    char ATcommand[19] = "AT$SF=";
    memcpy(&ATcommand[6],&payload,12);
    ATcommand[18] = '\n';

    /*
    //debug
    printf("\n data: ");
    for(int i=0; i<12; i++)
        printf("%2d: 0x%X \n", i+1, payload[i]);

    printf("\n command: ");
    for(int i=0; i<19; i++)
        printf("%X ", ATcommand[i]);
    cout << endl;
    */

    serial.flushReceiver();
    status = serial.writeBytes(ATcommand,19);
    if(status!=1) return status;

    char answ[20];
    int val = serial.readString(answ,'\n',20,8000);
    if(val>0)
    {
        if(strncmp(answ,"OK\n",3)==0)
            status = OK;
        else
        {
            cout << "[!] Unknown AT reply (n=" << val << "): " << answ << endl;
            resetAT() ? printf("[!] Cannot reset AT. \n") : 0;
            status = UNKNOWN_REPLY;
        }
    }
    else //val<0
    {
        if(val==0) status = NO_REPLY;
        else status = val;
    }

    return status;
}


/// //////////////////
/// system functions

void Watchdog(void)
{
    cout << "[i] Watchdog started." << endl;

    while(true)
    {
        if(clock() > wdtMain+wdtTimeout)
        {
            cout << endl << "[X] MAIN thread watchdog expired. Exit()..." << endl;
            exit(10);
        }
        if((clock() > wdtSim+wdtTimeout) && simulatorRunning)
        {
            cout << endl << "[X] Simulator thread watchdog expired. Exit()..." << endl;
            exit(11);
        }
        if((clock() > wdtBrdcst+wdtTimeout*3) && broadcasterAllowed)
        {
            cout << endl << "[X] Broadcaster thread watchdog expired. Exit()..." << endl;
            exit(12);
        }
    }
}

void atExitFunc()
{
    serial.closeDevice();
    cout << "/////";
    //resetUnit();
}

void resetUnit()
{
// TODO (Martin#1#): resetUnit
    putchar('\a');
    ;
}


string time(void)
{
    time_t clock = chrono::system_clock::to_time_t(chrono::system_clock::now());
    struct tm* system_time = localtime(&clock);

    char buff[16];
    snprintf(buff, sizeof(buff), "%02d:%02d:%02d", system_time->tm_hour, system_time->tm_min, system_time->tm_sec);

    return buff;
}

string minsToTime(uint16_t mins)
{
    uint8_t days=0, hours=0, minutes=0;
    days = mins/1440; mins -= days*1440;
    hours = mins/60; mins -= hours*60;
    minutes = mins;

    string result = (days ? (to_string(days)+" days, ") : "") + (hours ? (to_string(hours)+" hours, ") : "") + to_string(minutes)+" mins";
    return result;
}

int resetAT(void)
{
    char answ[20];
    serial.flushReceiver();
    serial.writeString("AT\n");
    serial.writeString("AT\n");

    int val = serial.readString(answ,'\n',6,2000);
    if(val==0) return NO_REPLY;
    if(strncmp(answ,"OK\n",3)==0 || strncmp(answ,"OK\nOK\n",6)==0) return OK;
    return val;
}


/// //////////////////////////////////////////////////////////////////////////////////////////////////////
/// main

int main()
{
    cout << "*init*" << endl;
    std::this_thread::sleep_for(500ms);

    setlocale(LC_ALL, "");  //funguje na linuxe?
    atexit(atExitFunc);
    wdtMain = 0, wdtSim = 0, wdtBrdcst = 0;
    simulatorRunning = false;
    broadcasterAllowed = false;
    chargingSStimestamp = 0;
    int i=0;
    char c=0;


    /// init setial port
    serial.openDevice(SERIAL_PORT, 9600) ? 0 : printf("[!] Serial port initialization failed. \n");

    /// init vehicle data
    initVehicle(Demo) ? printf("[!] Vehicle initialization failed. \n") : 0;

    /// init modem
    resetAT() ? printf("[!] Modem not ready. \n") : 0;

    printf("\nVehicle:\n  %s %s\n  %s\n  %s\n  %d\n  %dkWh\n\n", vehicle.vendor.c_str(), vehicle.model.c_str(), vehicle.VIN.c_str(), vehicle.registrationPlate.c_str(), vehicle.yearManufactured, vehicle.factoryCapacity/1000);

    sim_earlyValues();

    std::thread thread_simulator(Simulator);
    std::thread thread_broadcaster(Broadcaster);
    std::thread thread_watchdog(Watchdog);
    //std::this_thread::sleep_for(1ms); // BUG: sposobuje simulator watchdog exp
    for(int i=0;i<100000;i++) i=i;

    simulatorRunning = true;   //StartSimulator()
    broadcasterAllowed = true;

    setState(Charging, 300, 80);

    std::this_thread::sleep_for(500ms);

    cout << endl << "[i] Entering main loop." << endl << endl;
    printf("--- time ----- voltage ----- charge ------ elapsed | remaining ---------------- range --- \n");
    //      * 03:05:23     665,3V     65,3% of 80%     (0 mins / 1 hours, 47 mins)          326km

    while(true)
    {
        /*
        //ovladanie simulatora
        if(c=_getch())
        {
            printf("%d", c);
            if(c=='a') printf("aaaa");
            if(c=='b') printf("bbbb");
        }
        */

        if(!(i%5)) report(briefly);

        i++;
        wdtMain = clock();
        thrSleep(1);
    }

    return 0;
}

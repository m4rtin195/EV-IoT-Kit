#include <iostream>
#include <cstdlib>
#include <string>
#include <ratio>
#include <thread>
#include <cmath>
#include <ctime>
#include <chrono>
#include <locale>
//#include <Windows.h> /// !!!
#include <conio.h>

using namespace std;

#define csleep(x) std::this_thread::sleep_for(std::chrono::seconds(x))

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

State state;
float charged, target_charge;
float voltage, max_voltage, current, max_current;
uint16_t charging_time, remaining_time;
float battery_resistance, battery_temp;
uint32_t factory_capacity, actual_capacity;
float outdoor_temp, indoor_temp, desired_temp;
uint16_t approach;
float fuel_weight, fuel_consumption, elec_consumption, elgas_ratio;
string coords = "";

/// system variables

struct tm* system_time;
clock_t wdtMain; //in ms
clock_t wdtSim;

bool simRunning;

////////

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

        case Load:
            return -1;
            break;
    }

    charged = 0;        //percent
    target_charge = 0;  //percent

    voltage = 0;        //volts
    max_voltage = 0;    //volts, desired or batt max?
    current = 0.0;      //amps
    max_current = 0.0;  //amps

    charging_time = 0;  //minutes
    remaining_time = 0; //minutes

    battery_resistance = 0.0;   //ohms
    battery_temp = 0.0; //celsius

    //factory_capacity = 0; //Wh
    actual_capacity = 0; ///ako zistit?

    outdoor_temp = 0.0; //celsius
    indoor_temp = 0.0;
    desired_temp = 0.0;

    approach = 0;       //km
    fuel_weight = 0.0;  //kilograms
    fuel_consumption = 0.0; //liters/100km
    elec_consumption = 0.0; //kWh/100km
    elgas_ratio = 0;    ///x:y ???

    return 0;
}


void setPreferencies()
{
    target_charge = 100;
    max_voltage = 700.0;
    max_current = 50.0;
    desired_temp = 20.0;    ///default?

    cout << "[i] User preferencies set up." << endl;
}

void sim_EarlyData()
{
    charged = 65.0;
    voltage = 455.5;
    current = 0.0;
    battery_resistance = 0.45;
    battery_temp = 40.0;
    outdoor_temp = 10.5;
    indoor_temp = 10.5;
    fuel_weight = 40.3;
    approach = 870;
    fuel_consumption = 4.4;
    elec_consumption = 21.0;
    elgas_ratio = 0;
    desired_temp = indoor_temp;

    state = Idle;
    cout << "[i] Early data set." << endl;
}

uint16_t calcActualCapacity()
{
    return 0;
}


void Watchdog(void)
{
    while(true)
    {
        if(clock() > wdtMain+5000)
        {
            cout << endl << "[!] Main thread watchdog expired. System stop." << endl;
            exit(-10);
        }
        if((clock() > wdtSim+5000) && simRunning)
        {
            cout << endl << "[!] Simulator thread watchdog expired. System stop." << endl;
            cout << clock() << " vs " << wdtSim ;
            exit(-10);
        }
    }
}

void atExitFunc()
{
    cout << "//";
}


void getClock(void)
{
    time_t clock = chrono::system_clock::to_time_t(chrono::system_clock::now());
    system_time = localtime(&clock);
    return;
}

string frmtime()
{
    char buff[16];
    snprintf(buff, sizeof(buff), "%02d:%02d:%02d", system_time->tm_hour, system_time->tm_min, system_time->tm_sec);

    return buff;
}

void report(bool brief = false)
{
    if(brief)
    {
        cout << "* " << frmtime() << "\t" << voltage << "V    " << charged << "%  of  " << target_charge << "%    (" << charging_time << " / " << remaining_time << ") \t" << approach << "km" << endl;  /// time to string

    }

    else
    {
        printf("\n");
        printf("Vehicle status report: \n");
        printf(" Charged: %.1f% \t Target charge: %.0f% \n", charged, target_charge);
        printf(" Voltage: %.1fV \t Max voltage: %.0fV \n", voltage, max_voltage);
        printf(" Current: %.1fA \t Max current: %.0fA \n", current, max_current);
        printf(" Capacity: %dWh \t Max capacity: %dWh \t Factory capacity: %dWh \t Wear: %.1f% \n", 5000, actual_capacity, factory_capacity, 0/*(actual_capacity/factory_capacity)*/);
        printf(" Charging time: %s \t Remaining time: %s \n", "1h12m", "52m");                                          ///
        printf(" Battery resistance: %.2fohm \t Battery temperature: %.1f°C \n", battery_resistance, battery_temp);
        printf(" Outdoor temperature: %.1f°C \t Indoor temperature: %.1f°C \t Desired temperature: %.1f°C \n", outdoor_temp, indoor_temp, desired_temp);
        printf(" Fuel consumption: \t %.1fl/100km \n Elec. consumption: \t %.1fkWh/100km \n", fuel_consumption, elec_consumption);
        printf(" Fuel amount: %.1fl \t Electric-Gas ratio: %s \t Approach: %dkm \n", fuel_weight, "1:1", approach);    ///
        printf(" Vehicle location: %s \n", "0.00 0.00");
        printf("\n");
    }
    return;
}

int setState(State s, float l_current = 0, float l_target_charge = 0)
{
    state = s;
    if(l_current <= 300) current = l_current; else return 1;
    if(l_target_charge!=0) target_charge = l_target_charge;

    cout << endl << "[>] State: "; if(state==0) cout << "Off"; if(state==1) cout << "Charging"; if(state==2) cout << "Idle"; if(state==3) cout << "Driving";
        if(state==Charging || state==Driving) printf("  (with current: %.1fA", current); if(state==Charging) printf("  with target charge: %.0f%)", target_charge);
    cout <<  endl << endl;

    return 0;
}

float calcCharge(void)
{
    return charged + 1.0;

    /// y = ax

    // 300A     130 min     -0,0350
    // 80A      1260 min    -0.0035
    // 15A      4032 min    -0,0010
    /*
    c = 150; 150-80 = 70
    tmp = 300-80 = 220
    70/220 = 0,32
    0,32*???
    */

    /// solve a!!!

    float a = -0.035;
    float x = 20.0;
    float y;

    y = 100.0 * (1 - exp(a * x));

    x = (1/a) * log(-((y-100) / 100));

    printf("\n result: y= %f x= %f \n", y, x);

    return a;
}

void Simulator(void)
{

    simRunning = true;
    while(true)
    {
        if(state == Off)
            ;

        if(state == Charging)
        {
            if(charged < target_charge)
            {
                charged = calcCharge();
                //charged++;
                //cout << "ch++" << endl;
            }
            else
            {
                cout << endl << "[>] Charge completed (in " << charging_time << ")";
                setState(Idle);
            }
        }

        if(state == Idle)
        {
            if(charged < target_charge)
                setState(Charging, max_current);
        }

        if(state == Driving)
        {
            ;
        }



        wdtSim = clock();
        csleep(1);
    }
}

/// /////////////////////////////////////////////////////////////////////////////////////////////////// ///

int main()
{
    cout << "*init \n";
    std::this_thread::sleep_for(500ms);

    //calcCharge();
    //return 0;

    setlocale(LC_ALL, "");
    atexit(atExitFunc);
    wdtMain = 0, wdtSim = 0;
    simRunning = false;
    int i=0;
    char c=0;

    //system_time = chrono::system_clock::to_time_t(chrono::system_clock::now());
    //cout << ctime(&system_clock) << endl;

    cout << "[i] cpu_cores: " << std::thread::hardware_concurrency() << endl;;

    initVehicle(Demo) ? printf("[!] Vehicle initialisation failed. \n") : 0;

    printf("\n");
    printf(" %s \n %s \n %s \n %s \n %d \n\n", vehicle.vendor.c_str(), vehicle.model.c_str(), vehicle.VIN.c_str(), vehicle.registrationPlate.c_str(), vehicle.yearManufactured);

    setPreferencies();
    sim_EarlyData();


    std::thread thread_watchdog(Watchdog);
    std::thread thread_simulator(Simulator);    //StartSimulator()


    state = Charging;
    setState(Charging, 300, 80);

    cout << "[i] Entering main loop." << endl << endl;
    while(1)
    {
        getClock();
        //cout << "aa" << endl;
/*
        if(c=_getch())
        {
            printf("%d", c);
            if(c=='a') printf("aaaa");
            if(c=='b') printf("bbbb");
        }
*/

        if(!(i%5)) report();

        csleep(1);
        i++;
        wdtMain = clock();
    }

    return 0;
}

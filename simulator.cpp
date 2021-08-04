#include "simulator.h"

/** Simulator bezi vo vlastnom vlakne, co sice nieje nutne lebo qt je event-driven, ma event-loop.
    toto vlakno ma ale teda samostatnu event-loop, do ktorej vklada eventy QTimer. event = zavolanie metody simulate() */

Simulator::Simulator(Vehicle* vehicle) : QThread()
{
    v = vehicle;

    tickTimer = new QTimer(this);
    connect(tickTimer, SIGNAL(timeout()), this, SLOT(_simulate()));

    SStimestamp.start();
    this->QThread::start();
}

Simulator::~Simulator()
{
    this->quit();
    this->wait();
}

void Simulator::enabled(bool b)
{
    if(b)
        tickTimer->start(SIMULATION_INTERVAL);
    else
        tickTimer->stop();
}

void Simulator::setInitialValues()
{
    v->state = Vehicle::Off;
    v->charge = 40.0;
    v->target_charge = 65.0;
    v->voltage = 600.0;
    v->current = 0.0;
    v->max_current = 300.0;
    v->elapsed_time = 0;
    v->remaining_time = 0;
    v->range = 325;
    v->elec_consumption = 21.0;
    v->indoor_temp = 20.5;

    v->outdoor_temp = 30.0;
    v->desired_temp = v->indoor_temp;
    v->location = "placeholder location";

    //emit logRequest(extended);
    cout << "[i] Initial values set." << endl;
}

/// override
void Simulator::run()
{
    cout << "[i] Simulator thread started." << endl;
    this->exec(); //run the event loop
}

/// tick
void Simulator::_simulate()
{
    switch(v->state)
    {
        case Vehicle::Off:
        {
            break;
        }

        case Vehicle::Charging:
        {
            if((v->current) > (v->max_current))
            {
                cout << "[!] Current bigger than user-set maximal current, limiting." << endl;
                v->current = v->max_current;
                emit redrawRequest();
            }
            if((v->charge) < (v->target_charge)) //standard condition, charging
            {
                v->charge = _calcCharge();
            }
            else
            {
                emit logRequest(briefly, priority_now);
                cout << endl << "[>] Charge completed (in " << Logger::minsToTime(v->elapsed_time) << ").";
                setState(Vehicle::Idle);
            }
            break;
        }

        case Vehicle::Idle:
        {
            //break;
            return;
        }

        case Vehicle::Driving:
        {
            break;
        }
    }

    _recalcOthers();

    emit redrawRequest();
    emit logRequest(briefly);
}

//return: 0=changed, 1=same
int Simulator::setState(Vehicle::State newState, float _current, float _target_charge)
{
    if(newState == v->state) return 1;
    //if(((v->state != Vehicle::Charging) && (newState == Vehicle::Charging)) || ((v->state == Vehicle::Charging) && (newState != Vehicle::Charging))) //change to or from charging wtf naco

    SStimestamp.restart();

    if(/*(v->state == Vehicle::Charging) &&*/ newState != Vehicle::Charging)
        v->remaining_time = 0;

    if((newState == Vehicle::Charging) || (newState == Vehicle::Driving))
    {
        if(_current <= 300) //zaporny pri jazde
            v->current = _current;
        else
        {
            v->current = 300;
            cout << "[!] Charging current out of defined range. Using 300Amps." << endl;
        }
    }
    else //off/idle
        v->current = 0;

    if(_target_charge>=0 && _target_charge<=100)
        v->target_charge = _target_charge;

    v->state = newState;
    this->_simulate();
    _recalcOthers();
    emit redrawRequest();


    // vypis //
    cout << endl << "[>] State: ";
    switch(v->state)
    {
        case Vehicle::Off:
            cout << "Off";
            break;

        case Vehicle::Charging:
            cout << QString("Charging (with current: %1A, to target charge: %2%)")
                    .arg(QString::number(_current, 'f', 1), QString::number(_target_charge, 'f', 0))
                    .toStdString();
            break;

        case Vehicle::Idle:
            cout << "Idle";
            break;

        case Vehicle::Driving:
            cout << QString("Driving (with current: %1A)")
                    .arg(QString::number(_current, 'f', 1))
                    .toStdString();
            break;

        default:
            cout << "wtf??";
    }

    if(v->state == Vehicle::Driving || v->state == Vehicle::Off) cout << endl << "[!] Not implemented.";

    cout << endl << endl;

    emit logRequest(briefly, priority_now);
    emit broadcastRequest(priority_now);
    return 0;
}

void Simulator::setCharge(float level)
{
    v->charge = level;
    this->_simulate();
}

void Simulator::setCurrent(float current)
{
    //WARNING mutexy!!!
    v->current = current;
    emit redrawRequest();
}

float Simulator::_calcCharge(void)
{
    //shadows
    float charge = v->charge;
    float target_charge = v->target_charge;
    float current = v->current;

    //  Charging speed:        (a)
    //  15A      4032 min    -0,0010     1.0
    //  - linear scale -
    //  300A     130 min     -0,0350     35

    //if(charge >= 100) return 100; //treba??

    float a;    // exponential eq parameter
    float x;    // "time" (position) in charging process
    float y;    // charge in %

    //solve (a) parameter for actual charging current
    a = -(currentEq_param_a * current + currentEq_param_c) / 1000; // y=ax+c
    if(a>0) return charge; //for too low current


    // find position in charging process for actual charge level
    x = (1/a) * log(-((charge-100) / 100));  //inverse func of next line  // x=(1/a)*log(-((y-100)/100))

    // calc new charge level for time+1 (second/minute)
    y = 100.0 * (1 - exp(a * (x + ((float)1/60*TIME_SPEED))));   //charging exp curve  // y=100*(1-exp(a*x))

    if(y>99.5) y=100;  //end of exp func is very long so consider charging as complete
    if(y>target_charge) y=target_charge;

    float _target_charge = (target_charge>99.5) ? 99.5 : target_charge;
    float x_full = (1/a) * log(-((_target_charge-100-0.01) / 100));  //x at target_charge
    v->remaining_time = (x_full - x);

    //printf("##: x= %f  xt = %f  y= %f  a=%f \n", x, x_full, y, a);
    return y;
}

void Simulator::_recalcOthers() //not related only to charging
{
    v->voltage = v->min_voltage + ((v->max_voltage - v->min_voltage) * v->charge/100.0);
    //v->elapsed_time = (float)(clock()-chargingSStimestamp) / CLOCKS_PER_SEC /60 * TIME_SPEED;
    v->elapsed_time = (float)(SStimestamp.elapsed())/1000/60 * TIME_SPEED; //result in minutes
    v->range = (v->battery_capacity/100)*(v->charge/100.0); //* vehicle_effectivity;
}

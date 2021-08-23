#include "simulator.h"

/** Event-loop Simulatora do ktorej vklada eventy QTimer (zavolanie metody simulate()) bezi vo vlastnom vlakne */

static float a;    // exponential eq parameter
static float x;    // "time" (position) in charging process
static float y;    // charge in %

Simulator::Simulator(Vehicle* vehicle) : QObject()
{
    v = vehicle;

    tickTimer = new QTimer(this);
    connect(tickTimer, SIGNAL(timeout()), this, SLOT(_simulate()));
    connect(this, SIGNAL(_simulateRequest()), this, SLOT(_simulate()), Qt::BlockingQueuedConnection);

    SStimestamp.start();
}

Simulator::~Simulator()
{
    tickTimer->stop();
}

// starts or stops timer which invokes _simulate()
void Simulator::enable(bool b)
{
    if(b)
        tickTimer->start(SIMULATION_INTERVAL);
    else
        tickTimer->stop();
}

bool Simulator::isEnabled()
{
    return tickTimer->isActive();
}


void Simulator::setInitialValues()
{
    v->readwriteLock->lockForWrite();

    v->state = Vehicle::Off;
    v->charge = 40.0;
    v->target_charge = 0;
    v->voltage = 580.0;
    v->current = 0;
    v->max_current = 300.0;
    v->elapsed_time = 0;
    v->remaining_time = 0;
    v->range = 320;
    v->elec_consumption = 21.0;
    v->indoor_temp = 20.5;

    v->outdoor_temp = 30.0;
    v->desired_temp = v->indoor_temp;
    v->location = "placeholder location";

    v->readwriteLock->unlock();
    //emit logRequest(extended);
    cout << "[i] Initial values set." << endl;
}


/// tick
/// simulate new values
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
                v->readwriteLock->lockForWrite();
                v->current = v->max_current;
                v->readwriteLock->unlock();
                emit redrawRequest();
            }
            if((v->charge) < (v->target_charge)) //standard condition, charging
            {
                _calcCharge();
                _calcOthers();
                emit logRequest(briefly);
            }
            else //charge > target_charge
            {
                emit logRequest(briefly, priority_now);
                cout << endl << "[>] Charge completed (in " << Logger::minsToTime(v->elapsed_time) << ").";
                setState(Vehicle::Idle);
            }

            break;
        }

        case Vehicle::Idle:
        {
            //break; //TODO
            return;
        }

        case Vehicle::Driving:
        {
            _calcOthers(); //kvoli elapsed_time
            emit logRequest(briefly);
            break;
        }
    }

    emit redrawRequest();
}

//public, call internal one trouch signal to separate trheads (calls from ui)
//separate method instead of making _setState as a public slot is becuase it would be more
//complicated to set default parameters in slot
/*void Simulator::setState(Vehicle::State newState, float _current, float _target_charge)
{
    //emit
    _setState(newState, _current, _target_charge); cout<<"emitted" << endl;
}*/ //TODO DELETE

//return: 0=changed, 1=same
int Simulator::setState(Vehicle::State newState, float _current, float _target_charge)
{
    if(newState == v->state) return 1;
    v->readwriteLock->lockForWrite();

    SStimestamp.restart();

    if(newState != Vehicle::Charging)
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
    //v->readwriteLock->unlock();


    // vypis //
    //v->readwriteLock->lockForRead(); //keep locked for write, to prevent switchng modes if recursed (causes deadlock)
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
    }

    if(v->state == Vehicle::Driving || v->state == Vehicle::Off) cout << endl << "[!] Not implemented.";

    cout << endl << endl;
    v->readwriteLock->unlock();

    emit _simulateRequest();
    emit redrawRequest();
    emit logRequest(briefly, priority_now);
    emit broadcastRequest(priority_now);
    return 0;
}

void Simulator::setCharge(float level)
{
    v->readwriteLock->lockForWrite();
    v->charge = level;
    v->readwriteLock->unlock();

    emit _simulateRequest(); //calls redrawRequest inside
}

void Simulator::setCurrent(float current)
{
    v->readwriteLock->lockForWrite();
    v->current = current;
    v->readwriteLock->unlock();

    emit redrawRequest();
}

float Simulator::_calcCharge(void)
{
    //shadows
    v->readwriteLock->lockForWrite(); //write, to prevent swtiching modes if already locked
    float charge = v->charge;
    float target_charge = v->target_charge;
    float current = v->current;

    //  Charging speed:        (a)
    //  15A      4032 min    -0,0010     1.0
    //  - linear scale -
    //  300A     130 min     -0,0350     35

    //if(charge >= 100) return 100; //treba??

    //declared as global so _calcOthers can access it to calc remaining time
    ///float a;    // exponential eq parameter
    ///float x;    // "time" (position) in charging process
    ///float y;    // charge in %

    //solve (a) parameter for actual charging current
    a = -(currentEq_param_a * current + currentEq_param_c) / 1000; // y=ax+c
    if(a>0) return charge; //for too low current


    // find position in charging process for actual charge level
    x = (1/a) * log(-((charge-100) / 100));  //inverse func of next line  // x=(1/a)*log(-((y-100)/100))

    // calc new charge level for time+1 (second/minute)
    y = 100.0 * (1 - exp(a * (x + (1.0/60*TIME_SPEED))));   //charging exp curve  // y=100*(1-exp(a*x))

    if(y>99.5) y=100;  //end of exp func is very long so consider charging as complete
    if(y>target_charge) y=target_charge;

    //printf("##: x= %f  xt = %f  y= %f  a=%f \n", x, x_full, y, a);

    v->charge = y;
    v->readwriteLock->unlock();
    return y;
}

void Simulator::_calcOthers() //not related only to charging
{
    v->readwriteLock->lockForWrite();

    float _target_charge = (v->target_charge>99.5) ? 99.5 : v->target_charge; //cut end of exp func
    float x_full = (1/a) * log(-((_target_charge-100-0.01) / 100));  //x at target_charge
    v->remaining_time = (x_full - x);

    //v->elapsed_time = (float)(clock()-chargingSStimestamp) / CLOCKS_PER_SEC /60 * TIME_SPEED;
    v->elapsed_time = static_cast<float>(SStimestamp.elapsed())/1000/60 * TIME_SPEED; //result in minutes

    v->voltage = v->min_voltage + ((v->max_voltage - v->min_voltage) * v->charge/100.0);
    v->range = (v->battery_capacity/100)*(v->charge/100.0); //* vehicle_effectivity;

    v->readwriteLock->unlock();
}

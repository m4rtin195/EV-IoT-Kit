#ifndef VEHICLE_H
#define VEHICLE_H

#include <iostream>
//#include <QtGlobal>
//#include <logger.h>

using namespace std; //global pre vsetky triedy

//simulation curve parameters
#define currentEq_param_a 0.119
#define currentEq_param_c -0.785

#define briefly true
#define extended false

class Vehicle
{
public:
    Vehicle();

    enum State {Off, Charging, Idle, Driving};

    const string vehicleId = "386625";
    const float min_voltage = 500.0; //bolo 600
    const float max_voltage = 700.0;
    const int battery_capacity = 50000;

    State state;
    float charge, target_charge, voltage; //in percent, volts
    float current, max_current;
    uint16_t elapsed_time, remaining_time; //in minutes, elapsed = since charging started/finished
    uint16_t range;
    float elec_consumption;
    float indoor_temp;

    float outdoor_temp;
    float desired_temp;
    string location;

    //void report(bool brief = false); //TODO report
};

#endif // VEHICLE_H

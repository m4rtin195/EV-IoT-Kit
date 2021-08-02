#include "vehicle.h"

Vehicle::Vehicle()
{
    state = Idle;
    charge = 0.0;         //percent
    target_charge = 0.0;  //percent
    voltage = 0.0;        //volts
    current = 0.0;        //amps
    max_current = 0.0;    //amps

    elapsed_time = 0;     //minutes
    remaining_time = 0;   //minutes

    //battery_resistance = 0.0;  //ohms
    //battery_temp = 0.0;   //celsius
    //actual_capacity = 0;  //Wh

    range = 0;              //km
    elec_consumption = 0.0; //kWh/100km
    indoor_temp = 0.0;      //celsius
}

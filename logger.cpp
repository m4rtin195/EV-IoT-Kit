#include "logger.h"

Logger::Logger(Vehicle* vehicle) : QObject()
{
    v = vehicle;
}

//static
void Logger::log(QString message)
{
    cout << message.toStdString() << endl;
}

//static
string Logger::time()
{
    time_t clock = chrono::system_clock::to_time_t(chrono::system_clock::now());
    struct tm* system_time = localtime(&clock);

    char buff[16];
    snprintf(buff, sizeof(buff), "%02d:%02d:%02d", system_time->tm_hour, system_time->tm_min, system_time->tm_sec);

    return buff;
}

string Logger::minsToTime(uint16_t mins)
{
    uint8_t days=0, hours=0, minutes=0;
    days = mins/1440; mins -= days*1440;
    hours = mins/60; mins -= hours*60;
    minutes = mins;

    string result = (days ? (to_string(days)+"d, ") : "") + (hours ? (to_string(hours)+"h, ") : "") + to_string(minutes)+"m";
    return result;
}


void Logger::broadcastCompleted(int status)
{
    if(BROADCASTER_LOGGING_ENABLED == false) return;

    switch(status)
    {
        case 1:  cout << "[>] Broadcasting message (at " << time() << ") successful (Sigfox)" << endl; break;
        case 2:  cout << "[>] Broadcasting message (at " << time() << ") successful (Wi-Fi)" << endl; break;
        default: cout << "[!] Broadcasting message (at " << time() << ") failed with code: " << status << endl;
    }
}

void Logger::logRequest(bool brief, bool priority)
{
    static int i = 0;
    if(SIMULATOR_LOGGING_ENABLED == false) return;
    if(priority != true && (++i % SIMULATOR_LOGGING_INTERVAL)) return;

    if(brief)
    {
        QString s = "* %1     %2V     %3% of %4%     (%5 / %6)  \t %7km";
        cout << s.arg(time().c_str())
                .arg(QString::number(v->voltage, 'f', 1)) //clazy:exclude=qstring-arg
                .arg(QString::number(v->charge, 'f', 1), QString::number(v->target_charge, 'f', 0))
                .arg(minsToTime(v->elapsed_time).c_str(), minsToTime(v->remaining_time).c_str())
                .arg(v->range)
                .toStdString();
        cout << endl;
    }

    else
    {
        printf("\n");
        printf("Vehicle status report: \n");
        printf("  Charge: %.1f%% \t Target charge: %.0f%% \n", v->charge, v->target_charge);
        printf("  Voltage: %.1fV \t Max voltage: %.0fV \n", v->voltage, v->max_voltage);
        printf("  Current: %.1fA \t Max current: %.0fA \n", v->current, v->max_current);
        printf("  Capacity: %dWh \t Max capacity: %dWh \t Factory capacity: %dWh \t Wear: %.1f%% \n", 50000, 0/*actual_capacity*/, v->battery_capacity, 0.0/*(actual_capacity/factory_capacity)*/);
        printf("  Elapsed time: %s \t Remaining time: %s \n", minsToTime(v->elapsed_time).c_str(), minsToTime(v->elapsed_time).c_str());
        printf("  Battery resistance: %.2fohm \t Battery temperature: %.1f°C \n", 0.0/*battery_resistance*/, 0.0/*battery_temp*/);
        printf("  Outdoor temperature: %.1f°C \t Indoor temperature: %.1f°C \t Desired temperature: %.1f°C \n", v->outdoor_temp, v->indoor_temp, v->desired_temp);
        printf("  Fuel Consumption:  %.1fl/100km \n  Elec. consumption: %.1fkWh/100km \n", 0.0/*fuel_consumption*/, v->elec_consumption);
        printf("  Fuel amount: %.1fl \t Electric-Gas ratio: %s \t range: %dkm \n", 0.0/*fuel_weight*/, "1:0", v->range);    ///
        printf("  Vehicle location: %s \n", "0.0000 0.0000");
        printf("\n");
    }
}


#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <iomanip>
#include <QObject>
#include <QString>
#include <QTextStream>

#include "vehicle.h"

using namespace std;

#define SIMULATOR_LOGGING_ENABLED true
#define BROADCASTER_LOGGING_ENABLED false

class Logger : public QObject
{
    Q_OBJECT
public:
    explicit Logger(Vehicle* vehicle);

    static string time();
    static string minsToTime(uint16_t mins);
    static void log(QString message);
    void report(bool brief);

private:
    Vehicle* v;

public slots:
    void broadcastCompleted(int status);
    void logRequest(bool brief);

};

#endif // LOGGER_H

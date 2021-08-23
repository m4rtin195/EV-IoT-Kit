#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <iomanip>
#include <QObject>
#include <QString>
#include <QTextStream>

#include "vehicle.h"

// config
#define BROADCASTER_LOGGING_ENABLED true    // write broadcast logs to console
#define SIMULATOR_LOGGING_ENABLED true      // write simulation logs to console
#define SIMULATOR_LOGGING_INTERVAL 5        // interval of periodical simulation logs [each x]
// /////

#define briefly true
#define extended false
#define priority_now true

using namespace std;

class Logger : public QObject
{
    Q_OBJECT
public:
    explicit Logger(Vehicle* vehicle);

    static void log(QString message);
    static string time();
    static string minsToTime(uint16_t mins);

private:
    Vehicle* v;

public slots:
    void broadcastCompleted(int status);
    bool logRequestSlot(bool brief, bool priority = false);
    void log(bool brief);

};

#endif // LOGGER_H

#ifndef BROADCASTER_H
#define BROADCASTER_H

#include <QObject>
#include <QString>
#include <QThread>
#include <QTimer>
#include <QtNetwork>

#include "include/serialib.h"
#include "include/half.hpp"
#include "logger.h"
#include "vehicle.h"

// config
#define BROADCAST_INTERVAL 10000    // interval of broadcasting attempts [ms]
#define SIGFOX_INTERVAL 1           // broadcasting via sigfox [each x broadcast attempt]
#if defined (_WIN32) || defined (_WIN64)
    #define SERIAL_PORT_NAME "COM3" // serial port name //win: COM0  rpi: /dev/ttyAMA1
#endif
#if defined (__linux__)
    #define SERIAL_PORT_NAME "/dev/ttyAMA1"
#endif
#define API_KEY "33VVtBeulA7RVGd3xBdpw5sFb3O14AAj22ZkEkar"    //AWS API key for broadcasting via network
// /////

#define OK 0
#define UNKNOWN_REPLY -3
#define NO_REPLY -4

//usable for either for masks and parameters
enum Connectivity {None=0, Sigfox=1, Wifi=2};

using namespace half_float;

class Broadcaster : public QThread
{
    Q_OBJECT

public:
    explicit Broadcaster(Vehicle* vehicle);
    ~Broadcaster();

    void enabled(bool b);
    Connectivity getConnectivity();

private:
    QTimer *tickTimer;
    serialib serial;
    QNetworkAccessManager nam;
    Vehicle* v;
    clock_t lastBroadcastTimestamp;
    int availConnectivity;

    void run() override;

    int _checkValues();
    int _buildSigfoxFrame(uint8_t* frame);
    int _checkConnectivity();
    int _broadcast();
    int _broadcastSerial(uint8_t* payload);
    int _broadcastInternet(uint8_t* payload);
    int _resetAT();

signals:
    void broadcastCompleted(int status);
    void connectivityChanged(int connectivity);

public slots:
    void broadcast(bool priority = false);
};

#endif // BROADCASTER_H

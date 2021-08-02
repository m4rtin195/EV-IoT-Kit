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

#define BROADCAST_INTERVAL 5000
#define SIGFOX_INTERVAL 10  //multiples of BROADCAST_INTERVAL

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
    QTimer *timer;
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
    void broadcast();
};

#endif // BROADCASTER_H

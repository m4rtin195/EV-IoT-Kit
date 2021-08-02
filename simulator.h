#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <QObject>
#include <QString>
#include <QThread>
#include <QTimer>
#include <QElapsedTimer>
#include <iostream>
#include <string>
#include <cmath>

#include "vehicle.h"
#include "logger.h"

#define SIMULATION_INTERVAL 1000
#define TIME_SPEED 60 //for DEMO: count seconds as minutes /change to 1 in production

class Simulator : public QThread
{
    Q_OBJECT

public:
    explicit Simulator(Vehicle* vehicle);
    ~Simulator();

    void enabled(bool b);
    void setInitialValues();
    int setState(Vehicle::State s, float _current = 0, float _target_charge = 0); //TODO moze to byt volane priamo?? int?
    void setCharge(float level);    //aj toto
    void setCurrent(float current);

private:
    Vehicle* v;
    QTimer* timer;
    QTimer timer2; //TODO ???
    QElapsedTimer SStimestamp;

    void run() override;

    float _calcCharge();
    void _recalcOthers();

signals:
    void redrawRequest();
    void logRequest(bool brief);
    void broadcastRequest();

private slots:
    void _simulate();

};

#endif // SIMULATOR_H

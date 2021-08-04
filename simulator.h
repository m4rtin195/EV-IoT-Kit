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

#include "logger.h"
#include "vehicle.h"

// config
#define SIMULATION_INTERVAL 1000    // interval of simulation of new values [ms]
#define TIME_SPEED 60               // for DEMO - count seconds as minutes /change to 1 in production //TODO premaz
// /////

class Simulator : public QThread
{
    Q_OBJECT

public:
    explicit Simulator(Vehicle* vehicle);
    ~Simulator();

    void enabled(bool b);
    void setInitialValues();
    int setState(Vehicle::State s, float _current = 0, float _target_charge = 0); //TODO moze to byt volane priamo??
    void setCharge(float level);    //aj toto
    void setCurrent(float current);

private:
    Vehicle* v;
    QTimer* tickTimer;
    QTimer timer2; //TODO ???
    QElapsedTimer SStimestamp;

    void run() override;

    float _calcCharge();
    void _recalcOthers();

signals:
    void redrawRequest();
    void logRequest(bool brief, bool priority = false);
    void broadcastRequest(bool priority = false);

private slots:
    void _simulate();

};

#endif // SIMULATOR_H

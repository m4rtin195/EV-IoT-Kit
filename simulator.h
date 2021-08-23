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
#define SIMULATION_INTERVAL 5000    // interval of simulation of new values [ms]
#define TIME_SPEED 60               // for DEMO - count seconds as minutes /change to 1 in production //TODO premaz
// /////

class Simulator : public QObject
{
    Q_OBJECT

public:
    explicit Simulator(Vehicle* vehicle);
    ~Simulator();

    void enable(bool b);
    bool isEnabled();
    void setInitialValues();
    int setState(Vehicle::State s, float _current = 0, float _target_charge = 0); //can be called also directly from main thread! (is it ok?)
    void setCharge(float level);    // aj toto
    void setCurrent(float current); // -//-

private:
    Vehicle* v;
    QTimer* tickTimer;
    QTimer timer2; //TODO ???
    QElapsedTimer SStimestamp;

    //int _setState(Vehicle::State s, float _current, float _target_charge);
    float _calcCharge();
    void _calcOthers();

signals:
    void redrawRequest();
    void logRequest(bool brief, bool priority = false);
    void broadcastRequest(bool priority = false);
    void _simulateRequest();

    //void _setStateRequest(Vehicle::State, float, float);

private slots:
    void _simulate();
};

#endif // SIMULATOR_H

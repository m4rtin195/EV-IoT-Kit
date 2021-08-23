#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>
#include <QDebug>

#include "include/q_debugstream.h"
#include "logger.h"
#include "vehicle.h"
#include "simulator.h"
#include "broadcaster.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Window; }
QT_END_NAMESPACE

class Window : public QWidget
{
    Q_OBJECT

public:
    Window(QWidget *parent = nullptr);
    ~Window();

    static void QMessageOutput(QtMsgType , const QMessageLogContext &, const QString &msg);

private:
    Ui::Window *ui;
    MessageHandler *msgHandler = Q_NULLPTR;
    ThreadLogStream* m_qd;

    Vehicle* vehicle;
    Vehicle* v;
    Logger* logger;
    Broadcaster* broadcaster;
    Simulator* simulator;

    QThread broadcasterThread;
    QThread simulatorThread;

    bool updatingLock = false;

signals:
    void broadcasterEnablingRequest(bool);
    void simulatorEnablingRequest(bool);

public slots:
    void updateUI();

private slots:
    void on_button_off_clicked();
    void on_button_idle_clicked();
    void on_button_charging_clicked();
    void on_button_driving_clicked();
    void on_button_ampMinus_clicked();
    void on_button_ampPlus_clicked();
    void on_button_exit_clicked();
    void on_scrollBar_actionTriggered_custom(int action);
    void on_scrollBar_sliderPressed();
    void on_scrollBar_sliderReleased();
};

#endif // WINDOW_H

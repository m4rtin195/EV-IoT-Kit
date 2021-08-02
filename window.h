#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>
#include <QThread> /////
#include <QDebug>

#include "logger.h"
#include "simulator.h"
#include "broadcaster.h"
#include "vehicle.h"
#include "q_debugstream.h"

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

    Logger* logger;
    Simulator* simulator;
    Vehicle* vehicle;
    Vehicle* v;

    Broadcaster* broadcaster;

public slots:
    void updateUI(); //TODO nemoze to byt obycajna metoda?

private slots:
    void on_button_off_clicked();
    void on_button_idle_clicked();
    void on_button_charging_clicked();
    void on_button_driving_clicked();
    void on_button_ampMinus_clicked();
    void on_button_ampPlus_clicked();
    void on_button_exit_clicked();
    void on_scrollBar_sliderMoved(int position);
};

#endif // WINDOW_H

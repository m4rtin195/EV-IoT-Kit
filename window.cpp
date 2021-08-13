#include "window.h"
#include "ui_window.h"

Window::Window(QWidget *parent) : QWidget(parent), ui(new Ui::Window)
{
    ui->setupUi(this);
    cout << "ui thread: " << QThread::currentThreadId() << endl;

    /**/
    // Set up ThreadLogStream, which redirect cout to signal sendLogString
    // Set up  MessageHandler,  wgucg catch message from sendLogString and Display
    //m_qd = new ThreadLogStream(std::cout); //Redirect Console output to QTextEdit
    //this->msgHandler = new MessageHandler(this->ui->logboard, this);
    connect(m_qd, &ThreadLogStream::sendLogString, msgHandler, &MessageHandler::catchMessage);
    /**/

    cout << "Initialization..." << endl;

    vehicle = new Vehicle(); v = vehicle;
    logger = new Logger(vehicle);
    broadcaster = new Broadcaster(vehicle);
    simulator = new Simulator(vehicle);
    QThread::msleep(100); //vypisy z thread

    connect(simulator, SIGNAL(logRequest(bool,bool)), logger, SLOT(logRequest(bool,bool)));
    connect(simulator, SIGNAL(redrawRequest()), this, SLOT(updateUI()));
    connect(simulator, SIGNAL(broadcastRequest(bool)), broadcaster, SLOT(broadcast(bool)));
    connect(broadcaster, SIGNAL(broadcastCompleted(int)), logger, SLOT(broadcastCompleted(int)));
    connect(broadcaster, SIGNAL(connectivityChanged(int)), this, SLOT(updateUI()));

    simulator->setInitialValues();
    updateUI();
    cout << endl << "--- time ----- voltage ----- charge -- elapsed | remaining ------- range ---" << endl;
    //               * 15:48:20     665,3V     65,3% of 80%     (0m / 8h,4m)            326km
    logger->logRequest(briefly, priority_now);

    ///demo
    //simulator->setState(Vehicle::Charging, 250, 70);
    //simulator->enabled(true);
    //broadcaster->enabled(true);
}

Window::~Window()
{
    delete ui;
    delete logger;
    delete simulator;
    delete broadcaster;
    delete vehicle;
}

void Window::QMessageOutput(QtMsgType, const QMessageLogContext &, const QString &msg)
{
   std::cout<<msg.toStdString().c_str()<<std::endl;
}

void Window::updateUI()
{
    if(v == nullptr) {cout << "v is null"; return;}
    QString s;

    if((v->state == Vehicle::Charging) || (v->state == Vehicle::Driving))
    {
        ui->button_ampMinus->setEnabled(true);
        ui->button_ampPlus->setEnabled(true);
    }
    else
    {
        ui->button_ampMinus->setEnabled(false);
        ui->button_ampPlus->setEnabled(false);
    }

    switch(v->state)
    {
        case Vehicle::Off: s = "off"; break;
        case Vehicle::Idle: s = "idle"; break;
        case Vehicle::Charging: s = "charging"; break;
        case Vehicle::Driving: s = "driving"; break;
    }
    ui->label_state->setText(s);

    if(v->state == Vehicle::Off)
    {
        ui->label_charge->setText("-");
        ui->label_current->setText("-");
        ui->progressBar->setValue(0);
        ui->scrollBar->setValue(0);
        ui->scrollBar->setEnabled(false);
    }

    else //working states
    {
        s = QString::number(v->charge, 'f', 1) + "%";
        ui->label_charge->setText(s);

        s = QString::number(v->current, 'f', 0) + "A";
        ui->label_current->setText(s);

        ui->progressBar->setValue(v->charge);
        ui->scrollBar->setValue(v->charge);
        ui->scrollBar->setEnabled(true);
    }

    switch(broadcaster->getConnectivity())
    {
        case Connectivity::None: s = "offline"; break;
        case Connectivity::Sigfox: s = "Sigfox"; break;
        case Connectivity::Wifi: s = "Wi-Fi"; break;
    }
    ui->label_connectivity->setText(s);
}


/// UI CALLBACKS ///

void Window::on_button_off_clicked()
{
    simulator->setState(Vehicle::Off);
    simulator->enabled(false);
    broadcaster->enabled(false);
}

void Window::on_button_idle_clicked()
{
    simulator->setState(Vehicle::Idle);
    simulator->enabled(true);
    broadcaster->enabled(true);
}

void Window::on_button_charging_clicked()
{
    simulator->setState(Vehicle::Charging, 120, 85);
    simulator->enabled(true);
    broadcaster->enabled(true);
}

void Window::on_button_driving_clicked()
{
    simulator->setState(Vehicle::Driving, -20);
    simulator->enabled(true);
    broadcaster->enabled(true);
}

void Window::on_button_ampMinus_clicked()
{
    simulator->setCurrent(v->current - 20);
}

void Window::on_button_ampPlus_clicked()
{
    simulator->setCurrent(v->current + 20);
}

void Window::on_button_exit_clicked()
{
    qApp->exit();
}

void Window::on_scrollBar_sliderMoved(int position)
{
    simulator->setCharge(position);
    updateUI();
}


#include "broadcaster.h"

#define is_big_endian (*(uint16_t *)"\0\xff" < 0x100)

Broadcaster::Broadcaster(Vehicle* vehicle) : QThread()
{
    v = vehicle;

    /// init setial port
    int status = serial.openDevice(SERIAL_PORT_NAME, 9600);
    cout << "opening port, status: " << status << endl;
    if(status != 1) {cout << "[!] Serial port initialization failed." << endl;}

    /// init modem
    if(_resetAT()) {cout << "[!] Modem not ready." << endl;}

    tickTimer = new QTimer(this);
    connect(tickTimer, SIGNAL(timeout()), this, SLOT(broadcast()));

    _checkConnectivity();
    this->QThread::start();
}

Broadcaster::~Broadcaster()
{
    serial.closeDevice();
    this->quit();
    this->wait();
}

void Broadcaster::enabled(bool b)
{
    if(b)
        tickTimer->start(BROADCAST_INTERVAL);
    else
        tickTimer->stop();
}

// override
void Broadcaster::run()
{
    cout << "[i] Broadcaster thread started." << endl;
    this->exec(); //run the event loop
}

// high level public do broadcast
void Broadcaster::broadcast(bool priority)
{
    // todo timing here
    if(priority != true && (v->state == Vehicle::Off)) return;
    if(priority != true && (v->state == Vehicle::Idle)) return;


    int status = _broadcast();
    emit broadcastCompleted(status);
}

// low level do broadcast
// return: 1=success via sigfox, 2=success via wifi, 0=no connectivity,
//        -1,-2=serial error, -3=unknown reply, -4=no reply, -5=skipped, -9=network error,
//        -10=wrong values, -20=not implemented, >100=httpcode
int Broadcaster::_broadcast()
{
    int status;
    uint8_t frame[12];
    memset(frame, 0, 12);
    if(_buildSigfoxFrame(frame) != 0)
    {
        cout << "[!] Failed to create frame." << endl;
        return -10;
    }

    _checkConnectivity();
    switch(getConnectivity())
    {
        case None:
        {
            cout << "[X] Device Offline." << endl;
            return 0;
        }
        case Sigfox:
        {
            cout << "broadcasting via sigfox" << endl;
            status = _broadcastSerial(frame);
            if(status == 0) status = 1; //remap
            return status;
        }
        case Wifi:
        {
            cout << "broadcasting via wifi" << endl;
            status = _broadcastInternet(frame);
            if(status == 0) status = 2; //remap
            return status;
        }
        default:
        {
            cout << "unknown connectivity mask" << endl;
            return -20;
        }
    }
}

// return: 0=ok, 1=impossible values, 2=values too big
int Broadcaster::_checkValues()
{
    if(v->state > 3) return 1;
    if(v->charge > 100) return 1;
    if(v->target_charge > 100.0) return 1;
    if(v->current > 1023.0) return 2;
    if(v->elapsed_time > 8191) return 2;  //5.68 days
    if(v->remaining_time > 8191) return 2;
    if(v->range > 2047) return 2;
    //if(v->elec_consumption);   //no need to check
    //if(v->indoor_temp);        //no need to check

    //if(v->outdoor_temp);
    //if(v->desired_temp);
    if(v->max_current > 1023.0) return 2;
    //if(v->location) return 1; //regex?

    return 0;
}

//return: same as _checkValues()
int Broadcaster::_buildSigfoxFrame(uint8_t* frame)
{
    //uint8_t payload[12];
    //memset(payload, 0, 12);
    uint32_t p1=0, p2=0, p3=0;
    void* tmpptr = 0;

    //check values are in processable bounds
    if(int st = _checkValues() != 0)
        return st;

    /// build frame
    //state
    uint8_t _state = v->state;
    _state &= 0x0007;
    p1 |= _state << 29;

    //current charge
    uint8_t _charge = static_cast<uint8_t>(v->charge);
    _charge &= 0x007F;
    p1 |= _charge << 22;

    //target charge
    uint8_t _target_charge = static_cast<uint8_t>(v->target_charge);
    _target_charge &= 0x007F;
    p1 |= _target_charge << 15;

    //current
    uint16_t _current = static_cast<uint16_t>(v->current);
    _current &= 0x03FF;
    p1 |= _current << 5;

    //elapsed time
    uint16_t _elapsed_time = v->elapsed_time;
    _elapsed_time &= 0x1FFF;
    p1 |= (_elapsed_time >> 8) << 0;
    p2 |= (_elapsed_time & 0xFF) << 24;

    //remaining time
    uint16_t _remaining_time = v->remaining_time;
    _remaining_time &= 0x1FFF;
    p2 |= _remaining_time << 11;

    //range
    uint16_t _range = v->range;
    _range &= 0x07FF;
    p2 |= _range << 0;

    //consumption
    half _consumption = half_cast<half>(v->elec_consumption);
    tmpptr = &_consumption;
    uint16_t _consumption_i = *(uint16_t*)tmpptr;
    p3 |= (_consumption_i & 0xFFFF) << 16;

    //temperature
    half _indoor_temp = half_cast<half>(v->indoor_temp);
    tmpptr = &_indoor_temp;
    uint16_t _indoor_temp_i = *(uint16_t*)tmpptr;
    p3 |= (_indoor_temp_i & 0xFFFF) << 0;


    //swap byte order
    if(!is_big_endian)
    {
        p1 = __builtin_bswap32(p1);
        p2 = __builtin_bswap32(p2);
        p3 = __builtin_bswap32(p3);
    }

    memcpy(frame+0,&p1,4);
    memcpy(frame+4,&p2,4);
    memcpy(frame+8,&p3,4);

    return 0;
}

//return: 0=success, -1,-2=serial error, -3=unknown reply, -4=no reply -5=skipped
int Broadcaster::_broadcastSerial(uint8_t* payload)
{
    //skip broadcasts for sigfox
    static int a = SIGFOX_INTERVAL;
    if(a++ < SIGFOX_INTERVAL) return -5;
    else a=0;

    int status = INT_MAX;

    char ATcommand[31];
    memcpy(&ATcommand[0], "AT$SF=", 6);
    for(int i=0; i<12; i++)
        sprintf(&ATcommand[6+(i*2)], "%02x", payload[i]);
    ATcommand[30] = 0x0A;

    /*//debug
    printf("\n data: ");
    for(int i=0; i<12; i++)
        printf("%2d: 0x%02X ", i+1, payload[i]);

    printf("\n command: ");
    for(int i=0; i<31; i++)
        printf("%c", ATcommand[i]);
    cout << endl;*/

    serial.flushReceiver();
    status = serial.writeBytes(ATcommand,31);
    if(status!=1) return status;
    else cout << "[>] Broadcasting..." << endl;

    char answ[20];
    int val = serial.readString(answ,'\n',20,10000);  //wait 8sec for transmission and ack
    if(val>0)
    {
        if(strncmp(answ,"OK\r\n",4)==0)
            status = OK;
        else
        {
            cout << "[!] Unknown AT reply (n=" << val << "): " << answ << endl;
            if(_resetAT()) {cout << "[!] Cannot reset AT." << endl;}
            status = UNKNOWN_REPLY;
        }
    }
    else //val<0
    {
        if(val==0) status = NO_REPLY;
        else status = val;
    }
    return status;
}

int Broadcaster::_resetAT()
{
    char answ[20];
    serial.flushReceiver();
    serial.writeString("AT\n");
    serial.writeString("AT\n");

    int val = serial.readString(answ,'\n',6,2000);
    if(val==0) return NO_REPLY;
    if(strncmp(answ,"OK\r\n",3)==0 || strncmp(answ,"OK\r\nOK\r\n",6)==0) return OK;
    return val;
}

//return: 0=success, -9=unknown error, other=httpcode
int Broadcaster::_broadcastInternet(uint8_t* payload)
{
    QJsonObject body;
    body.insert("timestamp", QJsonValue(QDateTime::currentMSecsSinceEpoch()));
    body.insert("vehicleId", QJsonValue(v->vehicleId.c_str()));
    body.insert("rawdata", QJsonValue(*payload));
    body.insert("encoded", QJsonValue(false));      //if floats are encoded to half
    body.insert("advanced", QJsonValue(true));
    body.insert("connectivity", QJsonValue(1));

    body.insert("state", QJsonValue(v->state));
    body.insert("current_charge", QJsonValue((int)v->charge));
    body.insert("target_charge", QJsonValue((int)v->target_charge));
    body.insert("current", QJsonValue(v->current));
    body.insert("elapsed_time", QJsonValue(v->elapsed_time));
    body.insert("remain_time", QJsonValue(v->remaining_time));
    body.insert("range", QJsonValue(v->range));
    body.insert("elec_consumption", QJsonValue(v->elec_consumption));
    body.insert("indoor_temp", QJsonValue(v->indoor_temp));

    body.insert("outdoor_temp", QJsonValue(v->outdoor_temp)); //NOTE demo
    body.insert("desired_temp", QJsonValue(v->desired_temp));
    body.insert("max_current", QJsonValue(v->max_current));
    body.insert("location", QJsonValue(v->location.c_str()));

    QJsonDocument doc(body);
    QByteArray _body = doc.toJson();
    //printf("%s", _body.toStdString().c_str());

    QNetworkRequest request(QUrl("https://uilqy1jfsf.execute-api.eu-central-1.amazonaws.com/v2/wifi/"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("connectivityMethod", "network");
    request.setRawHeader("x-api-key", API_KEY);

    QNetworkReply* reply = nam.post(request, _body);
    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    int httpcode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    //qDebug() << reply->readAll();

    if(httpcode == 201)
        return 0;
    else
        if(httpcode == 0) return -5;
        else return httpcode;
}

//returns available connectivity mask
int Broadcaster::_checkConnectivity()
{
    cout << "checking connectivity....";
    int connectivity = 0;

    //Wifi
    QNetworkRequest request(QUrl("http://www.google.com/"));
    QNetworkReply* reply = nam.head(request); //HTTP HEAD operation
    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    if(reply->error() == QNetworkReply::NoError)
        connectivity |= (1 << Connectivity::Wifi);

    //Sigfox
    if(serial.available() == 0)
        if(_resetAT() == OK)
            connectivity |= (1 << Connectivity::Sigfox);

    if(connectivity != availConnectivity)
    {
        availConnectivity = connectivity;
        emit connectivityChanged(connectivity);
    }
    cout << connectivity << endl;
    return connectivity;
}

//returns used (top-priority) connectivity
Connectivity Broadcaster::getConnectivity()
{
    if(availConnectivity == 0)
        return Connectivity::None;

    if(availConnectivity & (1 << Connectivity::Wifi)) //Wifi (and maybe Sigfox)
        return Connectivity::Wifi;

    else if(availConnectivity & (1 << Connectivity::Sigfox)) //Sigfox only
        return Connectivity::Sigfox;

    return Connectivity::None;
}

#include "broadcaster.h"

#define is_big_endian (*(uint16_t *)"\0\xff" < 0x100)

Broadcaster::Broadcaster(Vehicle* vehicle) : QObject()
{
    v = vehicle;

    /// init setial port
    int status = serial.openDevice(SERIAL_PORT_NAME, 9600);
    cout << "opening port, status: " << status << endl;
    if(status != 1) {cout << "[!] Serial port opening failed." << endl;}

    /// init modem
    if(int st = _resetAT()) {cout << "[!] Modem not ready. (" << st << ")" << endl;}

    tickTimer = new QTimer(this);
    connect(tickTimer, SIGNAL(timeout()), this, SLOT(broadcast()));

    _checkConnectivity();
}

Broadcaster::~Broadcaster()
{
    serial.closeDevice();
}

// starts or stops timer which invokes broadcast()
void Broadcaster::enable(bool b)
{
    if(b)
        tickTimer->start(BROADCAST_INTERVAL);
    else
        tickTimer->stop();
}

bool Broadcaster::isEnabled()
{
    return tickTimer->isActive();
}


/// tick, do broadcast
// emits: 0=no connectivity, 1=success via sigfox, 2=success via wifi,
//       -1,-2=serial error, -3=unknown reply, -4=no reply, -5=skipped, -9=network error,
//       -10=wrong values, -20=not implemented, >100=httpcode
void Broadcaster::broadcast(bool priority)
{
    // TODO timing here
    v->readwriteLock->lockForRead();
    if(priority != true && (v->state == Vehicle::Off)) return;
    if(priority != true && (v->state == Vehicle::Idle)) return;

    v->readwriteLock->unlock();

    int status;

    _checkConnectivity();
    switch(getConnectivity())
    {
        case None:
        {
            cout << "[X] Device Offline." << endl;
            status = 0;
            break;
        }

        case Sigfox:
        {
            //skip broadcasts for sigfox
            static int a = SIGFOX_INTERVAL; //interval match counter
            if((priority != true) && (a++ < SIGFOX_INTERVAL)) //this broadcasting is not priority and didnt match interval
            {
                status = -5;
                break;
            }

            a=0; //interval match -> reset counter
            cout << "broadcasting via sigfox" << endl;
            status = _broadcastSerial();
            if(status == 0) status = 1; //remap
            break;
        }

        case Wifi:
        {
            cout << "broadcasting via wifi" << endl;
            status = _broadcastInternet();
            if(status == 0) status = 2; //remap
            break;
        }

        default:
        {
            cout << "unknown connectivity mask" << endl;
            status = -20;
        }
    }

    emit broadcastCompleted(status);
}

// return: 0=ok, 1=impossible values, 2=values too big
int Broadcaster::_checkValues()
{
    v->readwriteLock->lockForRead();

    int st = 0;
    if(v->state > 3) st = 1;
    if(v->charge > 100) st = 1;
    if(v->target_charge > 100.0) st = 1;
    if(v->current > 1023.0) st = 2;
    if(v->elapsed_time > 8191) st = 2;  //5.68 days
    if(v->remaining_time > 8191) st = 2;
    if(v->range > 2047) st = 2;
    //if(v->elec_consumption);   //no need to check
    //if(v->indoor_temp);        //no need to check

    //if(v->outdoor_temp);
    //if(v->desired_temp);
    if(v->max_current > 1023.0) st = 2;
    //if(v->location) status = 1; //regex?

    v->readwriteLock->unlock();
    return st;
}

//return: same as _checkValues()
int Broadcaster::_buildSigfoxFrame(uint8_t* frame)
{
    //uint8_t payload[12];
    //memset(payload, 0, 12);
    uint32_t p1=0, p2=0, p3=0;
    void* tmpptr = nullptr;

    //check values are in processable bounds
    int st = _checkValues();
    if(st != 0)
        return st;

    v->readwriteLock->lockForRead();

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

    v->readwriteLock->unlock();

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

//return: 0=success, -1,-2=serial error, -3=unknown reply, -4=no reply
int Broadcaster::_broadcastSerial()
{   
    int status = INT_MAX;
    uint8_t payload[12];
    memset(payload, 0, 12);
    int st = _buildSigfoxFrame(payload);
    if(st != 0)
    {
        cout << "[!] Failed to create frame. (" << st << ")" << endl;
        return -10;
    }

    char ATcommand[32];
    memcpy(&ATcommand[0], "AT$SF=", 6);
    for(int i=0; i<12; i++)
        sprintf(&ATcommand[6+(i*2)], "%02x", payload[i]);
    ATcommand[30] = 0x0D;
    ATcommand[31] = 0x0A;

    /*//debug
    printf("\n data: ");
    for(int i=0; i<12; i++)
        printf("%2d: 0x%02X ", i+1, payload[i]);

    printf("\n command: ");
    for(int i=0; i<32; i++)
        printf("%c", ATcommand[i]);
    cout << endl;
    //*/

    serial.flushReceiver();
    status = serial.writeBytes(ATcommand,32);
    if(status!=1) return status;
    else cout << "[>] Broadcasting..." << endl;

    char answ[20];
    int val = serial.readString(answ,'\n',20,10000);  //wait 10sec for transmission and ack
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
    else //val<0 = no reply from modem
    {
        if(val==0) status = NO_REPLY;
        else status = val;
    }
    return status;
}

//return: 0=OK, -3=unknown reply (too long), -4=no reply
int Broadcaster::_resetAT()
{
    //for breaking potential valid previous input
    serial.writeChar(0xff);
    serial.writeChar('\n');
    QThread::msleep(100);
    serial.flushReceiver();

    //test
    serial.writeString("AT\r\n");

    char answ[20] = {0};
    int st = serial.readString(answ,'\n',4,2000);  //max 4 chars - "OK\r\n", 2sec timeout

    if(st==0) //timeout reached
        return NO_REPLY;
    else if(strncmp(answ,"OK\n",3)==0 || strncmp(answ,"OK\r\n",4)==0)
        return OK;
    else
        return st; //not "OK" reply
}

//return: 0=success, -9=unknown error, other=httpcode
int Broadcaster::_broadcastInternet()
{
    //check values are in processable bounds
    if(int st = _checkValues() != 0)
        return st;

    v->readwriteLock->lockForRead();

    QJsonObject body;
    body.insert("timestamp", QJsonValue(QDateTime::currentMSecsSinceEpoch()));
    body.insert("vehicleId", QJsonValue(v->vehicleId.c_str()));
    body.insert("encoded", QJsonValue(false));      //if floats are encoded to half
    body.insert("advanced", QJsonValue(true));
    body.insert("connectivity", QJsonValue(1));

    body.insert("state", QJsonValue(v->state));
    body.insert("current_charge", QJsonValue(static_cast<int>(v->charge)));
    body.insert("target_charge", QJsonValue(static_cast<int>(v->target_charge)));
    body.insert("current", QJsonValue(static_cast<double>(v->current)));
    body.insert("elapsed_time", QJsonValue(v->elapsed_time));
    body.insert("remain_time", QJsonValue(v->remaining_time));
    body.insert("range", QJsonValue(v->range));
    body.insert("elec_consumption", QJsonValue(static_cast<double>(v->elec_consumption)));
    body.insert("indoor_temp", QJsonValue(static_cast<double>(v->indoor_temp)));

    body.insert("outdoor_temp", QJsonValue(static_cast<double>(v->outdoor_temp))); //NOTE demo
    body.insert("desired_temp", QJsonValue(static_cast<double>(v->desired_temp)));
    body.insert("max_current", QJsonValue(static_cast<double>(v->max_current)));
    body.insert("location", QJsonValue(v->location.c_str()));

    v->readwriteLock->unlock();
    QJsonDocument doc(body);
    QByteArray _body = doc.toJson();
    //printf("%s", _body.toStdString().c_str());

    QNetworkRequest request(QUrl("https://uilqy1jfsf.execute-api.eu-central-1.amazonaws.com/v2/wifi/"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("connectivityMethod", "network");
    request.setRawHeader("x-api-key", API_KEY);

    QNetworkAccessManager* nam;
    nam = new QNetworkAccessManager();
    QNetworkReply* reply = nam->post(request, _body);
    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    int httpcode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    //qDebug() << reply->readAll();

    if(httpcode == 201)
        return 0;
    else
        if(httpcode == 0) return -9;
        else return httpcode;
}

//returns available connectivity mask
int Broadcaster::_checkConnectivity()
{
    //cout << "checking connectivity....";
    int connectivity = 0;

    //Wifi
    QNetworkRequest request(QUrl("http://www.google.com/"));
    QNetworkAccessManager nam;
    QNetworkReply* reply = nam.head(request); //HTTP HEAD operation
    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    if(reply->error() == QNetworkReply::NoError)
        connectivity |= (1 << Connectivity::Wifi);

    //Sigfox
    if(_resetAT() == OK)
        connectivity |= (1 << Connectivity::Sigfox);


    if(connectivity != availConnectivity)
    {
        availConnectivity = connectivity;
        emit connectivityChanged(connectivity);
    }
    //cout << connectivity << endl;
    return connectivity;
}

//returns used (top-priority) Connectivity upon last _checkConnectivity()
Connectivity Broadcaster::getConnectivity()
{
    if(availConnectivity == 0)
        return Connectivity::None;

    //if(availConnectivity & (1 << Connectivity::Wifi)) //Wifi (and maybe Sigfox)
      //  return Connectivity::Wifi;

    else if(availConnectivity & (1 << Connectivity::Sigfox)) //Sigfox only
        return Connectivity::Sigfox;

    return Connectivity::None;
}






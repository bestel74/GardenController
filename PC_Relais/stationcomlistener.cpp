#include "stationcomlistener.h"



StationComListener::StationComListener(QObject *parent) :
    QObject(parent)
{
    // Connect to ddb
    _database = QSqlDatabase::addDatabase("QMYSQL");
    _database.setHostName(MYSQL_ADDR);
    _database.setDatabaseName(MYSQL_DDBN);
    _database.setUserName(MYSQL_USER);
    _database.setPassword(MYSQL_PASS);
    _database.open();

    connect(this, SIGNAL(sig_newPacket(S_Packet*)), this, SLOT(slot_readPacket(S_Packet*)));

    connect(&_timerComPortScanner, &QTimer::timeout, this, &StationComListener::slot_scanComPort);

    _timerComPortScanner.setInterval(SCANNER_UPDATE_TIME_MS);
    _timerComPortScanner.setSingleShot(true);
    _timerComPortScanner.start();
}



void StationComListener::slot_scanComPort() {
    qDebug(QString("%1 available ports found...").arg(
               QString::number(QSerialPortInfo::availablePorts().count())
               ).toUtf8());

    foreach(QSerialPortInfo serialPort, QSerialPortInfo::availablePorts()) {
        quint16 vid = serialPort.vendorIdentifier();
        quint16 pid = serialPort.productIdentifier();

        // Filter with known vid/pid
        if(_listKnownVid.contains(vid) && _listKnownPid.contains(pid)) {
            _serialPortInfo = serialPort;
            _serialPort.setPort(_serialPortInfo);

            connect(&_serialPort, &QSerialPort::readyRead, this, &StationComListener::slot_readComPort);
            connect(&_serialPort, &QSerialPort::readChannelFinished, this, &StationComListener::slot_comPortClosed);

            if(_serialPort.open(QIODevice::ReadWrite)) {
                qDebug(QString("Port \"%1\" is opened !").arg(
                           _serialPortInfo.portName()
                           ).toUtf8());

                _serialPort.setBaudRate(SERIALPORT_BAUDRATE);
                _serialPort.setDataBits(SERIALPORT_DATA);
                _serialPort.setParity(SERIALPORT_PARITY);
                _serialPort.setStopBits(SERIALPORT_STOP);
                _serialPort.setFlowControl(SERIALPORT_FLOWCONTORL);
            }
            else {
                qDebug(QString("An error occurred when trying to open \"%1\": %2").arg(
                           _serialPortInfo.portName(),
                           _serialPort.errorString()
                           ).toUtf8());
                disconnect(&_serialPort, &QSerialPort::readyRead, this, &StationComListener::slot_readComPort);
                disconnect(&_serialPort, &QSerialPort::readChannelFinished, this, &StationComListener::slot_comPortClosed);
            }
        }
    }

    // Relaunch scanner
    if(!_serialPort.isOpen()) {
        _timerComPortScanner.start();
    }
}


void StationComListener::slot_readComPort() {
    _buffer.append(_serialPort.readAll());
    checkForValidData();

    qDebug(QString("Message from \"%1\"").arg(
               _serialPortInfo.portName()
               ).toUtf8());
}


void StationComListener::slot_comPortClosed() {
    qDebug(QString("Port \"%1\" is now closed").arg(
               _serialPortInfo.portName()
               ).toUtf8());

    _serialPort.close();
    disconnect(&_serialPort, &QSerialPort::readyRead, this, &StationComListener::slot_readComPort);
    disconnect(&_serialPort, &QSerialPort::readChannelFinished, this, &StationComListener::slot_comPortClosed);
}


void StationComListener::checkForValidData() {
    // Need sizeof S_PacketHeader before start to read
    if(_buffer.size() >= sizeof(S_PacketHeader)) {
        int startIndex = _buffer.indexOf(C_MAGIC_NUMBER, 0);
        if(startIndex == 0) {
            S_Packet *pack = new S_Packet;

            // Copy header
            memcpy((char*)pack, _buffer.data(), sizeof(S_PacketHeader));
            int totalLength = sizeof(S_PacketHeader) + pack->header.dataLength;

            // Do we receive datas ?
            if(pack->header.dataLength > 0 && _buffer.size() >= totalLength) {
                pack->data = new unsigned char [pack->header.dataLength];
                memcpy((char*)pack->data, _buffer.data() + sizeof(S_PacketHeader), pack->header.dataLength);

                emit sig_newPacket(pack);
                _buffer.remove(0, totalLength);
            }
            else if(pack->header.dataLength == 0) {
                // No data, but it's OK
                emit sig_newPacket(pack);
                _buffer.remove(0, totalLength);
            }
            else {
                // Need to wait for more data
                delete pack;
            }
        }
        else if(startIndex > 0) {
            _buffer.remove(0, startIndex);
            checkForValidData();
        }
        else {
            _buffer.clear();
        }
    }
}

unsigned char magicNumber[3];
unsigned short dataLength;
unsigned char packetType;
unsigned char radioID;
short rssi;

void StationComListener::slot_readPacket(S_Packet *pack) {
    switch(pack->header.packetType) {
        case E_PacketType_VBatAndInternalTemperature:
        {
            short vbat, temp;
            memcpy((char *)&vbat, pack->data, sizeof(vbat));
            memcpy((char *)&temp, pack->data + sizeof(vbat), sizeof(temp));

            if(!_database.isOpen()) {
                _database.open();
            }
            QString query = QString("INSERT INTO status (`radioID`,`date`,`rssi`,`internalTemperature`,`vBat`) VALUES(") +
                    QString("'") + QString::number(pack->header.radioID) + QString("',") +
                    QString("NOW(),") +
                    QString("'") + QString::number(pack->header.rssi) + QString("',") +
                    QString("'") + QString::number(temp) + QString("',") +
                    QString("'") + QString::number(vbat) + QString("');");
            qDebug(QString("Query: ").toUtf8() + query.toUtf8());
            _database.exec(query);
        }
        break;

        case E_PacketType_Empty:
        {
            qDebug(QString("E_PacketType_Empty: Station is alive :)").toUtf8());
        }
        break;

        default: break;
    }
}

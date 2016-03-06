#ifndef STATIONCOMLISTENER_H
#define STATIONCOMLISTENER_H

#include <QObject>
#include <QTimer>
#include <QList>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QSqlDatabase>
#include <QSqlQuery>

#include "protocol.h"
#include "login.h"
// login.h defines these privates values :
//  MYSQL_ADDR     "IP or DNS"
//  MYSQL_DDBN     "database name"
//  MYSQL_USER     "user login"
//  MYSQL_PASS     "user password"



#define SCANNER_UPDATE_TIME_MS      1000

#define SERIALPORT_BAUDRATE         QSerialPort::Baud115200
#define SERIALPORT_DATA             QSerialPort::Data8
#define SERIALPORT_STOP             QSerialPort::OneStop
#define SERIALPORT_PARITY           QSerialPort::NoParity
#define SERIALPORT_FLOWCONTORL      QSerialPort::NoFlowControl

const QList<quint16> _listKnownVid = {0x0403};
const QList<quint16> _listKnownPid = {0x6015};


class StationComListener : public QObject
{
    Q_OBJECT
public:
    explicit StationComListener(QObject *parent = 0);

signals:
    void sig_newPacket(S_Packet *pack);

public slots:

private slots:
    void slot_scanComPort();
    void slot_readComPort();
    void slot_comPortClosed();
    void slot_readPacket(S_Packet *pack);

private:
    void checkForValidData();

    // UART
    QTimer _timerComPortScanner;
    QSerialPort _serialPort;
    QSerialPortInfo _serialPortInfo;
    QByteArray _buffer;

    // Database
    QSqlDatabase _database;
};

#endif // STATIONCOMLISTENER_H

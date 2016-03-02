#-------------------------------------------------
#
# Project created by QtCreator 2016-02-27T17:32:54
#
#-------------------------------------------------

QT       += core sql
QT       -= gui

# Compatible with QT4
lessThan(QT_MAJOR_VERSION, 5) {
    CONFIG += serialport
}
else {
    QT += serialport
}

CONFIG += serialport

CONFIG += c++11

TARGET = Projet_potager
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    stationcomlistener.cpp

HEADERS += \
    protocol.h \
    stationcomlistener.h \
    login.h

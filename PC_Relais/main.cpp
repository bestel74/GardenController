#include <QCoreApplication>

#include "stationcomlistener.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    StationComListener *listener = new StationComListener(&a);

    return a.exec();
}

#ifndef ESPANALOGHALLSENSOR_H
#define ESPANALOGHALLSENSOR_H

#include "ispeedsensor.h"

#include <QHostAddress>

class QTcpSocket;

class ESPAnalogHallSensor : public ISpeedSensor
{
    Q_OBJECT
public:
    explicit ESPAnalogHallSensor(QObject *parent = nullptr);
    ~ESPAnalogHallSensor();

signals:
    void sensorMinMaxChanged(int min, int max);

public slots:
    void setState(bool on);

    void resetTravelledCount();

    void setDebugOutput(bool val);

    void setThresholds(int highEnter, int highExit, int lowEnter, int lowExit);

    void resetSensorMinMax();

private slots:
    void parseData();

private:
    QTcpSocket *mSocket;
    QHostAddress mIPAddress;
    const int mPort = 1234;

    int maxValue = -1;
    int minValue = -1;
};

#endif // ESPANALOGHALLSENSOR_H

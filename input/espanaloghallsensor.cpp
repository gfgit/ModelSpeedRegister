#include "espanaloghallsensor.h"

#include <QTcpSocket>
#include <QElapsedTimer>

#include <QDebug>
#include <QTime>

ESPAnalogHallSensor::ESPAnalogHallSensor(QObject *parent)
    : ISpeedSensor{parent}
{
    mSocket = new QTcpSocket(this);
    connect(mSocket, &QTcpSocket::readyRead, this, &ESPAnalogHallSensor::parseData);

    mIPAddress = QHostAddress("192.168.1.4");
}

ESPAnalogHallSensor::~ESPAnalogHallSensor()
{
    setState(false);
}

void ESPAnalogHallSensor::setState(bool on)
{
    if(on && mSocket->state() != QTcpSocket::ConnectedState)
    {
        mSocket->connectToHost(mIPAddress, mPort, QIODevice::ReadWrite);
    }
    else if(!on && mSocket->state() == QTcpSocket::ConnectedState)
    {
        mSocket->disconnect();
    }
}

void ESPAnalogHallSensor::resetTravelledCount()
{
    if(mSocket->state() != QTcpSocket::ConnectedState)
        return;

    mSocket->write("r\n");
    mSocket->flush();
}

void ESPAnalogHallSensor::setDebugOutput(bool val)
{
    if(mSocket->state() != QTcpSocket::ConnectedState)
        return;

    mSocket->write(val ? "d\n" : "n\n");
    mSocket->flush();
}

void ESPAnalogHallSensor::setThresholds(int highEnter, int highExit, int lowEnter, int lowExit)
{
    if(mSocket->state() != QTcpSocket::ConnectedState)
        return;

    QString format(QLatin1String("s %1 %2 %3 %4\n"));
    QByteArray cmd = format.arg(highEnter).arg(highExit).arg(lowEnter).arg(lowEnter).toLatin1();

    mSocket->write(cmd);
    mSocket->flush();
}

void ESPAnalogHallSensor::resetSensorMinMax()
{
    minValue = -1;
    maxValue = -1;
    emit sensorMinMaxChanged(-1, -1);
}

void ESPAnalogHallSensor::parseData()
{
    QElapsedTimer t;
    t.start();

    while(mSocket->canReadLine())
    {
        QByteArray line = mSocket->readLine(30);

        qDebug() << "SENSOR:" << QTime::currentTime() << line;

        QList<QByteArray> list = line.split(' ');
        if(list.size() == 2)
        {
            // Might be a debug output
            // "NUM_HALF_ROTATIONS SENSOR_VALUE"
            bool ok = false;
            int halfRotations = list.at(0).toInt(&ok);
            int sensorValue = 0;
            bool minMaxChanged = false;

            if(ok)
            {
                Q_UNUSED(halfRotations);
                sensorValue = list.at(1).toInt(&ok);
            }

            if(ok)
            {
                if(sensorValue > maxValue)
                {
                    maxValue = sensorValue;
                    minMaxChanged = true;
                }

                if(minValue == -1 || sensorValue < minValue)
                {
                    minValue = sensorValue;
                    minMaxChanged = true;
                }
            }

            if(minMaxChanged)
            {
                emit sensorMinMaxChanged(minValue, maxValue);
            }
        }
        else if(list.size() == 4)
        {
            qint64 timestamp = 0;
            qint64 halfRotationCount = 0;
            double travelledSpace = 0;
            double avgSpeed = 0;

            bool ok = false;
            timestamp = list.at(0).toLongLong(&ok);

            if(ok)
                halfRotationCount = list.at(1).toLongLong(&ok);

            if(ok)
                travelledSpace = list.at(2).toDouble(&ok);

            if(ok)
                avgSpeed = list.at(3).toDouble(&ok);

            Q_UNUSED(halfRotationCount);

            if(ok)
            {
                emit speedReading(avgSpeed, travelledSpace, timestamp);
            }
        }

        if(t.hasExpired(500))
        {
            // Avoid blocking event loop
            // Reschedule parsing
            QMetaObject::invokeMethod(this, &ESPAnalogHallSensor::parseData, Qt::QueuedConnection);
            break;
        }
    }
}

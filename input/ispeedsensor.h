#ifndef ISPEEDSENSOR_H
#define ISPEEDSENSOR_H

#include <QObject>

#include "../commandstation/utils.h"

class ISpeedSensor : public QObject
{
    Q_OBJECT
public:
    explicit ISpeedSensor(QObject *parent = nullptr);

signals:
    void speedReading(double metersPerSecond, LocomotiveDirection direction, qint64 timestampMilliSec);
};

#endif // ISPEEDSENSOR_H

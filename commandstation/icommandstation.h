#ifndef ICOMMANDSTATION_H
#define ICOMMANDSTATION_H

#include <QObject>

#include "utils.h"

class ICommandStation : public QObject
{
    Q_OBJECT
public:
    static constexpr int EMERGENCY_STOP = -1;

    explicit ICommandStation(QObject *parent = nullptr);

    virtual bool setLocomotiveSpeed(int address, int speedStep, LocomotiveDirection direction) = 0;

    virtual bool emergencyStop(int address) = 0;

signals:
    void locomotiveSpeedFeedback(int address, int speedStep, LocomotiveDirection direction, bool wasQueued);
};

#endif // ICOMMANDSTATION_H

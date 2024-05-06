#ifndef DUMMYCOMMANDSTATION_H
#define DUMMYCOMMANDSTATION_H

#include "icommandstation.h"

class DummyCommandStation : public ICommandStation
{
    Q_OBJECT
public:
    explicit DummyCommandStation(QObject *parent = nullptr);

    bool setLocomotiveSpeed(int address, int speedStep, LocomotiveDirection direction) override;

    bool emergencyStop(int address) override;
};

#endif // DUMMYCOMMANDSTATION_H

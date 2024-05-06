#include "dummycommandstation.h"

#include <QTimer>

DummyCommandStation::DummyCommandStation(QObject *parent)
    : ICommandStation{parent}
{

}

bool DummyCommandStation::setLocomotiveSpeed(int address, int speedStep, LocomotiveDirection direction)
{
    QTimer::singleShot(500, this, [this, address, speedStep, direction]()
                       {
                           locomotiveSpeedFeedback(address, speedStep, direction);
                       });
    return true;
}

bool DummyCommandStation::emergencyStop(int address)
{
    QTimer::singleShot(500, this, [this, address]()
                       {
                           locomotiveSpeedFeedback(address, 0, LocomotiveDirection::Forward);
                       });
    return true;
}

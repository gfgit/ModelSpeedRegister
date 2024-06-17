#include "locomotivepool.h"

#include "../commandstation/icommandstation.h"

#include "locomotive.h"

LocomotivePool::LocomotivePool(QObject *parent)
    : QObject{parent}
{

}

LocomotivePool::~LocomotivePool()
{
    for(Locomotive* loco : mLocos)
    {
        loco->mPool = nullptr;
    }
    mLocos.clear();
}

void LocomotivePool::addLoco(Locomotive *loco)
{
    if(mLocos.contains(loco))
        return;
    mLocos.append(loco);

    if(loco->mPool)
        loco->mPool->removeLoco(loco);
    loco->mPool = this;
}

void LocomotivePool::removeLoco(Locomotive *loco)
{
    if(mLocos.removeOne(loco))
        loco->mPool = nullptr;
}

ICommandStation *LocomotivePool::commandStation() const
{
    return mCommandStation;
}

void LocomotivePool::setCommandStation(ICommandStation *newCommandStation)
{
    if(mCommandStation)
        disconnect(mCommandStation, &ICommandStation::locomotiveSpeedFeedback, this, &LocomotivePool::onLocomotiveSpeedFeedback);

    mCommandStation = newCommandStation;

    if(mCommandStation)
        connect(mCommandStation, &ICommandStation::locomotiveSpeedFeedback, this, &LocomotivePool::onLocomotiveSpeedFeedback);
}

void LocomotivePool::onLocomotiveSpeedFeedback(int address, int speedStep, LocomotiveDirection direction, bool wasQueued)
{
    for(Locomotive* loco : mLocos)
    {
        if(loco->address() == address)
        {
            loco->setSpeed_internal(speedStep, direction, wasQueued);
            break;
        }
    }
}

void LocomotivePool::driveLoco(int address, int speedStep, LocomotiveDirection direction)
{
    if(mCommandStation)
        mCommandStation->setLocomotiveSpeed(address, speedStep, direction);
}

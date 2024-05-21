#include "locomotive.h"

#include "locomotivepool.h"

Locomotive::Locomotive(QObject *parent)
    : QObject{parent}
{

}

Locomotive::~Locomotive()
{
    if(mPool)
        mPool->removeLoco(this);
}

int Locomotive::address() const
{
    return mAddress;
}

void Locomotive::setAddress(int newAddress)
{
    mAddress = newAddress;
}

int Locomotive::speedStep() const
{
    return mSpeedStep;
}

LocomotiveDirection Locomotive::direction() const
{
    return mDirection;
}

const LocoSpeedMapping &Locomotive::speedMapping() const
{
    return mSpeedMapping;
}

void Locomotive::setSpeedMapping(const LocoSpeedMapping &newSpeedMapping)
{
    mSpeedMapping = newSpeedMapping;
}

void Locomotive::driveLoco(int speedStep, LocomotiveDirection direction)
{
    if(speedStep < 0 && speedStep > 126)
        return;

    if(speedStep == mTargetSpeedStep && direction == mTargetDirection)
        return;

    if(mPool && mAddress != 0)
    {
        mTargetSpeedStep = speedStep;
        mTargetDirection = direction;
        mPool->driveLoco(mAddress, speedStep, direction);
    }
}

void Locomotive::setSpeed_internal(int speedStep, LocomotiveDirection dir)
{
    bool notify = false;
    if(mSpeedStep != speedStep || mDirection != dir)
        notify = true;

    mSpeedStep = speedStep;
    mDirection = dir;

    mTargetSpeedStep = mSpeedStep;
    mTargetDirection = mDirection;

    if(notify)
        emit changed(this);
}

LocomotiveDirection Locomotive::targetDirection() const
{
    return mTargetDirection;
}

int Locomotive::targetSpeedStep() const
{
    return mTargetSpeedStep;
}

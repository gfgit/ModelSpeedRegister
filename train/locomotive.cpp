#include "locomotive.h"

#include "locomotivepool.h"

#include <QDebug>

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
    emit nameChanged(mSpeedMapping.name());
    emit changed(this, true);
}

void Locomotive::driveLoco(int speedStep, LocomotiveDirection direction)
{
    if(speedStep < 0 && speedStep > 126)
        return;

    int oldTargetStep = mTargetSpeedStep;
    LocomotiveDirection oldTargetDir = mTargetDirection;

    mTargetSpeedStep = speedStep;
    mTargetDirection = direction;

    if(speedStep == oldTargetStep && direction == oldTargetDir)
    {
        emit changed(this, false);
        return;
    }

    qDebug() << "DRIVE: adr=" << mAddress << "Dir=" << (direction == LocomotiveDirection::Forward ? 'F' : 'R')
             << "Step=" << speedStep;

    if(mPool && mAddress != 0)
    {
        mPool->driveLoco(mAddress, speedStep, direction);
    }

    emit changed(this, false);
}

void Locomotive::setSpeed_internal(int speedStep, LocomotiveDirection dir, bool wasQueued)
{
    bool hasChanged = false;
    if(mSpeedStep != speedStep || mDirection != dir)
        hasChanged = true;

    qDebug() << "RECV: adr=" << mAddress << "Dir=" << (dir == LocomotiveDirection::Forward ? 'F' : 'R')
             << "Step=" << speedStep << "Changed=" << hasChanged << "Queued=" << wasQueued;

    mSpeedStep = speedStep;
    mDirection = dir;

    mTargetSpeedStep = mSpeedStep;
    mTargetDirection = mDirection;

    if(hasChanged)
        emit changed(this, wasQueued);
}

LocomotiveDirection Locomotive::targetDirection() const
{
    return mTargetDirection;
}

int Locomotive::targetSpeedStep() const
{
    return mTargetSpeedStep;
}

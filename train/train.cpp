#include "train.h"

#include "locomotive.h"

#include <QDebug>

#include <QTimerEvent>

LocomotiveDirection oppositeDir(LocomotiveDirection dir)
{
    if(dir == LocomotiveDirection::Forward)
        return LocomotiveDirection::Reverse;
    return LocomotiveDirection::Forward;
}

Train::Train(QObject *parent) :
    QObject(parent)
{
    active = false;
}

Locomotive *Train::getLocoAt(int idx) const
{
    if(idx < 0 || idx > mLocomotives.size())
        return nullptr;

    return mLocomotives.at(idx).loco;
}

bool Train::getLocoInvertDirAt(int idx) const
{
    if(idx < 0 || idx > mLocomotives.size())
        return false;

    return mLocomotives.at(idx).invertDir;
}

int Train::getLocoIdx(Locomotive *loco) const
{
    for(int i = 0; i < mLocomotives.size(); i++)
    {
        if(mLocomotives.at(i).loco == loco)
        {
            return i;
        }
    }

    return INVALID_LOCO_IDX;
}

void Train::onLocoChanged(Locomotive *loco, bool queued)
{
    if(!active || queued)
        return;

    int locoIdx = getLocoIdx(loco);

    if(locoIdx < 0)
        return;

    bool invert = mLocomotives.at(locoIdx).invertDir;
    LocomotiveDirection trainDirection = loco->targetDirection();
    if(invert)
        trainDirection = oppositeDir(trainDirection);

    if(trainDirection != mDirection)
        setDirection(trainDirection);

    if(loco->targetSpeedStep() == mLocomotives.at(locoIdx).lastSetStep)
        return;

    qDebug() << "TRAIN CHANGE: adr=" << loco->address();

    if(loco->targetSpeedStep() == EMERGENCY_STOP)
    {
        setEmergencyStop();
        return;
    }

    onLocoChangedInternal(locoIdx, loco->targetSpeedStep());
}

bool Train::addLoco(Locomotive *loco)
{
    if(active)
        return false;

    if(getLocoIdx(loco) != INVALID_LOCO_IDX)
    {
        return true; // Already added
    }

    mLocomotives.append(LocoItem{loco, false});
    connect(loco, &Locomotive::changed, this, &Train::onLocoChanged);

    // Reset
    mSpeedTable = TrainSpeedTable();

    return true;
}

bool Train::removeLoco(Locomotive *loco)
{
    if(active)
        return false;

    int locoIdx = getLocoIdx(loco);

    if(locoIdx < 0)
        return false; // Not in Train

    mLocomotives.removeAt(locoIdx);
    disconnect(loco, &Locomotive::changed, this, &Train::onLocoChanged);

    // Reset
    mSpeedTable = TrainSpeedTable();

    return true;
}

void Train::updateSpeedTable()
{
    if(mLocomotives.size() < 2)
    {
        // Reset
        mSpeedTable = TrainSpeedTable();
        return;
    }

    std::vector<LocoSpeedMapping> mappings;
    mappings.reserve(mLocomotives.size());
    for(int i = 0; i < mLocomotives.size(); i++)
    {
        mappings.push_back(mLocomotives.at(i).loco->speedMapping());
    }

    mSpeedTable = TrainSpeedTable::buildTable(mappings);

    int lastTableIdx = mSpeedTable.count() - 1;
    const TrainSpeedTable::Entry& lastEntry = mSpeedTable.getEntryAt(lastTableIdx);
    mMaxSpeed.tableIdx = lastTableIdx;
    mMaxSpeed.speed = lastEntry.avgSpeed;
}

void Train::setDirection(LocomotiveDirection dir)
{
    mDirection = dir;

    if(!active)
        return;

    auto invertedDir = oppositeDir(mDirection);

    // Apply direction to all locomotives
    for(int i = 0; i < mLocomotives.size(); i++)
    {
        const LocoItem& item = mLocomotives.at(i);

        LocomotiveDirection locoDir = item.invertDir ? invertedDir : mDirection;

        if(item.loco->targetDirection() == locoDir)
            continue; // Already in correct direction

        item.loco->driveLoco(item.lastSetStep, locoDir);
    }
}

void Train::setMaximumSpeed(double speed)
{
    auto match = mSpeedTable.getClosestMatch(speed);
    mMaxSpeed.tableIdx = match.first;
    mMaxSpeed.speed = match.second.avgSpeed;

    if(active && mTargetSpeed.tableIdx > mMaxSpeed.tableIdx)
    {
        // Current speed exceeds maximum, slow down
        setTargetSpeedInternal(mMaxSpeed.speed, mMaxSpeed.tableIdx);
    }
}

void Train::setEmergencyStop()
{
    if(!active)
        return;

    for(int i = 0; i < mLocomotives.size(); i++)
    {
        driveLoco(i, EMERGENCY_STOP);
    }

    // Cancel acceleration
    killTimer(mAccelerationTimerId);
    mAccelerationTimerId = 0;
    mState = State::Idle;

    // Reset speed to zero
    mLastSetSpeed = mTargetSpeed = SpeedPoint();
}

void Train::timerEvent(QTimerEvent *e)
{
    if(e->timerId() == mApplySpeedTimerId)
    {
        applyDelayedSpeed();
        return;
    }
    else if(e->timerId() == mAccelerationTimerId)
    {
        updateAccelerationState();
        return;
    }

    QObject::timerEvent(e);
}

void Train::startDelayedSpeedApply(int locoIdx)
{
    if(mApplySpeedTimerId && mApplySpeedLocoIdx != locoIdx)
    {
        // Source loco changed, apply now
        applyDelayedSpeed();
    }

    // Start new timer
    mApplySpeedLocoIdx = locoIdx;

    killTimer(mApplySpeedTimerId);
    mApplySpeedTimerId = startTimer(600);
}

void Train::stopDelayedSpeedApply()
{
    mApplySpeedLocoIdx = INVALID_LOCO_IDX;
    killTimer(mApplySpeedTimerId);
    mApplySpeedTimerId = 0;
}

void Train::applyDelayedSpeed()
{
    int locoIdx = mApplySpeedLocoIdx;
    stopDelayedSpeedApply();

    if(locoIdx == INVALID_LOCO_IDX)
        return;

    const auto& entry = mSpeedTable.getEntryAt(mLastSetSpeed.tableIdx);
    int step = entry.getStepForLoco(locoIdx);

    driveLoco(locoIdx, step);
}

void Train::onLocoChangedInternal(int locoIdx, int step)
{
    if(!active)
        return;

    const TrainSpeedTable::Entry& maxSpeedEntry = mSpeedTable.getEntryAt(mMaxSpeed.tableIdx);
    int maxLocoStep = maxSpeedEntry.getStepForLoco(locoIdx);
    if(step > maxLocoStep)
    {
        // Locomotive exceeded Train max speed, revert immediately
        driveLoco(locoIdx, maxLocoStep);

        setTargetSpeedInternal(mMaxSpeed.speed, mMaxSpeed.tableIdx);

        return;
    }

    auto match = mSpeedTable.getClosestMatch(locoIdx, step);

    SpeedPoint speedPoint;
    speedPoint.speed = match.second.avgSpeed;
    speedPoint.tableIdx = match.first;

    bool needsDelay = false;
    const int newStep = match.second.getStepForLoco(locoIdx);

    if(newStep != step)
    {
        // Requested step was adjusted
        // This could prevent setting speed going up one by one

        int oldStep = mLocomotives.at(locoIdx).lastSetStep;

        if(mLastSetSpeed.tableIdx == match.first)
        {
            // We would get rounded back to previous state
            if(qAbs(oldStep - step) > 3)
            {
                // It takes more than +3 steps to reach next tableIdx
                // We use 3 as threshold to ignore spurious rotary knob changes
                // We help user reach next tableIdx before delay timeout triggers
                // In fact it's hard to go more than 4 steps up before delaied
                // step apply triggers.
                int newTableIdx = mLastSetSpeed.tableIdx;
                if(step > oldStep)
                    newTableIdx++;
                else
                    newTableIdx--;

                // Update speed point
                speedPoint.tableIdx = newTableIdx;
                speedPoint.speed = mSpeedTable.getEntryAt(newTableIdx).avgSpeed;
            }
            else if(oldStep < step && step > newStep ||
                    oldStep > step && step < newStep)
            {
                // We would override user requested speed
                // with original speed, give user some time
                // to set higher/lower step
                needsDelay = true;
            }
        }
        else if(abs(mLastSetSpeed.tableIdx - match.first) == 1)
        {
            // User managed to get to next/prev tableIdx but still got
            // adjusted. Give some extra time if trend got inverted
            if(oldStep < step && step > newStep ||
                    oldStep > step && step < newStep)
            {
                needsDelay = true;
            }
        }
    }

    if(needsDelay)
    {
        // Keep step set by user
        mLocomotives[locoIdx].lastSetStep = newStep;
    }
    else
    {
        // We will accelerate/brake up to requested step
        // But first reset to last set step
        // This is needed so all locomotives start accelerating/braking
        // At same speed, otherwise this locomotive would accelerate faster
        // or brake faster because it's the first one to receive commands
        // and would be out of sync with the others.
        const auto& entry = mSpeedTable.getEntryAt(mLastSetSpeed.tableIdx);
        int step = entry.getStepForLoco(locoIdx);
        driveLoco(locoIdx, step);
    }

    setTargetSpeedInternal(speedPoint.speed, speedPoint.tableIdx,
                           needsDelay ? locoIdx : INVALID_LOCO_IDX);
}

void Train::setTargetSpeedInternal(double speed, int tableIdx, int sourceLocoIdx)
{
    if(tableIdx == TrainSpeedTable::NULL_TABLE_ENTRY && !qFuzzyIsNull(speed))
    {
        auto match = mSpeedTable.getClosestMatch(speed);
        speed = match.second.avgSpeed;
        tableIdx = match.first;
    }

    if(tableIdx > mMaxSpeed.tableIdx)
    {
        speed = mMaxSpeed.speed;
        tableIdx = mMaxSpeed.tableIdx;
    }

    mTargetSpeed.speed = speed;
    mTargetSpeed.tableIdx = tableIdx;

    if(sourceLocoIdx != INVALID_LOCO_IDX)
        startDelayedSpeedApply(sourceLocoIdx);

    if(mTargetSpeed.tableIdx > mLastSetSpeed.tableIdx)
    {
        if(mState == State::Accelerating)
        {
            // Keep accelerating
        }
        else
        {
            double currentSpeed = mLastSetSpeed.speed;
            int nextTableIdx = mLastSetSpeed.tableIdx + 1;

            if(mState == State::Braking)
            {
                // We start from under last set speed
                int millis = mAccelerationElapsed.elapsed();
                double deltaSpeed = mDecelerationRate * double(millis) / 1000.0;
                currentSpeed = mLastSetSpeed.speed - deltaSpeed;
                nextTableIdx = mLastSetSpeed.tableIdx;
            }

            scheduleAccelerationFrom(currentSpeed,
                                     nextTableIdx,
                                     State::Accelerating);
        }
    }
    else if(mTargetSpeed.tableIdx < mLastSetSpeed.tableIdx)
    {
        if(mState == State::Braking)
        {
            // Keep braking
        }
        else
        {
            double currentSpeed = mLastSetSpeed.speed;
            int prevTableIdx = mLastSetSpeed.tableIdx - 1;

            if(mState == State::Accelerating)
            {
                // We start from above last set speed
                int millis = mAccelerationElapsed.elapsed();
                double deltaSpeed = mAccelerationRate * double(millis) / 1000.0;
                currentSpeed = mLastSetSpeed.speed + deltaSpeed;
                prevTableIdx = mLastSetSpeed.tableIdx;
            }

            scheduleAccelerationFrom(currentSpeed,
                                     prevTableIdx,
                                     State::Braking);
        }
    }
    else if(mState != State::Idle)
    {
        // Cancel current acceleration
        mState = State::Idle;
        killTimer(mAccelerationTimerId);
        mAccelerationTimerId = 0;

        // Force setting speed
        setSpeedInternal(mTargetSpeed);
    }
}

void Train::scheduleAccelerationFrom(double currentSpeed, int newTableIdx, State state)
{
    // Calculate seconds to previous table index
    double newSpeed = mSpeedTable.getEntryAt(newTableIdx).avgSpeed;

    const double deltaSpeed = newSpeed - currentSpeed;
    double accelRate = mAccelerationRate;
    if(state == State::Braking)
    {
        // Sign is inverted because speed is going down
        accelRate = -mDecelerationRate;
    }

    const double deltaSeconds = deltaSpeed / accelRate;
    const int millis = qCeil(deltaSeconds * 1000.0);

    mState = state;
    killTimer(mAccelerationTimerId);
    mAccelerationTimerId = startTimer(millis, Qt::PreciseTimer);
    mAccelerationElapsed.start();
}

void Train::updateAccelerationState()
{
    killTimer(mAccelerationTimerId);
    mAccelerationTimerId = 0;

    int newTableIdx = mLastSetSpeed.tableIdx;
    if(mState == State::Accelerating)
    {
        newTableIdx++;
    }
    else if(mState == State::Braking)
    {
        newTableIdx--;
    }
    else
        return;

    const auto& entry = mSpeedTable.getEntryAt(newTableIdx);
    SpeedPoint newSpeed;
    newSpeed.speed = entry.avgSpeed;
    newSpeed.tableIdx = newTableIdx;

    setSpeedInternal(newSpeed);

    if(mTargetSpeed.tableIdx > mLastSetSpeed.tableIdx)
    {
        // Keep accelerating
        double currentSpeed = mLastSetSpeed.speed;
        int nextTableIdx = mLastSetSpeed.tableIdx + 1;

        scheduleAccelerationFrom(currentSpeed,
                                 nextTableIdx,
                                 State::Accelerating);
    }
    else if(mTargetSpeed.tableIdx < mLastSetSpeed.tableIdx)
    {
        // Keep braking
        double currentSpeed = mLastSetSpeed.speed;
        int prevTableIdx = mLastSetSpeed.tableIdx - 1;

        scheduleAccelerationFrom(currentSpeed,
                                 prevTableIdx,
                                 State::Braking);
    }
    else
    {
        // We reached target speed
        mState = State::Idle;
    }
}

void Train::setSpeedInternal(const SpeedPoint &speedPoint)
{
    mLastSetSpeed = speedPoint;
    const auto& entry = mSpeedTable.getEntryAt(speedPoint.tableIdx);

    // Apply speed to all locomotives
    for(int i = 0; i < mLocomotives.size(); i++)
    {
        if(mApplySpeedLocoIdx == i)
            continue; // This loco has delayed apply scheduled

        const LocoItem& item = mLocomotives.at(i);

        LocomotiveDirection locoDir = mDirection;
        if(item.invertDir)
            locoDir = oppositeDir(locoDir);

        int step = entry.getStepForLoco(i);

        if(item.loco->targetSpeedStep() != step || item.loco->targetDirection() != locoDir)
        {
            driveLoco(i, step);
        }
    }
}

void Train::driveLoco(int locoIdx, int step)
{
    LocoItem& item = mLocomotives[locoIdx];
    item.lastSetStep = step;

    LocomotiveDirection locoDir = mDirection;
    if(item.invertDir)
        locoDir = oppositeDir(locoDir);

    item.loco->driveLoco(step, locoDir);
}

bool Train::setActive(bool newActive)
{
    if(newActive == active)
        return true;

    if(!newActive)
    {
        // Prevent deactivation if train is still moving
        if(mLastSetSpeed.tableIdx != TrainSpeedTable::NULL_TABLE_ENTRY)
            return false;
    }

    active = newActive;
    if(active)
    {
        updateSpeedTable();
        setEmergencyStop();
        mLastSetSpeed = mTargetSpeed = SpeedPoint();
        setDirection(mDirection); // Force update direction
    }
    else
        stopDelayedSpeedApply();

    emit activeChanged(active);

    return true;
}

void Train::setLocoInvertDir(int idx, bool invertDir)
{
    if(idx < 0 || idx > mLocomotives.size())
        return;

    mLocomotives[idx].invertDir = invertDir;
}

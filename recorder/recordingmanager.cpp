#include "recordingmanager.h"

#include "locomotiverecording.h"

#include "../input/ispeedsensor.h"
#include "../commandstation/icommandstation.h"

#include <QTimerEvent>

RecordingManager::RecordingManager(QObject *parent)
    : QObject{parent}
{
    m_currentRecording = new LocomotiveRecording(this);
}

RecordingManager::~RecordingManager()
{
    stop();
}

void RecordingManager::onNewSpeedReading(double metersPerSecond, LocomotiveDirection direction, qint64 timestampMilliSec)
{
    if(!m_currentRecording)
        return;

    RecordingItem item;
    item.timestampMilliSec = timestampMilliSec;
    item.actualSpeedStep = actualDCCStep;
    item.requestedSpeedStep = requestedDCCStep;
    item.metersPerSecond = metersPerSecond;
    item.metersPerSecondAvg = 0; // Will be calculated later

    //TODO: direction

    m_currentRecording->addItem(item);
}

void RecordingManager::onLocomotiveSpeedFeedback(int address, int speedStep, LocomotiveDirection direction)
{
    if(address != locomotiveDCCAddress)
        return;

    actualDCCStep = speedStep;
    //TODO: direction
}

LocomotiveRecording *RecordingManager::currentRecording() const
{
    return m_currentRecording;
}

void RecordingManager::timerEvent(QTimerEvent *e)
{
    if(e->timerId() == mTimerId)
    {
        requestedDCCStep++;
        if(requestedDCCStep > 126)
        {
            stop();
            return;
        }

        if(mCommandStation)
        {
            mCommandStation->setLocomotiveSpeed(locomotiveDCCAddress,
                                                requestedDCCStep,
                                                LocomotiveDirection::Forward);
        }
        return;
    }

    QObject::timerEvent(e);
}

void RecordingManager::start()
{
    stop();

    m_currentRecording->clear();

    requestedDCCStep = 0;
    if(mCommandStation)
    {
        mCommandStation->setLocomotiveSpeed(locomotiveDCCAddress,
                                            requestedDCCStep,
                                            LocomotiveDirection::Forward);
    }

    mTimerId = startTimer(1500);
}

void RecordingManager::stop()
{
    if(mTimerId)
    {
        killTimer(mTimerId);
        mTimerId = 0;
    }
}

ISpeedSensor *RecordingManager::speedSensor() const
{
    return mSpeedSensor;
}

void RecordingManager::setSpeedSensor(ISpeedSensor *newSpeedSensor)
{
    if(mSpeedSensor)
        disconnect(mSpeedSensor, &ISpeedSensor::speedReading, this, &RecordingManager::onNewSpeedReading);

    mSpeedSensor = newSpeedSensor;

    if(mSpeedSensor)
        connect(mSpeedSensor, &ISpeedSensor::speedReading, this, &RecordingManager::onNewSpeedReading);
}

ICommandStation *RecordingManager::commandStation() const
{
    return mCommandStation;
}

void RecordingManager::setCommandStation(ICommandStation *newCommandStation)
{
    if(mCommandStation)
        disconnect(mCommandStation, &ICommandStation::locomotiveSpeedFeedback, this, &RecordingManager::onLocomotiveSpeedFeedback);

    mCommandStation = newCommandStation;

    if(mCommandStation)
        connect(mCommandStation, &ICommandStation::locomotiveSpeedFeedback, this, &RecordingManager::onLocomotiveSpeedFeedback);
}

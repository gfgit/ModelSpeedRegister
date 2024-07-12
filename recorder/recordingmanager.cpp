#include "recordingmanager.h"

#include "locomotiverecording.h"

#include "../input/ispeedsensor.h"
#include "../commandstation/icommandstation.h"

#include "idataseries.h"
#include "series/requestedspeedstepseries.h"
#include "series/receivedspeedstepseries.h"

#include <QTimerEvent>

RecordingManager::RecordingManager(QObject *parent)
    : QObject{parent}
{
    m_currentRecording = new LocomotiveRecording(this);

    mReqStepSeries = new RequestedSpeedStepSeries(this);
    registerSeries(mReqStepSeries);

    mRecvStepSeries = new ReceivedSpeedStepSeries(this);
    registerSeries(mRecvStepSeries);
}

RecordingManager::~RecordingManager()
{
    stop();
}

void RecordingManager::onNewSpeedReading(double metersPerSecond, double travelledMillimeters, qint64 timestampMilliSec)
{
    if(!m_currentRecording)
        return;

    if(mStartTimestamp == -1)
    {
        // Reset time reference
        mStartTimestamp = timestampMilliSec;
    }

    timestampMilliSec -= mStartTimestamp;

    qDebug() << "READ:" << timestampMilliSec << metersPerSecond << travelledMillimeters;

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

    mRecvStepSeries->addPoint(speedStep, mElapsed.elapsed() / 1000.0);
    //TODO: direction
}

void RecordingManager::onSeriesDestroyed(QObject *s)
{
    IDataSeries *series = static_cast<IDataSeries *>(s);
    mSeries.removeOne(series);
    emit seriesUnregistered(series);
}

ReceivedSpeedStepSeries *RecordingManager::recvStepSeries() const
{
    return mRecvStepSeries;
}

RequestedSpeedStepSeries *RecordingManager::reqStepSeries() const
{
    return mReqStepSeries;
}

QVector<IDataSeries *> RecordingManager::getSeries() const
{
    return mSeries;
}

void RecordingManager::registerSeries(IDataSeries *s)
{
    if(mSeries.contains(s))
        return;

    connect(s, &IDataSeries::destroyed, this, &RecordingManager::onSeriesDestroyed);

    mSeries.append(s);
    emit seriesRegistered(s);
}

void RecordingManager::unregisterSeries(IDataSeries *s)
{
    if(!mSeries.contains(s))
        return;

    disconnect(s, &IDataSeries::destroyed, this, &RecordingManager::onSeriesDestroyed);

    mSeries.removeOne(s);
    emit seriesUnregistered(s);
}

int RecordingManager::getLocomotiveDCCAddress() const
{
    return locomotiveDCCAddress;
}

void RecordingManager::setLocomotiveDCCAddress(int newLocomotiveDCCAddress)
{
    locomotiveDCCAddress = newLocomotiveDCCAddress;
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
            mReqStepSeries->addPoint(requestedDCCStep, mElapsed.elapsed() / 1000.0);

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

    mRecvStepSeries->clear();
    mReqStepSeries->clear();

    m_currentRecording->clear();

    mTimerId = startTimer(10000);
    mStartTimestamp = -1;

    mElapsed.start();
}

void RecordingManager::stop()
{
    if(mTimerId)
    {
        killTimer(mTimerId);
        mTimerId = 0;
    }

    // Stop the locomotive
    requestedDCCStep = 0;
    if(mCommandStation)
    {
        mCommandStation->setLocomotiveSpeed(locomotiveDCCAddress,
                                            requestedDCCStep,
                                            LocomotiveDirection::Forward);
    }
}

void RecordingManager::emergencyStop()
{
    stop();

    // Stop the locomotive
    if(mCommandStation)
    {
        mCommandStation->emergencyStop(locomotiveDCCAddress);
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

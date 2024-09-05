#include "recordingmanager.h"

#include "../input/ispeedsensor.h"
#include "../commandstation/icommandstation.h"

#include "idataseries.h"
#include "series/requestedspeedstepseries.h"
#include "series/receivedspeedstepseries.h"
#include "series/rawsensordataseries.h"
#include "series/sensortravelleddistanceseries.h"
#include "series/dataseriescurvemapping.h"

#include <QTimerEvent>

#include <QDebug>

RecordingManager::RecordingManager(QObject *parent)
    : QObject{parent}
{
    mReqStepSeries = new RequestedSpeedStepSeries(this);
    registerSeries(mReqStepSeries);

    mRecvStepSeries = new ReceivedSpeedStepSeries(this);
    registerSeries(mRecvStepSeries);

    mRawSensorSeries = new RawSensorDataSeries(this);
    registerSeries(mRawSensorSeries);

    mSensorTravelledSeries = new SensorTravelledDistanceSeries(this);
    registerSeries(mSensorTravelledSeries);
}

RecordingManager::~RecordingManager()
{
    stop();
    stopInternal();
}

void RecordingManager::onNewSpeedReading(double metersPerSecond, double travelledMillimeters, qint64 timestampMilliSec)
{
    if(mState == State::Stopped)
        return;

    if(mStartTimestamp == -1)
    {
        // Reset time reference
        mStartTimestamp = timestampMilliSec;
    }

    timestampMilliSec -= mStartTimestamp;

    mRawSensorSeries->addPoint(metersPerSecond, timestampMilliSec / 1000.0);
    mSensorTravelledSeries->addPoint(travelledMillimeters, timestampMilliSec / 1000.0);

    qDebug() << "READ:" << timestampMilliSec << metersPerSecond << travelledMillimeters;

    //TODO: direction

    if(mState == State::WaitingToStop)
        tryStopInternal();
}

void RecordingManager::onLocomotiveSpeedFeedback(int address, int speedStep, LocomotiveDirection direction)
{
    if(mState == State::Stopped || address != locomotiveDCCAddress)
        return;

    int oldRecvStep = actualDCCStep;
    actualDCCStep = speedStep;

    if(mReqStepIsPending && actualDCCStep == requestedDCCStep)
        mReqStepIsPending = false;

    mRecvStepSeries->addPoint(oldRecvStep, mElapsed.elapsed() / 1000.0);
    mRecvStepSeries->addPoint(actualDCCStep, mElapsed.elapsed() / 1000.0);

    //TODO: direction

    if(mState == State::WaitingToStop)
        tryStopInternal();
}

void RecordingManager::onSeriesDestroyed(QObject *s)
{
    IDataSeries *series = static_cast<IDataSeries *>(s);
    mSeries.removeOne(series);
    emit seriesUnregistered(series);
}

void RecordingManager::requestStepInternal(int step)
{
    int oldStep = requestedDCCStep;
    requestedDCCStep = step;

    if(mCommandStation)
    {
        mReqStepSeries->addPoint(oldStep, mElapsed.elapsed() / 1000.0);
        mReqStepSeries->addPoint(requestedDCCStep, mElapsed.elapsed() / 1000.0);

        mReqStepIsPending = true;
        mCommandStation->setLocomotiveSpeed(locomotiveDCCAddress,
                                            requestedDCCStep,
                                            LocomotiveDirection::Forward);
    }
}

RecordingManager::State RecordingManager::state() const
{
    return mState;
}

void RecordingManager::setState(State newState)
{
    mState = newState;
    emit stateChanged(int(mState));
}

void RecordingManager::tryStopInternal()
{
    if(mState != State::WaitingToStop || mReqStepIsPending)
        return; // Command station reply is still pending

    QPointF lastSensorRead = mRawSensorSeries->getPointAt(mRawSensorSeries->getPointCount() - 1);
    QPointF lastReq = mReqStepSeries->getPointAt(mReqStepSeries->getPointCount() - 1);
    if(lastSensorRead.x() < lastReq.x())
        return; // Wait for last sensor data

    // We received all data until our last requested step
    stopInternal();
}

int RecordingManager::defaultStepTimeMillis() const
{
    return mDefaultStepTimeMillis;
}

void RecordingManager::setDefaultStepTimeMillis(int newDefaultStepTimeMillis)
{
    if((newDefaultStepTimeMillis < 1000) || (newDefaultStepTimeMillis > 10000))
        return;
    mDefaultStepTimeMillis = newDefaultStepTimeMillis;
}

void RecordingManager::goToNextStep()
{
    if(mState != State::Running)
        return;

    // Restart timer for next step
    killTimer(mStepTimerId);
    mStepTimerId = startTimer(mDefaultStepTimeMillis);
    currentTimerIsCustom = false;

    // Go to next step directly
    requestStep(requestedDCCStep + 1);
}

void RecordingManager::setCustomTimeForCurrentStep(int millis)
{
    if(mState != State::Running)
        return;

    // Restart timer for current step
    killTimer(mStepTimerId);
    mStepTimerId = startTimer(millis);
    currentTimerIsCustom = true;
}

int RecordingManager::startingDCCStep() const
{
    return mStartingDCCStep;
}

void RecordingManager::setStartingDCCStep(int newStartingDCCStep)
{
    if(newStartingDCCStep == 0)
        newStartingDCCStep = 1;

    if(newStartingDCCStep >= 126)
        return;
    mStartingDCCStep = newStartingDCCStep;
}

ReceivedSpeedStepSeries *RecordingManager::recvStepSeries() const
{
    return mRecvStepSeries;
}

RequestedSpeedStepSeries *RecordingManager::reqStepSeries() const
{
    return mReqStepSeries;
}

RawSensorDataSeries *RecordingManager::rawSensorSeries() const
{
    return mRawSensorSeries;
}

SensorTravelledDistanceSeries *RecordingManager::sensorTravelledSeries() const
{
    return mSensorTravelledSeries;
}

void RecordingManager::requestStep(int step)
{
    if(step > 126)
    {
        stop();
        return;
    }

    requestStepInternal(step);
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

    switch (s->getType())
    {
    case DataSeriesType::SensorRawData:
    case DataSeriesType::MovingAverage:
    case DataSeriesType::TotalStepAverage:
    {
        // Inject a mapping
        DataSeriesCurveMapping *mapping = new DataSeriesCurveMapping(this);
        mapping->setRecvStep(mRecvStepSeries);
        mapping->setSource(s);
        registerSeries(mapping);
        break;
    }
    default:
        break;
    }
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
    if(locomotiveDCCAddress <= 0 || locomotiveDCCAddress > 9999)
        return;
    locomotiveDCCAddress = newLocomotiveDCCAddress;
}

void RecordingManager::timerEvent(QTimerEvent *e)
{
    if(e->timerId() == mStepTimerId && mStepTimerId)
    {
        requestStep(requestedDCCStep + 1);

        if(currentTimerIsCustom)
        {
            // Reset to default timer for next step
            killTimer(mStepTimerId);
            mStepTimerId = startTimer(mDefaultStepTimeMillis);
            currentTimerIsCustom = false;
        }
        return;
    }
    else if(e->timerId() == mForceStopTimerId)
    {
        stopInternal();
        return;
    }

    QObject::timerEvent(e);
}

bool RecordingManager::start()
{
    if(mState != State::Stopped)
    {
        stop();
        return false;
    }

    mRecvStepSeries->clear();
    mReqStepSeries->clear();
    mRawSensorSeries->clear();
    mSensorTravelledSeries->clear();

    actualDCCStep = 0;
    requestedDCCStep = 0;
    mReqStepIsPending = false;

    mStartTimestamp = -1;

    currentTimerIsCustom = false;
    mStepTimerId = startTimer(mDefaultStepTimeMillis);

    mElapsed.start();

    setState(State::Running);
    requestStep(mStartingDCCStep);

    return true;
}

void RecordingManager::stop()
{
    if(mState != State::Running)
        return;

    if(mStepTimerId)
    {
        killTimer(mStepTimerId);
        mStepTimerId = 0;
        currentTimerIsCustom = false;
    }

    setState(State::WaitingToStop);

    // Wait a bit to receive last sensor readings
    if(mForceStopTimerId)
    {
        killTimer(mForceStopTimerId);
        mForceStopTimerId = 0;
    }
    mForceStopTimerId = startTimer(5000);

    // Stop the locomotive
    requestStepInternal(0);
}

void RecordingManager::stopInternal()
{
    if(mState != State::WaitingToStop)
        return;

    if(mForceStopTimerId)
    {
        killTimer(mForceStopTimerId);
        mForceStopTimerId = 0;
    }

    setState(State::Stopped);
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

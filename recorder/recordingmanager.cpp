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
}

void RecordingManager::onNewSpeedReading(double metersPerSecond, double travelledMillimeters, qint64 timestampMilliSec)
{
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
}

void RecordingManager::onLocomotiveSpeedFeedback(int address, int speedStep, LocomotiveDirection direction)
{
    if(address != locomotiveDCCAddress)
        return;

    int oldRecvStep = actualDCCStep;
    actualDCCStep = speedStep;

    mRecvStepSeries->addPoint(oldRecvStep, mElapsed.elapsed() / 1000.0);
    mRecvStepSeries->addPoint(actualDCCStep, mElapsed.elapsed() / 1000.0);

    //TODO: direction
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

        mCommandStation->setLocomotiveSpeed(locomotiveDCCAddress,
                                            requestedDCCStep,
                                            LocomotiveDirection::Forward);
    }
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
    locomotiveDCCAddress = newLocomotiveDCCAddress;
}

void RecordingManager::timerEvent(QTimerEvent *e)
{
    if(e->timerId() == mTimerId)
    {
        requestStep(requestedDCCStep + 1);
        return;
    }

    QObject::timerEvent(e);
}

void RecordingManager::start()
{
    stop();

    mRecvStepSeries->clear();
    mReqStepSeries->clear();
    mRawSensorSeries->clear();
    mSensorTravelledSeries->clear();

    actualDCCStep = 0;
    requestedDCCStep = 0;

    mTimerId = startTimer(5000);
    mStartTimestamp = -1;

    mElapsed.start();

    requestStep(requestedDCCStep + 1);
}

void RecordingManager::stop()
{
    if(mTimerId)
    {
        killTimer(mTimerId);
        mTimerId = 0;
    }

    // Stop the locomotive
    requestStepInternal(0);
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

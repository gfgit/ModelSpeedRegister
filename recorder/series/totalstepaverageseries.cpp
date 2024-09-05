#include "totalstepaverageseries.h"

TotalStepAverageSeries::TotalStepAverageSeries(QObject *parent)
    : IDataSeries{parent}
    , mTravelledSource(nullptr)
    , mRecvStepSeries(nullptr)
    , mReqStepSeries(nullptr)
{
    setName(tr("Total Step Avg"));
}

void TotalStepAverageSeries::onRecvStepPointAdded(int indexAdded, const QPointF &point)
{
    if(indexAdded % 2 == 1)
    {
        // Odd Indexes are start steps
        lastRecvStepIdx = calculateAvg(lastRecvStepIdx);
    }
}

void TotalStepAverageSeries::onSensorPointAdded(int indexAdded, const QPointF &point)
{
    if(waitingForMoreSensorData)
    {
        // Continue previously paused calculation
        lastRecvStepIdx = calculateAvg(lastRecvStepIdx);
    }
}

void TotalStepAverageSeries::onReqStepPointAdded(int indexAdded, const QPointF &point)
{
    if(waitingForRequestEnd && indexAdded % 2 == 0)
    {
        // Even Indexes are end steps
        lastRecvStepIdx = calculateAvg(lastRecvStepIdx);
    }
}

void TotalStepAverageSeries::onPointChanged()
{
    recalculate();
}

void TotalStepAverageSeries::onPointRemoved()
{
    recalculate();
}

void TotalStepAverageSeries::onSourceDestroyed(QObject *source)
{
    if(source == mRecvStepSeries)
        mRecvStepSeries = nullptr;
    else if(source == mReqStepSeries)
        mReqStepSeries = nullptr;
    else if(source == mTravelledSource)
        mTravelledSource = nullptr;
    else
        return; // Doesn't belong to us

    int oldSize = mPoints.size();
    for(int i = oldSize - 1; i >= 0; i--)
    {
        emit pointRemoved(i);
    }
    mPoints.clear();
}

void TotalStepAverageSeries::updateAvg(int indexChanged, const QPointF &point, DataSeriesAction action)
{
    // TODO: optimize
}

void TotalStepAverageSeries::recalculate()
{
    // Clear previous average
    int oldSize = mPoints.size();
    for(int i = oldSize - 1; i >= 0; i--)
    {
        emit pointRemoved(i);
    }
    mPoints.clear();

    waitingForMoreSensorData = false;
    waitingForRequestEnd = false;

    if(!mTravelledSource || !mReqStepSeries || !mRecvStepSeries)
    {
        return;
    }

    // Recalculate from start
    lastRecvStepIdx = calculateAvg(1);
}

int TotalStepAverageSeries::calculateAvg(int fromRecvIdx)
{
    if(!mTravelledSource || !mReqStepSeries || !mRecvStepSeries)
    {
        return 0;
    }

    int travelledIdx = 0;
    int recvStepIdx = fromRecvIdx;
    if(recvStepIdx % 2 != 1)
        recvStepIdx++; // Always use Start received step (Odd indexes)

    for(; recvStepIdx < mRecvStepSeries->getPointCount(); recvStepIdx += 2)
    {
        // For each step we have 2 points (start, end)
        // Take received start time up to next requested step time

        QPointF recvStart = mRecvStepSeries->getPointAt(recvStepIdx);

        QPointF reqEnd;
        bool hasEnd = false;
        for(int reqStepIdx = qMax(0, recvStepIdx - 3); reqStepIdx < mRecvStepSeries->getPointCount(); reqStepIdx += 2)
        {
            // Even indexes are requested steps end (Just before start of next requested step)
            reqEnd = mReqStepSeries->getPointAt(reqStepIdx);
            if(qFuzzyCompare(reqEnd.y(), recvStart.y()))
            {
                hasEnd = true;
                break;
            }
        }


        if(!hasEnd)
        {
            // Wait for request end
            waitingForRequestEnd = true;
            return recvStepIdx;
        }

        qint64 millisStart = recvStart.x() * 1000 + mAccelerationMilliseconds;
        qint64 millisEnd = reqEnd.x() * 1000;

        if(millisStart > millisEnd)
            continue;

        QPointF travelledStart, travelledEnd;
        bool startFound = false;

        for(; travelledIdx < mTravelledSource->getPointCount(); travelledIdx++)
        {
            QPointF pt = mTravelledSource->getPointAt(travelledIdx);
            qint64 ptMillis = pt.x() * 1000;

            if(!startFound && ptMillis >= millisStart)
            {
                travelledStart = pt;
                startFound = true;
            }

            if(ptMillis > millisEnd)
                break;

            travelledEnd = pt;
        }

        if(travelledIdx >= mTravelledSource->getPointCount())
        {
            // Wait for more sensor data to arrive
            waitingForMoreSensorData = true;
            return recvStepIdx;
        }

        if(!startFound || travelledStart == travelledEnd)
            continue;

        double deltaMillimeters = travelledEnd.y() - travelledStart.y();
        double deltaMilliseconds = (travelledEnd.x() - travelledStart.x()) * 1000.0;
        double avgMetersPerSecond = deltaMillimeters / deltaMilliseconds;

        QPointF start = travelledStart;
        start.ry() = avgMetersPerSecond;

        QPointF end = travelledEnd;
        end.ry() = avgMetersPerSecond;

        mPoints.append(start);
        emit pointAdded(mPoints.size() - 1, start);

        mPoints.append(end);
        emit pointAdded(mPoints.size() - 1, end);
    }

    return recvStepIdx;
}

qint64 TotalStepAverageSeries::accelerationMilliseconds() const
{
    return mAccelerationMilliseconds;
}

void TotalStepAverageSeries::setAccelerationMilliseconds(qint64 newAccelerationMilliseconds)
{
    mAccelerationMilliseconds = newAccelerationMilliseconds;
    recalculate();
}

IDataSeries *TotalStepAverageSeries::recvStepSeries() const
{
    return mRecvStepSeries;
}

void TotalStepAverageSeries::setRecvStepSeries(IDataSeries *newRecvStepSeries)
{
    if(mRecvStepSeries)
    {
        disconnect(mRecvStepSeries, &IDataSeries::pointAdded, this, &TotalStepAverageSeries::onRecvStepPointAdded);
        disconnect(mRecvStepSeries, &IDataSeries::pointChanged, this, &TotalStepAverageSeries::onPointChanged);
        disconnect(mRecvStepSeries, &IDataSeries::pointRemoved, this, &TotalStepAverageSeries::onPointRemoved);
        disconnect(mRecvStepSeries, &QObject::destroyed, this, &TotalStepAverageSeries::onSourceDestroyed);
    }

    mRecvStepSeries = newRecvStepSeries;

    if(mRecvStepSeries)
    {
        connect(mRecvStepSeries, &IDataSeries::pointAdded, this, &TotalStepAverageSeries::onRecvStepPointAdded);
        connect(mRecvStepSeries, &IDataSeries::pointChanged, this, &TotalStepAverageSeries::onPointChanged);
        connect(mRecvStepSeries, &IDataSeries::pointRemoved, this, &TotalStepAverageSeries::onPointRemoved);
        connect(mRecvStepSeries, &QObject::destroyed, this, &TotalStepAverageSeries::onSourceDestroyed);
    }

    recalculate();
}

IDataSeries *TotalStepAverageSeries::reqStepSeries() const
{
    return mReqStepSeries;
}

void TotalStepAverageSeries::setReqStepSeries(IDataSeries *newReqStepSeries)
{
    if(mReqStepSeries)
    {
        disconnect(mReqStepSeries, &IDataSeries::pointAdded, this, &TotalStepAverageSeries::onReqStepPointAdded);
        disconnect(mReqStepSeries, &IDataSeries::pointChanged, this, &TotalStepAverageSeries::onPointChanged);
        disconnect(mReqStepSeries, &IDataSeries::pointRemoved, this, &TotalStepAverageSeries::onPointRemoved);
        disconnect(mReqStepSeries, &QObject::destroyed, this, &TotalStepAverageSeries::onSourceDestroyed);
    }

    mReqStepSeries = newReqStepSeries;

    if(mReqStepSeries)
    {
        connect(mReqStepSeries, &IDataSeries::pointAdded, this, &TotalStepAverageSeries::onReqStepPointAdded);
        connect(mReqStepSeries, &IDataSeries::pointChanged, this, &TotalStepAverageSeries::onPointChanged);
        connect(mReqStepSeries, &IDataSeries::pointRemoved, this, &TotalStepAverageSeries::onPointRemoved);
        connect(mReqStepSeries, &QObject::destroyed, this, &TotalStepAverageSeries::onSourceDestroyed);
    }

    recalculate();
}

IDataSeries *TotalStepAverageSeries::travelledSource() const
{
    return mTravelledSource;
}

void TotalStepAverageSeries::setTravelledSource(IDataSeries *newSource)
{
    if(mTravelledSource)
    {
        disconnect(mTravelledSource, &IDataSeries::pointAdded, this, &TotalStepAverageSeries::onSensorPointAdded);
        disconnect(mTravelledSource, &IDataSeries::pointChanged, this, &TotalStepAverageSeries::onPointChanged);
        disconnect(mTravelledSource, &IDataSeries::pointRemoved, this, &TotalStepAverageSeries::onPointRemoved);
        disconnect(mTravelledSource, &QObject::destroyed, this, &TotalStepAverageSeries::onSourceDestroyed);
    }

    mTravelledSource = newSource;

    if(mTravelledSource)
    {
        connect(mTravelledSource, &IDataSeries::pointAdded, this, &TotalStepAverageSeries::onSensorPointAdded);
        connect(mTravelledSource, &IDataSeries::pointChanged, this, &TotalStepAverageSeries::onPointChanged);
        connect(mTravelledSource, &IDataSeries::pointRemoved, this, &TotalStepAverageSeries::onPointRemoved);
        connect(mTravelledSource, &QObject::destroyed, this, &TotalStepAverageSeries::onSourceDestroyed);
    }

    recalculate();
}

DataSeriesType TotalStepAverageSeries::getType() const
{
    return DataSeriesType::TotalStepAverage;
}

int TotalStepAverageSeries::getPointCount() const
{
    return mPoints.size();
}

QPointF TotalStepAverageSeries::getPointAt(int index) const
{
    return mPoints.value(index, QPointF());
}

QString TotalStepAverageSeries::getPointTooltip(int index) const
{
    if(!mTravelledSource || index < 0 || index >= mPoints.size())
        return QString();


    QString fullName = name() + QLatin1String(" (%1)").arg(mTravelledSource->name());
    return IDataSeries::defaultTooltip(fullName, index, mPoints.at(index));
}

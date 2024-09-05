#include "dataseriescurvemapping.h"

DataSeriesCurveMapping::DataSeriesCurveMapping(QObject *parent)
    : IDataSeries{parent}
    , mSource(nullptr)
    , mRecvStepSeries(nullptr)
{
    setName(tr("Mapping"));
}

void DataSeriesCurveMapping::onSourcePointAdded(int indexAdded, const QPointF &point)
{
    mLastSourceIdx = calculateAvg(mLastSourceIdx);
}

void DataSeriesCurveMapping::onSourceDestroyed(QObject *source)
{
    if(source == mSource)
        mSource = nullptr;
    else if(source == mRecvStepSeries)
        mRecvStepSeries = nullptr;
    else
        return;

    int oldSize = mPoints.size();
    for(int i = oldSize - 1; i >= 0; i--)
    {
        emit pointRemoved(i);
    }
    mPoints.clear();
}

void DataSeriesCurveMapping::recalculate()
{
    int oldSize = mPoints.size();
    for(int i = oldSize - 1; i >= 0; i--)
    {
        emit pointRemoved(i);
    }
    mPoints.clear();

    mLastSourceIdx = 0;
    mLastSourceIdx = calculateAvg(mLastSourceIdx);
}

int DataSeriesCurveMapping::calculateAvg(int fromSourceIdx)
{
    if(!mSource || !mRecvStepSeries)
        return fromSourceIdx;

    int sourceIdx = fromSourceIdx;
    int recvIdx = 0;

    int step = 0;

    for(; sourceIdx < mSource->getPointCount(); sourceIdx++)
    {
        QPointF pt = mSource->getPointAt(sourceIdx);

        for(; recvIdx < mRecvStepSeries->getPointCount(); recvIdx++)
        {
            QPointF stepPt = mRecvStepSeries->getPointAt(recvIdx);

            if(stepPt.x() > pt.x())
            {
                recvIdx--;
                break;
            }

            step = int(stepPt.y());
        }

        QPointF result(step, pt.y());
        mPoints.append(result);
        emit pointAdded(mPoints.size() - 1, result);
    }

    return sourceIdx;
}

IDataSeries *DataSeriesCurveMapping::recvStep() const
{
    return mRecvStepSeries;
}

void DataSeriesCurveMapping::setRecvStep(IDataSeries *newRecvStep)
{
    if(mRecvStepSeries)
    {
        disconnect(mRecvStepSeries, &IDataSeries::pointChanged, this, &DataSeriesCurveMapping::recalculate);
        disconnect(mRecvStepSeries, &IDataSeries::pointRemoved, this, &DataSeriesCurveMapping::recalculate);
        disconnect(mRecvStepSeries, &QObject::destroyed, this, &DataSeriesCurveMapping::onSourceDestroyed);
    }

    mRecvStepSeries = newRecvStep;

    if(mRecvStepSeries)
    {
        connect(mRecvStepSeries, &IDataSeries::pointChanged, this, &DataSeriesCurveMapping::recalculate);
        connect(mRecvStepSeries, &IDataSeries::pointRemoved, this, &DataSeriesCurveMapping::recalculate);
        connect(mRecvStepSeries, &QObject::destroyed, this, &DataSeriesCurveMapping::onSourceDestroyed);
    }

    recalculate();
}

IDataSeries *DataSeriesCurveMapping::source() const
{
    return mSource;
}

void DataSeriesCurveMapping::setSource(IDataSeries *newSource)
{
    if(mSource)
    {
        disconnect(mSource, &IDataSeries::pointAdded, this, &DataSeriesCurveMapping::onSourcePointAdded);
        disconnect(mSource, &IDataSeries::pointChanged, this, &DataSeriesCurveMapping::recalculate);
        disconnect(mSource, &IDataSeries::pointRemoved, this, &DataSeriesCurveMapping::recalculate);
        disconnect(mSource, &QObject::destroyed, this, &DataSeriesCurveMapping::onSourceDestroyed);
    }

    mSource = newSource;

    if(mSource)
    {
        setName(mSource->name());

        connect(mSource, &IDataSeries::pointAdded, this, &DataSeriesCurveMapping::onSourcePointAdded);
        connect(mSource, &IDataSeries::pointChanged, this, &DataSeriesCurveMapping::recalculate);
        connect(mSource, &IDataSeries::pointRemoved, this, &DataSeriesCurveMapping::recalculate);
        connect(mSource, &QObject::destroyed, this, &DataSeriesCurveMapping::onSourceDestroyed);
    }

    recalculate();
}

DataSeriesType DataSeriesCurveMapping::getType() const
{
    return DataSeriesType::CurveMapping;
}

int DataSeriesCurveMapping::getPointCount() const
{
    return mPoints.size();
}

QPointF DataSeriesCurveMapping::getPointAt(int index) const
{
    return mPoints.value(index, QPointF());
}

QString DataSeriesCurveMapping::getPointTooltip(int index) const
{
    if(!mSource || index < 0 || index >= mPoints.size())
        return QString();


    QString fullName = name() + QLatin1String(" (%1)").arg(mSource->name());
    return IDataSeries::defaultTooltip(fullName, index, mPoints.at(index));
}

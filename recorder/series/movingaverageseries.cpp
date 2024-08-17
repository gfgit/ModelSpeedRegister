#include "movingaverageseries.h"

MovingAverageSeries::MovingAverageSeries(int windowSize, QObject *parent)
    : IDataSeries{parent}
    , mSource(nullptr)
    , mWindowSize(windowSize)
{
    if((mWindowSize % 2) == 0)
        mWindowSize++; // We support odd window sizes only for now

    setName(tr("MovingAvg %1").arg(mWindowSize));
}

void MovingAverageSeries::onPointAdded(int indexAdded, const QPointF &point)
{
    updateAvg(indexAdded, point, DataSeriesAction::PointAdded);
}

void MovingAverageSeries::onPointChanged(int indexChanged, const QPointF &point)
{
    updateAvg(indexChanged, point, DataSeriesAction::PointChanged);
}

void MovingAverageSeries::onPointRemoved(int indexRemoved)
{
    updateAvg(indexRemoved, QPointF(), DataSeriesAction::PointRemoved);
}

void MovingAverageSeries::onSourceDestroyed()
{
    mSource = nullptr;

    int oldSize = mPoints.size();
    for(int i = oldSize - 1; i >= 0; i--)
    {
        emit pointRemoved(i);
    }
    mPoints.clear();
}

void MovingAverageSeries::updateAvg(int indexChanged, const QPointF &point, DataSeriesAction action)
{
    if(indexChanged > mPoints.size())
        indexChanged = mPoints.size();

    if(action == DataSeriesAction::PointRemoved)
    {
        // Remove point, then update average
        mPoints.removeAt(indexChanged);
        emit pointRemoved(indexChanged);
    }
    else if(action == DataSeriesAction::PointAdded)
    {
        // Add point but notify later when we have calculated y value
        mPoints.insert(indexChanged, QPointF(point.x(), 0));
    }

    const int halfWindow = (mWindowSize - 1) / 2;
    const int firstItem = qMax(0, indexChanged - halfWindow);
    const int lastItem = qMin(mPoints.size() - 1, indexChanged + halfWindow);

    for(int indexToUpdate = firstItem; indexToUpdate <= lastItem; indexToUpdate++)
    {
        const int start = indexToUpdate - halfWindow;
        const int end = indexToUpdate + halfWindow;

        if(start < 0 || end >= mPoints.size())
        {
            // We are too near the end. Cannot calculate average, set y = 0
            if(mPoints[indexToUpdate].y() != 0 || indexToUpdate == indexChanged)
            {
                mPoints[indexToUpdate].ry() = 0;

                if(indexToUpdate == indexChanged && action == DataSeriesAction::PointAdded)
                    emit pointAdded(indexToUpdate, mPoints[indexToUpdate]);
                else
                    emit pointChanged(indexToUpdate, mPoints[indexToUpdate]);
            }

            continue;
        }

        double sum = 0;

        for(int sourceIndex = start; sourceIndex <= end; sourceIndex++)
        {
            sum += mSource->getPointAt(sourceIndex).y();
        }

        const double avg = sum / mWindowSize;

        if(mPoints[indexToUpdate].y() != avg || indexToUpdate == indexChanged)
        {
            mPoints[indexToUpdate].ry() = avg;

            if(indexToUpdate == indexChanged && action == DataSeriesAction::PointAdded)
                emit pointAdded(indexToUpdate, mPoints[indexToUpdate]);
            else
                emit pointChanged(indexToUpdate, mPoints[indexToUpdate]);
        }
    }
}

IDataSeries *MovingAverageSeries::source() const
{
    return mSource;
}

void MovingAverageSeries::setSource(IDataSeries *newSource)
{
    if(mSource)
    {
        disconnect(mSource, &IDataSeries::pointAdded, this, &MovingAverageSeries::onPointAdded);
        disconnect(mSource, &IDataSeries::pointChanged, this, &MovingAverageSeries::onPointChanged);
        disconnect(mSource, &IDataSeries::pointRemoved, this, &MovingAverageSeries::onPointRemoved);
        disconnect(mSource, &QObject::destroyed, this, &MovingAverageSeries::onSourceDestroyed);

        // Clear previous average
        int oldSize = mPoints.size();
        for(int i = oldSize - 1; i >= 0; i--)
        {
            emit pointRemoved(i);
        }
        mPoints.clear();
    }

    mSource = newSource;

    if(mSource)
    {
        connect(mSource, &IDataSeries::pointAdded, this, &MovingAverageSeries::onPointAdded);
        connect(mSource, &IDataSeries::pointChanged, this, &MovingAverageSeries::onPointChanged);
        connect(mSource, &IDataSeries::pointRemoved, this, &MovingAverageSeries::onPointRemoved);
        connect(mSource, &QObject::destroyed, this, &MovingAverageSeries::onSourceDestroyed);

        // Build new average
        for(int i = 0; i < mSource->getPointCount(); i++)
        {
            onPointAdded(i, mSource->getPointAt(i));
        }
    }
}

DataSeriesType MovingAverageSeries::getType() const
{
    return DataSeriesType::MovingAverage;
}

int MovingAverageSeries::getPointCount() const
{
    return mPoints.size();
}

QPointF MovingAverageSeries::getPointAt(int index) const
{
    return mPoints.value(index, QPointF());
}

QString MovingAverageSeries::getPointTooltip(int index) const
{
    if(!mSource || index < 0 || index >= mPoints.size())
        return QString();


    QString fullName = name() + QLatin1String(" (%1)").arg(mSource->name());
    return IDataSeries::defaultTooltip(fullName, index, mPoints.at(index));
}

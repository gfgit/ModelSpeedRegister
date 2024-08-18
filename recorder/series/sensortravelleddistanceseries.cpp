#include "sensortravelleddistanceseries.h"

SensorTravelledDistanceSeries::SensorTravelledDistanceSeries(QObject *parent)
    : IDataSeries{parent}
{
    setName(tr("Travelled Distance"));
}

DataSeriesType SensorTravelledDistanceSeries::getType() const
{
    return DataSeriesType::TravelledDistance;
}

int SensorTravelledDistanceSeries::getPointCount() const
{
    return mPoints.count();
}

QPointF SensorTravelledDistanceSeries::getPointAt(int index) const
{
    if(index < 0 || index >= mPoints.count())
        return QPointF();
    return mPoints.at(index);
}

QString SensorTravelledDistanceSeries::getPointTooltip(int index) const
{
    if(index < 0 || index >= mPoints.count())
        return QString();

    const QPointF point = mPoints.at(index);

    return tr("Travelled: %1\n"
              "Time: %2 s").arg(point.y()).arg(point.x());
}

void SensorTravelledDistanceSeries::addPoint(double speed, double seconds)
{
    QPointF point(seconds, speed);
    mPoints.append(point);
    emit pointAdded(mPoints.size() - 1, point);
}

void SensorTravelledDistanceSeries::clear()
{
    int oldSize = mPoints.size();
    mPoints.clear();
    for(int i = oldSize - 1; i >= 0; i--)
    {
        emit pointRemoved(i);
    }
}


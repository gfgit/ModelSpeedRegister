#include "rawsensordataseries.h"

RawSensorDataSeries::RawSensorDataSeries(QObject *parent)
    : IDataSeries{parent}
{
    setName(tr("Requested DCC Step"));
}

DataSeriesType RawSensorDataSeries::getType() const
{
    return DataSeriesType::SensorRawData;
}

int RawSensorDataSeries::getPointCount() const
{
    return mPoints.count();
}

QPointF RawSensorDataSeries::getPointAt(int index) const
{
    if(index < 0 || index >= mPoints.count())
        return QPointF();
    return mPoints.at(index);
}

QString RawSensorDataSeries::getPointTooltip(int index) const
{
    if(index < 0 || index >= mPoints.count())
        return QString();

    const QPointF point = mPoints.at(index);

    return tr("Req Step: %1\n"
              "Time: %2 s").arg(point.y()).arg(point.x());
}

void RawSensorDataSeries::addPoint(double speed, double seconds)
{
    QPointF point(seconds, speed);
    mPoints.append(point);
    emit pointAdded(mPoints.size() - 1, point);
}

void RawSensorDataSeries::clear()
{
    int oldSize = mPoints.size();
    mPoints.clear();
    for(int i = oldSize - 1; i >= 0; i--)
    {
        emit pointRemoved(i);
    }
}


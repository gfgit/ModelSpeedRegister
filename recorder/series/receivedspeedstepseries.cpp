#include "receivedspeedstepseries.h"

ReceivedSpeedStepSeries::ReceivedSpeedStepSeries(QObject *parent)
    : IDataSeries{parent}
{
    setName(tr("Requested DCC Step"));
}

DataSeriesType ReceivedSpeedStepSeries::getType() const
{
    return DataSeriesType::RequestedSpeedStep;
}

int ReceivedSpeedStepSeries::getPointCount() const
{
    return mPoints.count();
}

QPointF ReceivedSpeedStepSeries::getPointAt(int index) const
{
    if(index < 0 || index >= mPoints.count())
        return QPointF();
    return mPoints.at(index);
}

QString ReceivedSpeedStepSeries::getPointTooltip(int index) const
{
    if(index < 0 || index >= mPoints.count())
        return QString();

    const QPointF point = mPoints.at(index);

    return tr("Req Step: %1\n"
              "Time: %2 s").arg(point.y()).arg(point.x());
}

void ReceivedSpeedStepSeries::addPoint(int reqStep, double seconds)
{
    QPointF point(seconds, reqStep);
    mPoints.append(point);
    emit pointAdded(mPoints.size() - 1, point);
}

void ReceivedSpeedStepSeries::clear()
{
    int oldSize = mPoints.size();
    mPoints.clear();
    for(int i = 0; i < oldSize; i++)
    {
        emit pointRemoved(i);
    }
}

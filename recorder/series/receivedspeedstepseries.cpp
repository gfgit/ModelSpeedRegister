#include "receivedspeedstepseries.h"

ReceivedSpeedStepSeries::ReceivedSpeedStepSeries(QObject *parent)
    : IDataSeries{parent}
{
    setName(tr("Received DCC Step"));
}

DataSeriesType ReceivedSpeedStepSeries::getType() const
{
    return DataSeriesType::ReceivedSpeedStep;
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

    return tr("Recv Step: %1\n"
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
    for(int i = oldSize - 1; i >= 0; i--)
    {
        emit pointRemoved(i);
    }
}

#include "idataseries.h"

IDataSeries::IDataSeries(QObject *parent)
    : QObject{parent}
{

}

QString IDataSeries::name() const
{
    return mName;
}

void IDataSeries::setName(const QString &newName)
{
    mName = newName;
}

QString IDataSeries::defaultTooltip(const QString& seriesName, int index, const QPointF &point)
{
    return tr("<b>%1</b><br>"
              "Index: <b>%2</b><br>"
              "X: <b>%3</b><br>"
              "Y: <b>%4</b>")
            .arg(seriesName)
            .arg(index)
            .arg(point.x()).arg(point.y());
}

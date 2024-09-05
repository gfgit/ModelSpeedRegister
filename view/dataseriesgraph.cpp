#include "dataseriesgraph.h"

#include "../recorder/idataseries.h"

DataSeriesGraph::DataSeriesGraph(IDataSeries *s, QObject *parent)
    : QLineSeries(parent)
    , mDataSeries(s)
{
    connect(mDataSeries, &IDataSeries::pointAdded, this, &DataSeriesGraph::pointAdded);
    connect(mDataSeries, &IDataSeries::pointRemoved, this, &DataSeriesGraph::pointRemoved);
    connect(mDataSeries, &IDataSeries::pointChanged, this, &DataSeriesGraph::pointChanged);

    setName(mDataSeries->name());

    for(int i = 0; i < s->getPointCount(); i++)
    {
        insert(i, s->getPointAt(i));
    }
}

IDataSeries *DataSeriesGraph::dataSeries() const
{
    return mDataSeries;
}

void DataSeriesGraph::pointAdded(int index, const QPointF &point)
{
    insert(index, point);
}

void DataSeriesGraph::pointRemoved(int index)
{
    removePoints(index, 1);
}

void DataSeriesGraph::pointChanged(int index, const QPointF &newPoint)
{
    replace(index, newPoint);
}

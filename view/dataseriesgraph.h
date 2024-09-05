#ifndef DATASERIESGRAPH_H
#define DATASERIESGRAPH_H

#include <QLineSeries>

class IDataSeries;

class DataSeriesGraph : public QLineSeries
{
    Q_OBJECT
public:
    DataSeriesGraph(IDataSeries *s, QObject *parent = nullptr);

    IDataSeries *dataSeries() const;

private slots:
    void pointAdded(int index, const QPointF& point);
    void pointRemoved(int index);
    void pointChanged(int index, const QPointF& newPoint);

private:
    IDataSeries *mDataSeries;
};


#endif // DATASERIESGRAPH_H

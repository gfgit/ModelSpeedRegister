#ifndef DATASERIESFILTERMODEL_H
#define DATASERIESFILTERMODEL_H

#include <QAbstractTableModel>
#include <QVector>

#include <QLineSeries>

class IDataSeries;
class RecordingManager;

class Chart;
class QValueAxis;

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


class DataSeriesFilterModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Columns
    {
        Name = 0,
        Color,
        Type,
        NCols
    };

    explicit DataSeriesFilterModel(Chart *chart, QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &idx, const QVariant &value, int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QColor getColorAt(const QModelIndex& idx) const;
    void setColorAt(const QModelIndex &idx, const QColor &color);

    RecordingManager *recMgr() const;
    void setRecMgr(RecordingManager *newRecMgr);

    void setAxisRange(const QRectF &r);
    QRectF getAxisRange() const;

private slots:
    void onSeriesRegistered(IDataSeries *s);
    void onSeriesUnregistered(IDataSeries *s);

private:
    RecordingManager *mRecMgr;
    Chart *mChart;

    QValueAxis *mSpeedAxis;
    QValueAxis *mStepAxis;
    QValueAxis *mTimeAxis;

    QVector<DataSeriesGraph *> mItems;
};

#endif // DATASERIESFILTERMODEL_H

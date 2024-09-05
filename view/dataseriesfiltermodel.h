#ifndef DATASERIESFILTERMODEL_H
#define DATASERIESFILTERMODEL_H

#include <QAbstractTableModel>
#include <QVector>

class IDataSeries;
class RecordingManager;

class Chart;
class QValueAxis;
class DataSeriesGraph;

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
    ~DataSeriesFilterModel();

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

    bool axisRangeFollowsChanges() const;
    void setAxisRangeFollowsChanges(bool newAxisRangeFollowsChanges);

private slots:
    void onSeriesRegistered(IDataSeries *s);
    void onSeriesUnregistered(IDataSeries *s);

private:
    RecordingManager *mRecMgr;
    Chart *mChart;

    QValueAxis *mSpeedAxis;
    QValueAxis *mStepAxis;
    QValueAxis *mTimeAxis;
    QValueAxis *mTravelledAxis;

    QVector<DataSeriesGraph *> mItems;

    bool mAxisRangeFollowsChanges = true;
};

#endif // DATASERIESFILTERMODEL_H

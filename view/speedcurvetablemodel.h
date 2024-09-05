#ifndef SPEEDCURVETABLEMODEL_H
#define SPEEDCURVETABLEMODEL_H

#include <QAbstractTableModel>
#include <QVector>

class DataSeriesGraph;
class DataSeriesCurveMapping;
class IDataSeries;

class Chart;
class QValueAxis;

class RecordingManager;

class SpeedCurveTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit SpeedCurveTableModel(Chart *chart, QObject *parent = nullptr);
    ~SpeedCurveTableModel();

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex &idx, const QVariant &value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& idx) const override;

    RecordingManager *recMgr() const;
    void setRecMgr(RecordingManager *newRecMgr);

    enum SpecialRows
    {
        VisibilityCheckBox = 0
    };

    QColor getSeriesColor(int column) const;
    void setSeriesColor(int column, const QColor& color);

    bool axisRangeFollowsChanges() const;
    void setAxisRangeFollowsChanges(bool newAxisRangeFollowsChanges);

    void setAxisRange(const QRectF &r);
    QRectF getAxisRange() const;

private slots:
    void onSeriesRegistered(IDataSeries *s);
    void onSeriesUnregistered(IDataSeries *s);
    void onSeriesChanged(int, const QPointF &pt);
    void onSeriesPointRemoved();

protected:
    void timerEvent(QTimerEvent *e) override;

private:
    enum State
    {
        Normal = 0,
        BeginWaitInternal,
        WaitingForRecalculation
    };

    void beginSetState(State newState);
    void endSetState();
    void recalculate();

    inline int getStepForRow(int row) const
    {
        if(mState == State::WaitingForRecalculation || row < 0 || row > mLastRow)
            return -1;

        int step = 0;
        for(int i = 0; i <= 126; i++)
        {
            if(mStepStart[i] > row)
                break;

            step = i;
        }

        return step;
    }

    int getFirstIndexForStep(DataSeriesGraph *s, int step) const;

private:
    RecordingManager *mRecMgr;
    Chart *mChart;

    QValueAxis *mStepAxis;
    QValueAxis *mSpeedAxis;

    struct DataSeriesColumn
    {
        DataSeriesGraph *mGraph;
        DataSeriesCurveMapping *mSeries;
    };
    QVector<DataSeriesColumn> mSeries;

    State mState = Normal;
    int mRecalculationTimerId = 0;

    int mStepStart[126 + 1] = {0};
    int mLastRow = 0;

    bool mAxisRangeFollowsChanges = true;
};

#endif // SPEEDCURVETABLEMODEL_H

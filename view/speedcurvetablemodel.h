#ifndef SPEEDCURVETABLEMODEL_H
#define SPEEDCURVETABLEMODEL_H

#include <QAbstractTableModel>
#include <QVector>

class DataSeriesGraph;
class DataSeriesCurveMapping;
class IDataSeries;

class Chart;
class QValueAxis;
class QLineSeries;

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

    enum ColumnType
    {
        Invalid = 0,
        TestSeries = 1,
        StoredSpeedCurve = 2
    };

    QColor getSeriesColor(int column) const;
    void setSeriesColor(int column, const QColor& color);

    QString getSeriesName(int column) const;
    void setSeriesName(int column, const QString& newName);

    ColumnType getColumnType(int column) const;

    bool axisRangeFollowsChanges() const;
    void setAxisRangeFollowsChanges(bool newAxisRangeFollowsChanges);

    void setAxisRange(const QRectF &r);
    QRectF getAxisRange() const;

    int currentEditCurve() const;
    void setCurrentEditCurve(int curveToEdit);

    QLineSeries *addNewCurve(const QString& name);
    void removeCurveAt(int column);
    QLineSeries *getCurveAt(int column) const;
    void updateCurveAt(int column);

    QPointF getValueAtIdx(const QModelIndex& idx) const;
    void storeValueInCurrentCurve(const QModelIndex& idx, const QPointF &val);
    void storeFirstOfEachStepInCurrentCurve(int sourceCol);

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

    int getFirstIndexForStep(QLineSeries *s, int step) const;

    QLineSeries *getSeriesAtColumn(int col) const;

private:
    RecordingManager *mRecMgr;
    Chart *mChart;

    QValueAxis *mStepAxis;
    QValueAxis *mSpeedAxis;

    // Series coming from Test, gets cleared on start
    struct DataSeriesColumn
    {
        DataSeriesGraph *mGraph;
        DataSeriesCurveMapping *mSeries;
    };
    QVector<DataSeriesColumn> mSeries;

    // Stored curves, these are persistent
    QVector<QLineSeries *> mCurves;

    State mState = Normal;
    int mRecalculationTimerId = 0;

    int mStepStart[126 + 1] = {0};
    int mLastRow = 0;

    int mCurrentEditCurve = -1;

    bool mAxisRangeFollowsChanges = true;
};

#endif // SPEEDCURVETABLEMODEL_H

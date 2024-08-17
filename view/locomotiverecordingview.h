#ifndef LOCOMOTIVERECORDINGVIEW_H
#define LOCOMOTIVERECORDINGVIEW_H

#include <QWidget>

class Chart;
class ChartView;

class DataSeriesFilterModel;
class RecordingManager;

class QTableView;

class QSplitter;

class LocomotiveRecordingView : public QWidget
{
    Q_OBJECT
public:
    explicit LocomotiveRecordingView(QWidget *parent = nullptr);

    RecordingManager *recMgr() const;
    void setRecMgr(RecordingManager *newRecMgr);

private:
    RecordingManager *mRecMgr = nullptr;

    Chart *mChart;
    ChartView *mChartView;

    QTableView *mFilterView;
    DataSeriesFilterModel *mFilterModel;

    QSplitter *mSplitter;
};

#endif // LOCOMOTIVERECORDINGVIEW_H

#ifndef LOCOSPEEDCURVEVIEW_H
#define LOCOSPEEDCURVEVIEW_H

#include <QWidget>
#include <QLineSeries>
#include <QScatterSeries>

class Chart;
class ChartView;

class QTableView;
class SpeedCurveTableModel;

class QSplitter;

class RecordingManager;

class LocoSpeedCurveView : public QWidget
{
    Q_OBJECT
public:
    explicit LocoSpeedCurveView(QWidget *parent = nullptr);

    void setTargedSpeedCurve(const QVector<double>& targetSpeedCurve);

    RecordingManager *recMgr() const;
    void setRecMgr(RecordingManager *newRecMgr);

private:
    Chart *mChart;
    ChartView *mChartView;

    QLineSeries mTargetSpeedCurve;
    RecordingManager *mRecMgr = nullptr;

    QTableView *mFilterView;
    SpeedCurveTableModel *mFilterModel;

    QSplitter *mSplitter;
};

#endif // LOCOSPEEDCURVEVIEW_H

#ifndef LOCOMOTIVERECORDINGVIEW_H
#define LOCOMOTIVERECORDINGVIEW_H

#include <QWidget>
#include <QLineSeries>

class Chart;
class ChartView;
class QValueAxis;

class LocomotiveRecording;

class LocomotiveRecordingView : public QWidget
{
    Q_OBJECT
public:
    explicit LocomotiveRecordingView(QWidget *parent = nullptr);

    LocomotiveRecording *recording() const;
    void setRecording(LocomotiveRecording *newRecording);

private slots:
    void onItemChanged(int index);

private:
    LocomotiveRecording *mRecording = nullptr;

    Chart *mChart;
    ChartView *mChartView;

    QLineSeries mSpeedSeries;
    QLineSeries mSpeedAVGSeries;
    QLineSeries mReqStepSeries;
    QLineSeries mActualStepSeries;

    QValueAxis *mAxisX;
    QValueAxis *mSpeedAxisY;
    QValueAxis *mStepAxisY;
};

#endif // LOCOMOTIVERECORDINGVIEW_H

#ifndef LOCOMOTIVERECORDINGVIEW_H
#define LOCOMOTIVERECORDINGVIEW_H

#include <QWidget>
#include <QLineSeries>

class QChartView;

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

    QChartView *mChartView;

    QLineSeries mSpeedSeries;
    QLineSeries mSpeedAVGSeries;
    QLineSeries mReqStepSeries;
    QLineSeries mActualStepSeries;
};

#endif // LOCOMOTIVERECORDINGVIEW_H

#ifndef LOCOMOTIVERECORDINGVIEW_H
#define LOCOMOTIVERECORDINGVIEW_H

#include <QWidget>

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
};

#endif // LOCOMOTIVERECORDINGVIEW_H

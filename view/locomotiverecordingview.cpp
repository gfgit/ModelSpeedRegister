#include "locomotiverecordingview.h"

#include "../recorder/locomotiverecording.h"

#include <QChartView>
#include <QVBoxLayout>

LocomotiveRecordingView::LocomotiveRecordingView(QWidget *parent)
    : QWidget{parent}
{
    QVBoxLayout *lay = new QVBoxLayout(this);
    mChartView = new QChartView;
    lay->addWidget(mChartView);
}

LocomotiveRecording *LocomotiveRecordingView::recording() const
{
    return mRecording;
}

void LocomotiveRecordingView::setRecording(LocomotiveRecording *newRecording)
{
    if(mRecording)
    {
        disconnect(mRecording, &LocomotiveRecording::itemChanged, this, &LocomotiveRecordingView::onItemChanged):
    }

    mRecording = newRecording;

    if(mRecording)
    {
        connect(mRecording, &LocomotiveRecording::itemChanged, this, &LocomotiveRecordingView::onItemChanged):
    }
}

void LocomotiveRecordingView::onItemChanged(int index)
{

}

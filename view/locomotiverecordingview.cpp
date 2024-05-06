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

    mSpeedSeries.setColor(Qt::red);
    mChartView->chart()->addSeries(&mSpeedSeries);

    mSpeedAVGSeries.setColor(Qt::darkMagenta);
    mChartView->chart()->addSeries(&mSpeedAVGSeries);

    mReqStepSeries.setColor(Qt::cyan);
    mChartView->chart()->addSeries(&mReqStepSeries);

    mActualStepSeries.setColor(Qt::green);
    mChartView->chart()->addSeries(&mActualStepSeries);
}

LocomotiveRecording *LocomotiveRecordingView::recording() const
{
    return mRecording;
}

void LocomotiveRecordingView::setRecording(LocomotiveRecording *newRecording)
{
    if(mRecording)
    {
        disconnect(mRecording, &LocomotiveRecording::itemChanged, this, &LocomotiveRecordingView::onItemChanged);
    }

    mRecording = newRecording;

    if(mRecording)
    {
        connect(mRecording, &LocomotiveRecording::itemChanged, this, &LocomotiveRecordingView::onItemChanged);
    }
}

void LocomotiveRecordingView::onItemChanged(int index)
{
    if(!mRecording)
        return;

    const RecordingItem item = mRecording->getItemAt(index);
    if(mSpeedSeries.count() <= index)
    {
        qint64 lastTimestamp = 0;
        if(mSpeedSeries.count())
            lastTimestamp = mSpeedSeries.at(mSpeedSeries.count() - 1).x();

        for(int i = mSpeedSeries.count(); i <= index; i++)
        {
            // Fill in new points
            QPoint p(lastTimestamp, 0);
            mSpeedSeries.append(p);
            mSpeedAVGSeries.append(p);
            mReqStepSeries.append(p);
            mActualStepSeries.append(p);
        }
    }

    mSpeedSeries.replace(index, item.timestampMilliSec, item.metersPerSecond);
    mSpeedSeries.replace(index, item.timestampMilliSec, item.metersPerSecondAvg);
    mSpeedSeries.replace(index, item.timestampMilliSec, item.requestedSpeedStep);
    mSpeedSeries.replace(index, item.timestampMilliSec, item.actualSpeedStep);
}

#include "locomotiverecordingview.h"

#include "../recorder/locomotiverecording.h"

#include "../chart/chartview.h"
#include "../chart/chart.h"
#include <QValueAxis>

#include <QVBoxLayout>

#include <QDebug>

LocomotiveRecordingView::LocomotiveRecordingView(QWidget *parent)
    : QWidget{parent}
{
    mChart = new Chart;
    mChart->setLocalizeNumbers(false);

    mAxisX = new QValueAxis(this);
    mAxisX->setRange(0, 100000);
    mAxisX->setLabelFormat("%d");
    mAxisX->setTitleText("Time (msec)");
    mAxisX->setTickType(QValueAxis::TicksDynamic);
    mAxisX->setTickInterval(1000);

    mAxisY = new QValueAxis(this);
    mAxisY->setRange(0, 50);
    mAxisX->setLabelFormat("%f");
    mAxisY->setTitleText("Speed");

    mChart->addAxis(mAxisX, Qt::AlignBottom);
    mChart->addAxis(mAxisY, Qt::AlignLeft);

    QVBoxLayout *lay = new QVBoxLayout(this);
    mChartView = new ChartView(mChart);
    lay->addWidget(mChartView);

    mChart->addSeries(&mSpeedSeries);
    mSpeedSeries.setColor(Qt::red);
    mSpeedSeries.attachAxis(mAxisX);
    mSpeedSeries.attachAxis(mAxisY);

    mChart->addSeries(&mSpeedAVGSeries);
    mSpeedAVGSeries.setColor(Qt::blue);
    mSpeedAVGSeries.attachAxis(mAxisX);
    mSpeedAVGSeries.attachAxis(mAxisY);

    mChart->addSeries(&mReqStepSeries);
    mReqStepSeries.setColor(Qt::cyan);
    mReqStepSeries.attachAxis(mAxisX);
    mReqStepSeries.attachAxis(mAxisY);

    mChart->addSeries(&mActualStepSeries);
    mActualStepSeries.setColor(Qt::black);
    mActualStepSeries.attachAxis(mAxisX);
    mActualStepSeries.attachAxis(mAxisY);

    connect(mChartView, &ChartView::scrollResetRequested, this,
            [this]()
            {
                // Reset origin to {0,0}
                mAxisX->setRange(0, mAxisX->max() - mAxisX->min());
                mAxisY->setRange(0, mAxisY->max() - mAxisY->min());
            });
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

    // Fill initial values
    for(int i = 0; i < mRecording->getItemCount(); i++)
    {
        onItemChanged(i);
    }
}

void LocomotiveRecordingView::onItemChanged(int index)
{
    if(!mRecording)
        return;

    if(index < 0)
    {
        mSpeedSeries.clear();
        mSpeedAVGSeries.clear();
        mReqStepSeries.clear();
        mActualStepSeries.clear();
        return;
    }

    const RecordingItem item = mRecording->getItemAt(index);

    if(mSpeedSeries.count() <= index)
    {

        qDebug() << "UPDATE time:" << item.timestampMilliSec;

        qint64 lastTimestamp = 0;
        if(mSpeedSeries.count())
            lastTimestamp = mSpeedSeries.at(mSpeedSeries.count() - 1).x();

        if(mAxisX->max() < lastTimestamp)
            mAxisX->setMax(lastTimestamp + 1000);

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

    if(item.metersPerSecond > mAxisY->max())
        mAxisY->setMax(item.metersPerSecond + 10);

    mSpeedSeries.replace(index, item.timestampMilliSec, item.metersPerSecond);
    mSpeedAVGSeries.replace(index, item.timestampMilliSec, item.metersPerSecondAvg);
    mReqStepSeries.replace(index, item.timestampMilliSec, item.requestedSpeedStep);
    mActualStepSeries.replace(index, item.timestampMilliSec, item.actualSpeedStep);
}

#include "locomotiverecordingview.h"
#include "dataseriesfiltermodel.h"

#include "../recorder/recordingmanager.h"

#include "../chart/chartview.h"
#include "../chart/chart.h"

#include <QVBoxLayout>
#include <QSplitter>
#include <QTableView>

#include <QDebug>

LocomotiveRecordingView::LocomotiveRecordingView(QWidget *parent)
    : QWidget{parent}
{
    mChart = new Chart;
    mChart->setLocalizeNumbers(false);

    QVBoxLayout *lay = new QVBoxLayout(this);

    mSplitter = new QSplitter;
    lay->addWidget(mSplitter);

    mChartView = new ChartView(mChart);
    mSplitter->addWidget(mChartView);

    mFilterView = new QTableView;
    mSplitter->addWidget(mFilterView);

    mFilterModel = new DataSeriesFilterModel(mChart, this);
    mFilterView->setModel(mFilterModel);

    connect(mChartView, &ChartView::scrollResetRequested, this,
            [this]()
            {
                // Reset origin to {0,0}
                QRectF r = mFilterModel->getAxisRange();
                r.moveTopLeft(QPointF());
                mFilterModel->setAxisRange(r);
            });
}

RecordingManager *LocomotiveRecordingView::recMgr() const
{
    return mRecMgr;
}

void LocomotiveRecordingView::setRecMgr(RecordingManager *newRecMgr)
{
    mRecMgr = newRecMgr;
    mFilterModel->setRecMgr(mRecMgr);
}

#include "locomotiverecordingview.h"
#include "dataseriesfiltermodel.h"

#include "../recorder/recordingmanager.h"

#include "../chart/chartview.h"
#include "../chart/chart.h"

#include <QVBoxLayout>
#include <QSplitter>
#include <QTableView>
#include <QCheckBox>

#include <QColorDialog>

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

    mFilterModel = new DataSeriesFilterModel(mChart, this);

    QWidget *lay2Widget = new QWidget;
    QVBoxLayout *lay2 = new QVBoxLayout(lay2Widget);
    mSplitter->addWidget(lay2Widget);

    QCheckBox *checkBox = new QCheckBox(tr("Axis Follow Changes"));
    lay2->addWidget(checkBox);

    connect(checkBox, &QCheckBox::toggled, this,
            [this](bool val)
            {
                mFilterModel->setAxisRangeFollowsChanges(val);
            });
    checkBox->setCheckState(Qt::Checked);

    mFilterView = new QTableView;
    lay2->addWidget(mFilterView);
    mFilterView->setModel(mFilterModel);

    connect(mFilterView, &QTableView::activated, this,
            [this](const QModelIndex& idx)
    {
        if(idx.column() != DataSeriesFilterModel::Color)
            return;

        QColor color = mFilterModel->getColorAt(idx);
        color = QColorDialog::getColor(color, this, tr("Choose Series Color"));
        mFilterModel->setColorAt(idx, color);
    });

    connect(mChartView, &ChartView::scrollResetRequested, this,
            [this]()
            {
                // Reset origin to {0,0}
                QRectF r = mFilterModel->getAxisRange();
                r.moveBottomLeft(QPointF());
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

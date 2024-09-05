#include "locospeedcurveview.h"

#include "../recorder/recordingmanager.h"

#include "../chart/chartview.h"
#include "../chart/chart.h"

#include "speedcurvetablemodel.h"

#include <QVBoxLayout>
#include <QTableView>
#include <QSplitter>
#include <QCheckBox>

#include <QColorDialog>

LocoSpeedCurveView::LocoSpeedCurveView(QWidget *parent)
    : QWidget{parent}
    , mRecMgr(nullptr)
{
    mChart = new Chart;
    mChart->setLocalizeNumbers(false);

    QVBoxLayout *lay = new QVBoxLayout(this);

    mSplitter = new QSplitter;
    lay->addWidget(mSplitter);

    mChartView = new ChartView(mChart);
    mSplitter->addWidget(mChartView);

    mFilterModel = new SpeedCurveTableModel(mChart, this);

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
        if(idx.row() != SpeedCurveTableModel::SpecialRows::VisibilityCheckBox)
            return;

        QColor color = mFilterModel->getSeriesColor(idx.column());
        color = QColorDialog::getColor(color, this, tr("Choose Series Color"));
        mFilterModel->setSeriesColor(idx.column(), color);
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

void LocoSpeedCurveView::setTargedSpeedCurve(const QVector<double> &targetSpeedCurve)
{
//    mTargetSpeedCurve.clear();
//    for(int i = 0; i < targetSpeedCurve.size(); i++)
//    {
//        mTargetSpeedCurve.append(i, targetSpeedCurve[i]);
//    }

//    if(!targetSpeedCurve.empty())
//    {
//        mAxisY->setMax(targetSpeedCurve.last() * 1.2);
    //    }
}

RecordingManager *LocoSpeedCurveView::recMgr() const
{
    return mRecMgr;
}

void LocoSpeedCurveView::setRecMgr(RecordingManager *newRecMgr)
{
    mRecMgr = newRecMgr;
    mFilterModel->setRecMgr(mRecMgr);
}

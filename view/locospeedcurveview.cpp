#include "locospeedcurveview.h"

#include "../recorder/recordingmanager.h"

#include "../chart/chartview.h"
#include "../chart/chart.h"

#include "speedcurvetablemodel.h"

#include <QVBoxLayout>
#include <QTableView>
#include <QSplitter>
#include <QCheckBox>

#include <QPointer>
#include <QMenu>
#include <QInputDialog>
#include <QFileDialog>
#include <QColorDialog>

#include "../recorder/rawspeedcurveio.h"

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
    mFilterView->setModel(mFilterModel);
    mFilterView->setContextMenuPolicy(Qt::CustomContextMenu);
    lay2->addWidget(mFilterView);

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

    connect(mFilterView, &QTableView::customContextMenuRequested,
            this, &LocoSpeedCurveView::onTableContextMenu);
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

void LocoSpeedCurveView::onTableContextMenu(const QPoint &pos)
{
    QModelIndex idx = mFilterView->indexAt(pos);

    if(!idx.isValid())
        return;

    auto colType = mFilterModel->getColumnType(idx.column());
    int currEdit = mFilterModel->currentEditCurve();

    QPointer<QMenu> menu = new QMenu(this);

    QAction *actSetVal = menu->addAction(tr("Use this Value"),
                                         this,
                                         [this, idx]()
    {
        QPointF val = mFilterModel->getValueAtIdx(idx);
        mFilterModel->storeValueInCurrentCurve(idx, val);
    });
    actSetVal->setVisible(currEdit != -1);

    if(currEdit == idx.column())
    {
        menu->addAction(tr("Custom Value"),
                        this,
                        [this, idx]()
        {
            QPointF val = mFilterModel->getValueAtIdx(idx);
            if(val.x() < 0)
                return;

            val.ry() = QInputDialog::getDouble(this,
                                               tr("Set Custom Value"),
                                               tr("Value:"),
                                               val.y());
            if(val.ry() < 0 || qFuzzyIsNull(val.ry()))
                val.ry() = -1;
            mFilterModel->storeValueInCurrentCurve(idx, val);
        });
    }
    else if(currEdit != -1)
    {
        menu->addAction(tr("Copy This Curve"),
                        this,
                        [this, idx]()
        {
            mFilterModel->storeFirstOfEachStepInCurrentCurve(idx.column());
        });
    }

    if(currEdit != -1)
        menu->addSeparator();

    menu->addAction(tr("Change Name"),
                    this, [this, idx]()
    {
        QString name = mFilterModel->getSeriesName(idx.column());
        name = QInputDialog::getText(this,
                                     tr("Choose Name"),
                                     tr("Series Name:"),
                                     QLineEdit::Normal,
                                     name);
        name = name.simplified();
        if(!name.isEmpty())
            mFilterModel->setSeriesName(idx.column(), name);
    });

    if(currEdit != idx.column())
    {
        QAction *actEdit = menu->addAction(tr("Edit this curve"),
                                           this,
                                           [this, idx]()
        {
            mFilterModel->setCurrentEditCurve(idx.column());
        });

        actEdit->setEnabled(colType == SpeedCurveTableModel::ColumnType::StoredSpeedCurve);
    }
    else
    {
        menu->addAction(tr("End Edit"),
                        this,
                        [this, idx]()
        {
            mFilterModel->setCurrentEditCurve(-1);
        });
    }

    QAction *actRem = menu->addAction(tr("Remove this curve"),
                                      this,
                                      [this, idx]()
    {
        mFilterModel->removeCurveAt(idx.column());
    });

    actRem->setEnabled(colType == SpeedCurveTableModel::ColumnType::StoredSpeedCurve);

    if(currEdit == -1)
    {
        menu->addAction(tr("New Curve"),
                        this, [this]()
        {
            QString name;
            name = QInputDialog::getText(this,
                                         tr("Add New Curve"),
                                         tr("Name:"));
            mFilterModel->addNewCurve(name);
        });

        if(colType == SpeedCurveTableModel::ColumnType::StoredSpeedCurve)
        {
            menu->addAction(tr("Load from File"),
                            this, [this, idx]()
            {
                QString fileName;
                fileName = QFileDialog::getOpenFileName(this,
                                                        tr("Open Curve File"));
                if(fileName.isEmpty())
                    return;

                QLineSeries *s = mFilterModel->getCurveAt(idx.column());
                if(!s)
                    return;
                RawSpeedCurveIO::readCurveFromFile(fileName, s);
                mFilterModel->updateCurveAt(idx.column());
            });

            menu->addAction(tr("Save to File"),
                            this, [this, idx]()
            {
                QString fileName;
                fileName = QFileDialog::getSaveFileName(this,
                                                        tr("Save Curve File"));
                if(fileName.isEmpty())
                    return;

                QLineSeries *s = mFilterModel->getCurveAt(idx.column());
                if(!s)
                    return;
                RawSpeedCurveIO::saveCurveToFile(fileName, s);
            });
        }
    }

    menu->exec(mFilterView->viewport()->mapToGlobal(pos));
    if(!menu)
        return;
}

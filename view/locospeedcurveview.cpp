#include "locospeedcurveview.h"

#include "../recorder/locospeedcurve.h"

#include "../chart/chartview.h"
#include "../chart/chart.h"
#include <QValueAxis>

#include <QVBoxLayout>

LocoSpeedCurveView::LocoSpeedCurveView(QWidget *parent)
    : QWidget{parent}
{
    mChart = new Chart;
    mChart->setLocalizeNumbers(false);

    mAxisX = new QValueAxis(this);
    mAxisX->setRange(0, 126);
    mAxisX->setLabelFormat("%.0f");
    mAxisX->setTitleText("Step");

    mAxisY = new QValueAxis(this);
    mAxisY->setRange(0, 2);
    mAxisY->setLabelFormat("%.1f");
    mAxisY->setTitleText("Speed");

    mChart->addAxis(mAxisX, Qt::AlignBottom);
    mChart->addAxis(mAxisY, Qt::AlignLeft);

    QVBoxLayout *lay = new QVBoxLayout(this);
    mChartView = new ChartView(mChart);
    lay->addWidget(mChartView);

    mChart->addSeries(&mTargetSpeedCurve);
    mTargetSpeedCurve.setColor(Qt::red);
    mTargetSpeedCurve.attachAxis(mAxisX);
    mTargetSpeedCurve.attachAxis(mAxisY);

    mChart->addSeries(&mSpeedCurveAvg);
    mSpeedCurveAvg.setColor(Qt::black);
    mSpeedCurveAvg.attachAxis(mAxisX);
    mSpeedCurveAvg.attachAxis(mAxisY);

    mChart->addSeries(&mSpeedCurvePoints);
    mSpeedCurvePoints.setColor(Qt::cyan);
    mSpeedCurvePoints.attachAxis(mAxisX);
    mSpeedCurvePoints.attachAxis(mAxisY);
    mSpeedCurvePoints.setMarkerSize(10);

    connect(mChartView, &ChartView::scrollResetRequested, this,
            [this]()
            {
                // Reset origin to {0,0}
                mAxisX->setRange(0, mAxisX->max() - mAxisX->min());
                mAxisY->setRange(0, mAxisY->max() - mAxisY->min());
            });
}

LocoSpeedCurve *LocoSpeedCurveView::speedCurve() const
{
    return mSpeedCurve;
}

void LocoSpeedCurveView::setSpeedCurve(LocoSpeedCurve *newSpeedCurve)
{
    mSpeedCurveAvg.clear();
    mSpeedCurvePoints.clear();

    mSpeedCurve = newSpeedCurve;

    if(mSpeedCurve)
    {
        disconnect(mSpeedCurve, &LocoSpeedCurve::speedCurveChanged, this, &LocoSpeedCurveView::onCurveChanged);
    }

    mSpeedCurve = newSpeedCurve;

    if(mSpeedCurve)
    {
        connect(mSpeedCurve, &LocoSpeedCurve::speedCurveChanged, this, &LocoSpeedCurveView::onCurveChanged);

        const auto speedCurve = mSpeedCurve->speedCurve();
        for(int key : speedCurve.keys())
        {
            onCurveChanged(key, speedCurve.values(key));
        }
    }
}

void LocoSpeedCurveView::setTargedSpeedCurve(const QVector<double> &targetSpeedCurve)
{
    mTargetSpeedCurve.clear();
    for(int i = 0; i < targetSpeedCurve.size(); i++)
    {
        mTargetSpeedCurve.append(i, targetSpeedCurve[i]);
    }

    if(!targetSpeedCurve.empty())
    {
        mAxisY->setMax(targetSpeedCurve.last() * 1.2);
    }
}

void LocoSpeedCurveView::onCurveChanged(int step, const QList<double> &values)
{
    if(step < 0)
    {
        mSpeedCurveAvg.clear();
        mSpeedCurvePoints.clear();
        return;
    }

    if(mSpeedCurveAvg.count() <= step)
    {
        for(int i = mSpeedCurveAvg.count(); i <= step; i++)
        {
            // Fill in new points
            mSpeedCurveAvg.append(i, 0);
        }
    }

    // Replace old points
    int x = values.size() - 1;
    for(int i = 0; i < mSpeedCurvePoints.count(); i++)
    {
        if(mSpeedCurvePoints.at(i).x() == step)
        {
            if(x < 0)
            {
                // Remove points in excess
                mSpeedCurvePoints.remove(i);
            }
            else
            {
                // Update points if different
                if(mSpeedCurvePoints.at(i).y() != values[x])
                {
                    mSpeedCurvePoints.replace(i, step, values[x]);
                }
                x--;
            }
        }
    }

    // Add new points
    for(; x >= 0; x--)
    {
        mSpeedCurvePoints.append(step, values[x]);
    }

    // Update average
    double sum = 0;
    for(const double& item : values)
    {
        sum += item;
    }

    const double avg = sum / values.size();
    mSpeedCurveAvg.replace(step, step, avg);
}

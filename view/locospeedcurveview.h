#ifndef LOCOSPEEDCURVEVIEW_H
#define LOCOSPEEDCURVEVIEW_H

#include <QWidget>
#include <QLineSeries>
#include <QScatterSeries>

class Chart;
class ChartView;
class QValueAxis;

class LocoSpeedCurve;

class LocoSpeedCurveView : public QWidget
{
    Q_OBJECT
public:
    explicit LocoSpeedCurveView(QWidget *parent = nullptr);

    LocoSpeedCurve *speedCurve() const;
    void setSpeedCurve(LocoSpeedCurve *newSpeedCurve);

    void setTargedSpeedCurve(const QVector<double>& targetSpeedCurve);

private slots:
    void onCurveChanged(int step, const QList<double> &values);

private:
    LocoSpeedCurve *mSpeedCurve = nullptr;

    Chart *mChart;
    ChartView *mChartView;

    QLineSeries mTargetSpeedCurve;
    QLineSeries mSpeedCurveAvg;
    QScatterSeries mSpeedCurvePoints;

    QValueAxis *mAxisX;
    QValueAxis *mAxisY;
};

#endif // LOCOSPEEDCURVEVIEW_H

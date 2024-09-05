#ifndef DATASERIESCURVEMAPPING_H
#define DATASERIESCURVEMAPPING_H

#include "../idataseries.h"

#include <QVector>

class DataSeriesCurveMapping : public IDataSeries
{
    Q_OBJECT
public:
    DataSeriesCurveMapping(QObject *parent = nullptr);

    IDataSeries *source() const;
    void setSource(IDataSeries *newSource);

    DataSeriesType getType() const override;
    int getPointCount() const override;
    QPointF getPointAt(int index) const override;
    QString getPointTooltip(int index) const override;

    IDataSeries *recvStep() const;
    void setRecvStep(IDataSeries *newRecvStep);

private slots:
    void onSourcePointAdded(int index, const QPointF& point);
    void onSourceDestroyed(QObject *source);

    void recalculate();

private:
    int calculateAvg(int fromSourceIdx);

private:
    IDataSeries *mSource;
    IDataSeries *mRecvStepSeries;

    QVector<QPointF> mPoints;

    int mLastSourceIdx = 0;
};

#endif // DATASERIESCURVEMAPPING_H

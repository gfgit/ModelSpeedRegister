#ifndef SENSORTRAVELLEDDISTANCESERIES_H
#define SENSORTRAVELLEDDISTANCESERIES_H

#include "../idataseries.h"

#include <QVector>

class SensorTravelledDistanceSeries : public IDataSeries
{
    Q_OBJECT
public:
    explicit SensorTravelledDistanceSeries(QObject *parent = nullptr);

    virtual DataSeriesType getType() const override;
    virtual int getPointCount() const override;
    virtual QPointF getPointAt(int index) const override;
    virtual QString getPointTooltip(int index) const override;

    void addPoint(double speed, double seconds);
    void clear();

private:
    QVector<QPointF> mPoints;
};

#endif // SENSORTRAVELLEDDISTANCESERIES_H

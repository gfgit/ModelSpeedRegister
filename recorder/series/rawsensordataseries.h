#ifndef RAWSENSORDATASERIES_H
#define RAWSENSORDATASERIES_H

#include "../idataseries.h"

#include <QVector>

class RecordingManager;

class RawSensorDataSeries : public IDataSeries
{
    Q_OBJECT
public:
    explicit RawSensorDataSeries(QObject *parent = nullptr);

    virtual DataSeriesType getType() const override;
    virtual int getPointCount() const override;
    virtual QPointF getPointAt(int index) const override;
    virtual QString getPointTooltip(int index) const override;

    void addPoint(double speed, double seconds);
    void clear();

private:
    QVector<QPointF> mPoints;
};

#endif // RAWSENSORDATASERIES_H

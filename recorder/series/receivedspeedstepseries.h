#ifndef RECEIVEDSPEEDSTEPSERIES_H
#define RECEIVEDSPEEDSTEPSERIES_H

#include "../idataseries.h"

#include <QVector>

class RecordingManager;

class ReceivedSpeedStepSeries : public IDataSeries
{
    Q_OBJECT
public:
    explicit ReceivedSpeedStepSeries(QObject *parent = nullptr);

    virtual DataSeriesType getType() const override;
    virtual int getPointCount() const override;
    virtual QPointF getPointAt(int index) const override;
    virtual QString getPointTooltip(int index) const override;

    void addPoint(int reqStep, double seconds);
    void clear();

private:
    QVector<QPointF> mPoints;
};

#endif // RECEIVEDSPEEDSTEPSERIES_H

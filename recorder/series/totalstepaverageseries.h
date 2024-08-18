#ifndef TOTALSTEPAVERAGESERIES_H
#define TOTALSTEPAVERAGESERIES_H

#include "../idataseries.h"

#include <QVector>

class TotalStepAverageSeries : public IDataSeries
{
    Q_OBJECT
public:
    TotalStepAverageSeries(QObject *parent = nullptr);

    IDataSeries *travelledSource() const;
    void setTravelledSource(IDataSeries *newSource);

    DataSeriesType getType() const override;
    int getPointCount() const override;
    QPointF getPointAt(int index) const override;
    QString getPointTooltip(int index) const override;

    IDataSeries *reqStepSeries() const;
    void setReqStepSeries(IDataSeries *newReqStepSeries);

    IDataSeries *recvStepSeries() const;
    void setRecvStepSeries(IDataSeries *newRecvStepSeries);

    qint64 accelerationMilliseconds() const;
    void setAccelerationMilliseconds(qint64 newAccelerationMilliseconds);

private slots:
    void onRecvStepPointAdded(int indexAdded, const QPointF& point);
    void onSensorPointAdded(int indexAdded, const QPointF& point);
    void onReqStepPointAdded(int indexAdded, const QPointF &point);
    void onPointChanged();
    void onPointRemoved();
    void onSourceDestroyed(QObject *source);

private:
    void updateAvg(int index, const QPointF &point, DataSeriesAction action);
    void recalculate();
    int calculateAvg(int fromRecvIdx);

private:
    IDataSeries *mTravelledSource;
    IDataSeries *mReqStepSeries;
    IDataSeries *mRecvStepSeries;

    QVector<QPointF> mPoints;

    qint64 mAccelerationMilliseconds = 1000;
    int lastRecvStepIdx = 1;
    bool waitingForMoreSensorData = false;
    bool waitingForRequestEnd = false;
};

#endif // TOTALSTEPAVERAGESERIES_H

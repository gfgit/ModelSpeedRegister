#ifndef MOVINGAVERAGESERIES_H
#define MOVINGAVERAGESERIES_H

#include "../idataseries.h"

#include <QVector>

class MovingAverageSeries : public IDataSeries
{
    Q_OBJECT
public:
    MovingAverageSeries(int windowSize, QObject *parent = nullptr);

    IDataSeries *source() const;
    void setSource(IDataSeries *newSource);

    DataSeriesType getType() const override;
    int getPointCount() const override;
    QPointF getPointAt(int index) const override;
    QString getPointTooltip(int index) const override;

private slots:
    void onPointAdded(int index, const QPointF& point);
    void onPointChanged(int indexChanged, const QPointF &point);
    void onPointRemoved(int indexRemoved);
    void onSourceDestroyed();

private:
    void updateAvg(int index, const QPointF &point, DataSeriesAction action);

private:
    IDataSeries *mSource;

    QVector<QPointF> mPoints;

    int mWindowSize;
};

#endif // MOVINGAVERAGESERIES_H

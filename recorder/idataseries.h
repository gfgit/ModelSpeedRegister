#ifndef IDATASERIES_H
#define IDATASERIES_H

#include <QObject>

#include <QString>
#include <QPointF>

enum DataSeriesType
{
    Unknown = 0,
    RequestedSpeedStep,
    ReceivedSpeedStep,
    SensorRawData,
    MovingAverage,
    TotalStepAverage
};

enum DataSeriesAction
{
    PointAdded = 0,
    PointChanged,
    PointRemoved
};

class IDataSeries : public QObject
{
    Q_OBJECT
public:
    explicit IDataSeries(QObject *parent = nullptr);

    QString name() const;
    void setName(const QString &newName);

    virtual DataSeriesType getType() const = 0;
    virtual int getPointCount() const = 0;
    virtual QPointF getPointAt(int index) const = 0;
    virtual QString getPointTooltip(int index) const = 0;

    static QString defaultTooltip(const QString &seriesName, int index, const QPointF& point);

signals:
    void pointAdded(int index, const QPointF& point);
    void pointRemoved(int index);
    void pointChanged(int index, const QPointF& newPoint);

private:
    QString mName;
};

#endif // IDATASERIES_H

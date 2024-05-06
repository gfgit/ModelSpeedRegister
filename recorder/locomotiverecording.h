#ifndef LOCOMOTIVERECORDING_H
#define LOCOMOTIVERECORDING_H

#include <QObject>
#include <QVector>
#include <QDateTime>

#include "../commandstation/utils.h"

struct RecordingItem
{
    qint64 timestampMilliSec  = 0;
    int requestedSpeedStep    = 0;
    int actualSpeedStep       = 0;
    double metersPerSecond    = 0;
    double metersPerSecondAvg = 0; // Moving average
};

class LocomotiveRecording : public QObject
{
    Q_OBJECT
public:
    explicit LocomotiveRecording(QObject *parent = nullptr);

    void addItem(const RecordingItem& item);

signals:
    void itemChanged(int index);

private:
    void calculateMovingAverage(int index);

private:
    QVector<RecordingItem> mItems;
    int mLocomotiveDCCAddress = 0;
    LocomotiveDirection mDirection;
    QString mRecordingName;
    QDateTime mRecordingDate;

    const int MovingAverageWindow = 5;
};

#endif // LOCOMOTIVERECORDING_H

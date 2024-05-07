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

    inline RecordingItem getItemAt(int index) const { return mItems.at(index); }
    inline int getItemCount() const { return mItems.count(); }
    void clear();

signals:
    void itemChanged(int index, const RecordingItem& oldItem);

private:
    void calculateMovingAverage(int index);

private:
    QVector<RecordingItem> mItems;
    int mLocomotiveDCCAddress = 0;
    LocomotiveDirection mDirection;
    QString mRecordingName;
    QDateTime mRecordingDate;

    const int MovingAverageWindow = 9;
};

#endif // LOCOMOTIVERECORDING_H

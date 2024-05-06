#include "locomotiverecording.h"

LocomotiveRecording::LocomotiveRecording(QObject *parent)
    : QObject{parent}
{

}

void LocomotiveRecording::addItem(const RecordingItem &item)
{
    mItems.append(item);

    const int lastItemIndex = mItems.size() - 1;
    emit itemChanged(lastItemIndex);

    const int halfWindow = (MovingAverageWindow - 1) / 2;
    const int lastAvgItem = lastItemIndex - halfWindow;
    calculateMovingAverage(lastAvgItem);
}

void LocomotiveRecording::calculateMovingAverage(int index)
{
    const int halfWindow = (MovingAverageWindow - 1) / 2;
    const int firstItem = index - halfWindow;
    const int lastItem = index + halfWindow;

    assert(mItems.size() > lastItem);

    double sum = 0;

    for(int i = firstItem; i <= lastItem; i++)
    {
        sum += mItems[i].metersPerSecond;
    }

    mItems[index].metersPerSecondAvg = sum / MovingAverageWindow;
    emit itemChanged(index);
}

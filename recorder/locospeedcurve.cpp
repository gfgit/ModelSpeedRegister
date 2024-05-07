#include "locospeedcurve.h"

#include "locomotiverecording.h"

LocoSpeedCurve::LocoSpeedCurve(QObject *parent)
    : QObject{parent}
{

}

LocomotiveRecording *LocoSpeedCurve::recording() const
{
    return mRecording;
}

void LocoSpeedCurve::setRecording(LocomotiveRecording *newRecording)
{
    mSpeedCurve.clear();

    if(mRecording)
    {
        disconnect(mRecording, &LocomotiveRecording::itemChanged, this, &LocoSpeedCurve::onItemChanged);
    }

    mRecording = newRecording;

    if(mRecording)
    {
        connect(mRecording, &LocomotiveRecording::itemChanged, this, &LocoSpeedCurve::onItemChanged);

        // Fill initial values
        for(int i = 0; i < mRecording->getItemCount(); i++)
        {
            const RecordingItem item = mRecording->getItemAt(i);
            mSpeedCurve.insert(item.actualSpeedStep, item.metersPerSecondAvg);
        }

        const auto keys = mSpeedCurve.keys();
        for(int key : keys)
        {
            emit speedCurveChanged(key, mSpeedCurve.values(key));
        }
    }
}

void LocoSpeedCurve::onItemChanged(int index, const RecordingItem &oldItem)
{
    if(index < 0)
    {
        mSpeedCurve.clear();
        emit speedCurveChanged(-1, {});
        return;
    }

    const RecordingItem item = mRecording->getItemAt(index);

    mSpeedCurve.remove(oldItem.actualSpeedStep, oldItem.metersPerSecondAvg);
    mSpeedCurve.insert(item.actualSpeedStep, item.metersPerSecondAvg);

    emit speedCurveChanged(item.actualSpeedStep, mSpeedCurve.values(item.actualSpeedStep));
}

QMultiHash<int, double> LocoSpeedCurve::speedCurve() const
{
    return mSpeedCurve;
}

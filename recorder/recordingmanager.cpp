#include "recordingmanager.h"

#include "locomotiverecording.h"

RecordingManager::RecordingManager(QObject *parent)
    : QObject{parent}
{

}

void RecordingManager::onNewSpeedReading(double metersPerSecond, LocomotiveDirection direction, qint64 timestampMilliSec)
{
    if(!m_currentRecording)
        return;

    RecordingItem item;
    item.timestampMilliSec = timestampMilliSec;
    item.actualSpeedStep = actualDCCStep;
    item.requestedSpeedStep = requestedDCCStep;
    item.metersPerSecond = metersPerSecond;
    item.metersPerSecondAvg = 0; // Will be calculated later

    //TODO: direction

    m_currentRecording->addItem(item);
}

void RecordingManager::onLocomotiveSpeedFeedback(int address, int speedStep, LocomotiveDirection direction)
{
    if(address != locomotiveDCCAddress)
        return;

    actualDCCStep = speedStep;
    //TODO: direction
}

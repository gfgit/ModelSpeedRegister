#ifndef RECORDINGMANAGER_H
#define RECORDINGMANAGER_H

#include <QObject>

#include "../commandstation/utils.h"

class ICommandStation;
class ISpeedSensor;

class LocomotiveRecording;

class RecordingManager : public QObject
{
    Q_OBJECT
public:
    explicit RecordingManager(QObject *parent = nullptr);
    ~RecordingManager();

    ICommandStation *commandStation() const;
    void setCommandStation(ICommandStation *newCommandStation);

    ISpeedSensor *speedSensor() const;
    void setSpeedSensor(ISpeedSensor *newSpeedSensor);

    LocomotiveRecording *currentRecording() const;

    void timerEvent(QTimerEvent *e) override;

public slots:
    void start();
    void stop();
    void emergencyStop();

private slots:
    void onNewSpeedReading(double metersPerSecond, LocomotiveDirection direction, qint64 timestampMilliSec);
    void onLocomotiveSpeedFeedback(int address, int speedStep, LocomotiveDirection direction);

private:
    ICommandStation *mCommandStation = nullptr;
    ISpeedSensor *mSpeedSensor = nullptr;

    int locomotiveDCCAddress = 3;
    int requestedDCCStep = 0;
    int actualDCCStep = 0;

    LocomotiveRecording *m_currentRecording;

    int mTimerId = 0;
};

#endif // RECORDINGMANAGER_H

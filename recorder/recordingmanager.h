#ifndef RECORDINGMANAGER_H
#define RECORDINGMANAGER_H

#include <QObject>
#include <QElapsedTimer>

#include "../commandstation/utils.h"

class ICommandStation;
class ISpeedSensor;

class IDataSeries;
class RequestedSpeedStepSeries;
class ReceivedSpeedStepSeries;

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

    int getLocomotiveDCCAddress() const;
    void setLocomotiveDCCAddress(int newLocomotiveDCCAddress);

    QVector<IDataSeries *> getSeries() const;

    void registerSeries(IDataSeries *s);
    void unregisterSeries(IDataSeries *s);

    RequestedSpeedStepSeries *reqStepSeries() const;

    ReceivedSpeedStepSeries *recvStepSeries() const;

signals:
    void seriesRegistered(IDataSeries *s);
    void seriesUnregistered(IDataSeries *s);

public slots:
    void start();
    void stop();
    void emergencyStop();

private slots:
    void onNewSpeedReading(double metersPerSecond, double travelledMillimeters, qint64 timestampMilliSec);
    void onLocomotiveSpeedFeedback(int address, int speedStep, LocomotiveDirection direction);

    void onSeriesDestroyed(QObject *s);

private:
    ICommandStation *mCommandStation = nullptr;
    ISpeedSensor *mSpeedSensor = nullptr;

    int locomotiveDCCAddress = 47;
    int requestedDCCStep = 0;
    int actualDCCStep = 0;

    LocomotiveRecording *m_currentRecording;

    int mTimerId = 0;
    qint64 mStartTimestamp = -1;

    QVector<IDataSeries *> mSeries;

    RequestedSpeedStepSeries *mReqStepSeries;
    ReceivedSpeedStepSeries *mRecvStepSeries;

    QElapsedTimer mElapsed;
};

#endif // RECORDINGMANAGER_H

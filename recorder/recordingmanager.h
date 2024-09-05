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
class RawSensorDataSeries;
class SensorTravelledDistanceSeries;

class RecordingManager : public QObject
{
    Q_OBJECT
public:
    enum class State
    {
        Stopped = 0,
        Running = 1,
        WaitingToStop = 2
    };

    explicit RecordingManager(QObject *parent = nullptr);
    ~RecordingManager();

    ICommandStation *commandStation() const;
    void setCommandStation(ICommandStation *newCommandStation);

    ISpeedSensor *speedSensor() const;
    void setSpeedSensor(ISpeedSensor *newSpeedSensor);

    void timerEvent(QTimerEvent *e) override;

    int getLocomotiveDCCAddress() const;
    void setLocomotiveDCCAddress(int newLocomotiveDCCAddress);

    QVector<IDataSeries *> getSeries() const;

    void registerSeries(IDataSeries *s);
    void unregisterSeries(IDataSeries *s);

    RequestedSpeedStepSeries *reqStepSeries() const;

    ReceivedSpeedStepSeries *recvStepSeries() const;

    RawSensorDataSeries *rawSensorSeries() const;

    void requestStep(int step);

    SensorTravelledDistanceSeries *sensorTravelledSeries() const;

    int startingDCCStep() const;
    void setStartingDCCStep(int newStartingDCCStep);

    int defaultStepTimeMillis() const;
    void setDefaultStepTimeMillis(int newDefaultStepTimeMillis);

    void goToNextStep();
    void setCustomTimeForCurrentStep(int millis);

    State state() const;

signals:
    void seriesRegistered(IDataSeries *s);
    void seriesUnregistered(IDataSeries *s);
    void stateChanged(int newState);

public slots:
    bool start();
    void stop();
    void emergencyStop();

private slots:
    void onNewSpeedReading(double metersPerSecond, double travelledMillimeters, qint64 timestampMilliSec);
    void onLocomotiveSpeedFeedback(int address, int speedStep, LocomotiveDirection direction);

    void onSeriesDestroyed(QObject *s);

private:
    void requestStepInternal(int step);

    void setState(State newState);

    void tryStopInternal();
    void stopInternal();

private:
    ICommandStation *mCommandStation = nullptr;
    ISpeedSensor *mSpeedSensor = nullptr;

    int locomotiveDCCAddress = 3;
    int requestedDCCStep = 0;
    int actualDCCStep = 0;
    bool mReqStepIsPending = false;

    int mStartingDCCStep = 1;
    int mDefaultStepTimeMillis = 3000;
    bool currentTimerIsCustom = false;

    int mStepTimerId = 0;
    qint64 mStartTimestamp = -1;

    int mForceStopTimerId = 0;

    QVector<IDataSeries *> mSeries;

    RequestedSpeedStepSeries *mReqStepSeries;
    ReceivedSpeedStepSeries *mRecvStepSeries;
    RawSensorDataSeries *mRawSensorSeries;
    SensorTravelledDistanceSeries *mSensorTravelledSeries;

    State mState = State::Stopped;

    QElapsedTimer mElapsed;
};

#endif // RECORDINGMANAGER_H

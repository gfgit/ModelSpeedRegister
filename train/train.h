#ifndef TRAIN_H
#define TRAIN_H

#include <QObject>
#include <QVector>

#include <QElapsedTimer>

#include "trainspeedtable.h"

#include "../commandstation/utils.h"

class Locomotive;

class Train : public QObject
{
    Q_OBJECT
public:
    Train(QObject *parent = nullptr);

    Locomotive *getLocoAt(int idx) const;
    bool getLocoInvertDirAt(int idx) const;

    int getLocoIdx(Locomotive *loco) const;

    inline bool isActive() const
    {
        return active;
    }

    bool addLoco(Locomotive *loco);
    bool removeLoco(Locomotive *loco);
    void updateSpeedTable();

    void setDirection(LocomotiveDirection dir);
    void setMaximumSpeed(double speed);

    void setEmergencyStop();

signals:
    void activeChanged(bool active);

public slots:
    bool setActive(bool newActive);
    void setLocoInvertDir(int idx, bool invertDir);

private:  
    enum
    {
        INVALID_LOCO_IDX = -1
    };

    struct SpeedPoint
    {
        double speed = 0;
        int tableIdx = TrainSpeedTable::NULL_TABLE_ENTRY;
    };

    enum class State
    {
        Idle = 0,
        Accelerating,
        Braking
    };

    void timerEvent(QTimerEvent *e) override;

    void startDelayedSpeedApply(int locoIdx);
    void stopDelayedSpeedApply();
    void applyDelayedSpeed();

    void onLocoChangedInternal(int locoIdx, int step);
    void setTargetSpeedInternal(double speed,
                                int tableIdx = -1,
                                int sourceLocoIdx = -1);

    void setSpeedInternal(const SpeedPoint& speedPoint);

    void driveLoco(int locoIdx, int step);

    void scheduleAccelerationFrom(double currentSpeed,
                                  int newTableIdx,
                                  State state);

    void updateAccelerationState();

private slots:
    void onLocoChanged(Locomotive *loco, bool queued);

private:
    struct LocoItem
    {
        Locomotive *loco = nullptr;
        bool invertDir = false;
        int lastSetStep = 0;
    };

    QVector<LocoItem> mLocomotives;

    TrainSpeedTable mSpeedTable;
    bool active = false;

    int mApplySpeedTimerId = 0;
    int mApplySpeedLocoIdx = INVALID_LOCO_IDX;

    SpeedPoint mTargetSpeed;
    SpeedPoint mMaxSpeed;
    SpeedPoint mLastSetSpeed;

    LocomotiveDirection mDirection = LocomotiveDirection::Forward;

    int mAccelerationTimerId = 0;
    QElapsedTimer mAccelerationElapsed;

    // TODO: allow setting
    double mAccelerationRate = 1.5 / 87.0;
    double mDecelerationRate = 1.0 / 87.0;

    State mState = State::Idle;
};

#endif // TRAIN_H

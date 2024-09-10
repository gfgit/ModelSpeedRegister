#ifndef TRAIN_H
#define TRAIN_H

#include <QObject>
#include <QVector>

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

    bool getActive() const;

    void addLoco(Locomotive *loco);
    bool removeLoco(Locomotive *loco);
    void updateSpeedTable();

    void setDirection(LocomotiveDirection dir);
    void setMaximumSpeed(double speed);

private:
    struct SpeedPoint
    {
        double speed = 0;
        int tableIdx = 0;
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

public slots:
    void setActive(bool newActive);
    void setLocoInvertDir(int idx, bool invertDir);

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

    bool mInsideLocoUpdate = false;

    int mApplySpeedTimerId = 0;
    int mApplySpeedLocoIdx = -1;

    SpeedPoint mTargetSpeed;
    SpeedPoint mMaxSpeed;
    SpeedPoint mLastSetSpeed;

    LocomotiveDirection mDirection;
};

#endif // TRAIN_H

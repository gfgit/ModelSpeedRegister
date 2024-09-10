#ifndef LOCOMOTIVE_H
#define LOCOMOTIVE_H

#include <QObject>

#include "../commandstation/utils.h"

#include "locospeedmapping.h"

class LocomotivePool;
class Train;

class Locomotive : public QObject
{
    Q_OBJECT
public:
    explicit Locomotive(QObject *parent = nullptr);
    ~Locomotive();

    int address() const;
    void setAddress(int newAddress);

    int speedStep() const;

    LocomotiveDirection direction() const;

    const LocoSpeedMapping& speedMapping() const;
    void setSpeedMapping(const LocoSpeedMapping &newSpeedMapping);

    void driveLoco(int speedStep, LocomotiveDirection direction);

    int targetSpeedStep() const;

    LocomotiveDirection targetDirection() const;

signals:
    void changed(Locomotive *self, bool queued);
    void nameChanged(const QString& name);

private:
    friend class LocomotivePool;

    void setSpeed_internal(int speedStep, LocomotiveDirection dir, bool wasQueued);

private:
    LocomotivePool *mPool = nullptr;
    Train *mTrain = nullptr;

    LocoSpeedMapping mSpeedMapping;

    int mAddress = 0;

    int mSpeedStep = 0;
    LocomotiveDirection mDirection = LocomotiveDirection::Forward;

    int mTargetSpeedStep = 0;
    LocomotiveDirection mTargetDirection = LocomotiveDirection::Forward;
};

#endif // LOCOMOTIVE_H

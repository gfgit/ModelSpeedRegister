#ifndef LOCOMOTIVEPOOL_H
#define LOCOMOTIVEPOOL_H

#include <QObject>
#include <QVector>

#include "../commandstation/utils.h"

class ICommandStation;
class Locomotive;

class LocomotivePool : public QObject
{
    Q_OBJECT
public:
    explicit LocomotivePool(QObject *parent = nullptr);
    ~LocomotivePool();

    void addLoco(Locomotive *loco);
    void removeLoco(Locomotive *loco);

    ICommandStation *commandStation() const;
    void setCommandStation(ICommandStation *newCommandStation);

private slots:
    void onLocomotiveSpeedFeedback(int address, int speedStep, LocomotiveDirection direction);

private:
    friend class Locomotive;
    void driveLoco(int address, int speedStep, LocomotiveDirection direction);

    ICommandStation *mCommandStation = nullptr;

    QVector<Locomotive *> mLocos;
};

#endif // LOCOMOTIVEPOOL_H

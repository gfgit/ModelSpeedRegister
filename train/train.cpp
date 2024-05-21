#include "train.h"

#include "locomotive.h"

#include <QDebug>

Train::Train(QObject *parent) :
    QObject(parent)
{

}

Train *Train::createTrain(Locomotive *a, Locomotive *b)
{
    Train *train = new Train;
    train->locoA = a;
    train->locoB = b;

    train->mSpeedTable = TrainSpeedTable::buildTable(a->speedMapping(), b->speedMapping());

    connect(a, &Locomotive::changed, train, &Train::onLocoChanged);
    connect(b, &Locomotive::changed, train, &Train::onLocoChanged);

    return train;
}

void Train::onLocoChanged(Locomotive *loco)
{
    if(!active)
        return;

    LocomotiveDirection trainDirection = loco->direction();
    auto entry = mSpeedTable.getClosestMatch(loco->address(), loco->speedStep());

    bool changed = false;

    if(locoA->targetSpeedStep() != entry.locoA.step || locoA->targetDirection() != trainDirection)
    {
        changed = true;
        locoA->driveLoco(entry.locoA.step, trainDirection);
    }

    if(locoB->targetSpeedStep() != entry.locoB.step || locoB->targetDirection() != trainDirection)
    {
        changed = true;
        locoB->driveLoco(entry.locoB.step, trainDirection);
    }

    qDebug() << "            TRAIN: A:" << entry.locoA.step << "B:" << entry.locoB.step
             << "FROM:" << (loco == locoA ? 'A' : 'B') << loco->speedStep();
}

bool Train::getActive() const
{
    return active;
}

void Train::setActive(bool newActive)
{
    active = newActive;
}

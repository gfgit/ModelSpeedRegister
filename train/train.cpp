#include "train.h"

#include "locomotive.h"

#include <QDebug>

#include <QTimer>

LocomotiveDirection oppositeDir(LocomotiveDirection dir)
{
    if(dir == LocomotiveDirection::Forward)
        return LocomotiveDirection::Reverse;
    return LocomotiveDirection::Forward;
}

Train::Train(QObject *parent) :
    QObject(parent)
{
    timerA.start();
    timerB.start();
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

void Train::onLocoChanged(Locomotive *loco, bool queued)
{
    if(!active || queued)
        return;

    qDebug() << "TRAIN CHANGE: adr=" << loco->address();

//    QElapsedTimer *t = nullptr;

//    if(loco == locoA)
//        t = &timerA;
//    else
//        t = &timerB;

//    if(t->elapsed() < 100)
//    {
//        bool &scheduled = loco == locoA ? locoAScheduled : locoBScheduled;
//        if(!scheduled)
//        {
//            scheduled = true;

//            QTimer::singleShot(100, this, [this, loco]()
//            {
//                if(loco == locoA)
//                    locoAScheduled = false;
//                else
//                    locoBScheduled = false;

//                onLocoChanged(locoA);
//            });
//        }

//        return;
//    }

//    t->restart();

    bool invert = loco == locoA ? locoAInvert : locoBInvert;

    LocomotiveDirection trainDirection = loco->direction();
    if(invert)
        trainDirection = oppositeDir(trainDirection);

    auto entry = mSpeedTable.getClosestMatch(loco->address(), loco->speedStep());

    bool changed = false;

    LocomotiveDirection locoADir = trainDirection;
    if(locoAInvert)
        locoADir = oppositeDir(locoADir);

    LocomotiveDirection locoBDir = trainDirection;
    if(locoBInvert)
        locoBDir = oppositeDir(locoBDir);

    if(locoA->targetSpeedStep() != entry.locoA.step || locoA->targetDirection() != locoADir)
    {
        changed = true;
        locoA->driveLoco(entry.locoA.step, locoADir);
    }

    if(locoB->targetSpeedStep() != entry.locoB.step || locoB->targetDirection() != locoBDir)
    {
        changed = true;
        locoB->driveLoco(entry.locoB.step, locoBDir);
    }

    qDebug() << "            TRAIN: A:" << entry.locoA.step << "B:" << entry.locoB.step
             << "FROM:" << (loco == locoA ? 'A' : 'B') << loco->speedStep();
}

bool Train::getLocoBInvert() const
{
    return locoBInvert;
}

void Train::setLocoBInvert(bool newLocoBInvert)
{
    locoBInvert = newLocoBInvert;
}

bool Train::getLocoAInvert() const
{
    return locoAInvert;
}

void Train::setLocoAInvert(bool newLocoAInvert)
{
    locoAInvert = newLocoAInvert;
}

bool Train::getActive() const
{
    return active;
}

void Train::setActive(bool newActive)
{
    active = newActive;
}

#ifndef TRAIN_H
#define TRAIN_H

#include <QObject>

#include "trainspeedtable.h"

class Locomotive;

class Train : public QObject
{
    Q_OBJECT
public:
    Train(QObject *parent = nullptr);

    static Train *createTrain(Locomotive *a, Locomotive *b);

    bool getActive() const;

public slots:
    void setActive(bool newActive);

private slots:
    void onLocoChanged(Locomotive *loco);

private:
    Locomotive *locoA;
    Locomotive *locoB;
    TrainSpeedTable mSpeedTable;
    bool active = false;
};

#endif // TRAIN_H

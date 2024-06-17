#ifndef TRAIN_H
#define TRAIN_H

#include <QObject>
#include <QElapsedTimer>

#include "trainspeedtable.h"

class Locomotive;

class Train : public QObject
{
    Q_OBJECT
public:
    Train(QObject *parent = nullptr);

    static Train *createTrain(Locomotive *a, Locomotive *b);

    bool getActive() const;

    bool getLocoAInvert() const;

    bool getLocoBInvert() const;
    void setLocoBInvert(bool newLocoBInvert);

public slots:
    void setActive(bool newActive);
    void setLocoAInvert(bool newLocoAInvert);

private slots:
    void onLocoChanged(Locomotive *loco, bool queued);

private:
    Locomotive *locoA;
    Locomotive *locoB;

    bool locoAInvert = false;
    bool locoBInvert = false;

    TrainSpeedTable mSpeedTable;
    bool active = false;

    QElapsedTimer timerA;
    QElapsedTimer timerB;

    bool locoAScheduled = false;
    bool locoBScheduled = false;
};

#endif // TRAIN_H

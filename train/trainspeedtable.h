#ifndef TRAINSPEEDTABLE_H
#define TRAINSPEEDTABLE_H

#include <QVector>

class LocoSpeedMapping;

class TrainSpeedTable
{
public:
    struct Item
    {
        int step = 0;
        double speed = 0;
    };

    struct Entry
    {
        // Todo support arbitrary loco number in train
        Item locoA;
        Item locoB;
    };

    TrainSpeedTable();

    Entry getClosestMatch(int address, int step) const;

    static TrainSpeedTable buildTable(const LocoSpeedMapping& locoA, const LocoSpeedMapping &locoB);

private:
    int mLocoA_Address = 0;
    int mLocoB_Address = 0;

    QVector<Entry> mEntries;
};

#endif // TRAINSPEEDTABLE_H

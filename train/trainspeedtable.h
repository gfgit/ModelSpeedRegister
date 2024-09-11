#ifndef TRAINSPEEDTABLE_H
#define TRAINSPEEDTABLE_H

#include <QVector>

class LocoSpeedMapping;

class TrainSpeedTable
{
public:
    enum
    {
        NULL_TABLE_ENTRY = -1
    };

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
        double avgSpeed = 0;

        inline void updateAvg()
        {
            avgSpeed = (locoA.speed + locoB.speed) / 2.0;
        }

        inline Item itemForLoco(int idx) const
        {
            // TODO support more than 2 locos
            return idx == 0 ? locoA : locoB;
        }
    };

    TrainSpeedTable();

    typedef std::pair<int, Entry> ClosestMatchRet;

    ClosestMatchRet getClosestMatch(int locoIdx, int step) const;

    ClosestMatchRet getClosestMatch(double speed) const;

    inline int count() const { return mEntries.count(); }
    inline Entry getEntryAt(int idx) const { return mEntries.value(idx, {}); }

    static TrainSpeedTable buildTable(const LocoSpeedMapping& locoA, const LocoSpeedMapping &locoB);

private:
    QVector<Entry> mEntries;
};

#endif // TRAINSPEEDTABLE_H

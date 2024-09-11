#include "trainspeedtable.h"

#include "locospeedmapping.h"

TrainSpeedTable::TrainSpeedTable()
{

}

TrainSpeedTable::ClosestMatchRet TrainSpeedTable::getClosestMatch(int locoIdx, int step) const
{
    if(step == 0)
    {
        return {NULL_TABLE_ENTRY, Entry{Item{0, 0}, Item{0, 0}}};
    }

    if(locoIdx < 0 || locoIdx >= 2) // TODO: support more than 2 locos
        return {NULL_TABLE_ENTRY, {}}; // Error

    for(int i = 0; i < mEntries.size(); i++)
    {
        const Entry &e = mEntries.at(i);
        const Item &item = e.itemForLoco(locoIdx);
        if(item.step == step)
            return {i, e}; // Exact match

        if(item.step > step)
        {
            // We got closest higher step
            // Check if lower step is even closer
            if(i > 0)
            {
                const Entry &prev = mEntries.at(i - 1);
                const Item &prevItem = prev.itemForLoco(locoIdx);
                if((step - prevItem.step) < (item.step - step))
                {
                    // Previous match is closer
                    return {i - 1, prev};
                }

                // Higher match is closer
                return {i, e};
            }
        }
    }

    // Return highest match if available
    if(!mEntries.empty())
        return {mEntries.size(), mEntries.last()};

    return {NULL_TABLE_ENTRY, {}}; // Empty table
}

TrainSpeedTable::ClosestMatchRet TrainSpeedTable::getClosestMatch(double speed) const
{
    int lastIdx = 0;

    for(int i = 0; i < mEntries.size(); i++)
    {
        const Entry& entry = mEntries.at(i);
        if(qFuzzyCompare(entry.avgSpeed, speed))
            return {i, entry};

        if(entry.avgSpeed > speed)
        {
            // Return previous entry
            if(i > 0)
            {
                return {i - 1, mEntries.at(i - 1)};
            }

            // No matches below this, return zero
            return {NULL_TABLE_ENTRY, {}};
        }
    }

    // Return highest match if available
    if(!mEntries.empty())
        return {mEntries.size(), mEntries.last()};

    return {NULL_TABLE_ENTRY, {}}; // Empty table
}

TrainSpeedTable::Entry TrainSpeedTable::getEntryAt(int idx) const
{
    // NULL_TABLE_ENTRY returns null Entry as expected!
    return mEntries.value(idx, {});
}

TrainSpeedTable TrainSpeedTable::buildTable(const LocoSpeedMapping &locoA, const LocoSpeedMapping &locoB)
{
    // TODO: prefer pushing/pulling, more than 2 locos

    TrainSpeedTable table;

    // Build a table with A speed matching a B speed
    int stepB = 1;
    for(int stepA = 1; stepA <= 126; stepA++)
    {
        double speedA = locoA.getSpeedForStep(stepA);
        if(qFuzzyIsNull(speedA))
            continue;

        Entry e;
        e.locoA.step = stepA;
        e.locoA.speed = speedA;

        bool inserted = false;
        for(; stepB <= 126; stepB++)
        {
            double speedB = locoB.getSpeedForStep(stepB);
            if(qFuzzyIsNull(speedB))
                continue;

            e.locoB.step = stepB;
            e.locoB.speed = speedB;
            e.updateAvg();

            if(qFuzzyCompare(speedA, speedB))
            {
                // Exact match!!!
                table.mEntries.append(e);
                inserted = true;
                break;
            }
            else if(speedB > speedA)
            {
                // We got closest higher speed
                if(stepB > 0)
                {
                    double prevSpeedB = locoB.getSpeedForStep(stepB - 1);
                    if((speedA - prevSpeedB) < (speedB - speedA))
                    {
                        // Previous match is closer
                        e.locoB.step = stepB - 1;
                        e.locoB.speed = prevSpeedB;
                        e.updateAvg();

                        // Append only if not already contained
                        if(table.mEntries.isEmpty() || table.mEntries.last().locoA.step != stepA)
                        {
                            table.mEntries.append(e);
                            inserted = true;
                        }
                    }
                    else
                    {
                        table.mEntries.append(e);
                        inserted = true;
                    }
                }
                else
                {
                    table.mEntries.append(e);
                    inserted = true;
                }
                break;
            }
        }

        if(!inserted)
        {
            // Match with highest available B speed
            e.locoB.step = 126;
            e.locoB.speed = locoB.getSpeedForStep(126);
            e.updateAvg();
            table.mEntries.append(e);
        }
    }

    // Now we keep only closest matches for given B speed
    // This way both stepA and stepB are unique for each entry

    int previousStep = 0;
    double previousDelta = 0;

    auto it = table.mEntries.begin();
    while(it != table.mEntries.end())
    {
        double delta = abs(it->locoA.speed - it->locoB.speed);

        if(previousStep == it->locoB.step)
        {
            // Check if we have a better match
            if(delta < previousDelta)
            {
                // Erase previous entry
                it = table.mEntries.erase(it - 1);
            }
            else
            {
                // Delete ourself
                it = table.mEntries.erase(it);
                continue;
            }
        }

        previousStep = it->locoB.step;
        previousDelta = delta;
        it++;
    }

    return table;
}

#include "trainspeedtable.h"

#include "locospeedmapping.h"

TrainSpeedTable::TrainSpeedTable()
{

}

TrainSpeedTable::Entry TrainSpeedTable::getClosestMatch(int address, int step) const
{
    if(step == 0)
    {
        return Entry{Item{0, 0}, Item{0, 0}};
    }

    if(address != mLocoA_Address && address != mLocoB_Address)
        return {}; // Error

    for(int i = 0; i < mEntries.size(); i++)
    {
        const Entry &e = mEntries.at(i);
        const Item &item = address == mLocoA_Address ? e.locoA : e.locoB;
        if(item.step == step)
            return e; // Exact match

        if(item.step > step)
        {
            // We got closest higher step
            // Check if lower step is even closer
            if(i > 0)
            {
                const Entry &prev = mEntries.at(i - 1);
                const Item &prevItem = address == mLocoA_Address ? prev.locoA : prev.locoB;
                if((step - prevItem.step) < (item.step - step))
                {
                    // Previous match is closer
                    return prev;
                }

                // Higher match is closer
                return e;
            }
        }
    }

    // Return highest match if available
    if(!mEntries.empty())
        return mEntries.last();

    return {}; // Empty table
}

TrainSpeedTable TrainSpeedTable::buildTable(const LocoSpeedMapping &locoA, const LocoSpeedMapping &locoB)
{
    // TODO: prefer pushing/pulling, more than 2 locos

    TrainSpeedTable table;
    table.mLocoA_Address = locoA.address();
    table.mLocoB_Address = locoB.address();

    // Build a table with A speed matching a B speed
    int stepB = 1;
    for(int stepA = 1; stepA < 126; stepA++)
    {
        double speedA = locoA.getSpeedForStep(stepA);
        if(qFuzzyIsNull(speedA))
            continue;

        Entry e;
        e.locoA.step = stepA;
        e.locoA.speed = speedA;

        bool inserted = false;
        for(; stepB < 126; stepB++)
        {
            double speedB = locoB.getSpeedForStep(stepB);
            if(qFuzzyIsNull(speedB))
                continue;

            e.locoB.step = stepB;
            e.locoB.speed = speedB;

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

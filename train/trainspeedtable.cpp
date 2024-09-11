#include "trainspeedtable.h"

#include "locospeedmapping.h"

#include <QDebug>

#include <QFile>
#include <QTextStream>

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

void getTableFromLocoArray(const QVector<LocoSpeedMapping>& locoMappings)
{
    const int NUM_LOCOS = locoMappings.size();
    const int LAST_LOCO = NUM_LOCOS - 1;

    if(NUM_LOCOS < 2)
        return; // Error?

    double maxTrainSpeed = locoMappings.first().getSpeedForStep(126);
    for(int locoIdx = 1; locoIdx < NUM_LOCOS; locoIdx++)
    {
        double maxSpeed = locoMappings.at(locoIdx).getSpeedForStep(126);
        if(maxSpeed < maxTrainSpeed)
            maxTrainSpeed = maxSpeed;
    }

    struct LocoStepCache
    {
        int maxStep = 0;
        int currentStep = 0;
        int minAcceptedStep = 0;
        int maxAcceptedStep = 0;

        double minSpeedSoFar = 0;
        double maxSpeedSoFar = 0;
        double currentSpeed = 0;
    };

    struct SpeedEntry
    {
        double maxDiff;
        double avgSpeed;
        QVector<int> stepForLoco;
    };

    const double MAX_SPEED_DIFF = 0.005;
    maxTrainSpeed += MAX_SPEED_DIFF;

    QVector<LocoStepCache> stepCache;
    stepCache.reserve(NUM_LOCOS);

    QVector<SpeedEntry> entries;

    for(int locoIdx = 0; locoIdx < NUM_LOCOS; locoIdx++)
    {
        LocoStepCache item;
        item.maxStep = locoMappings.at(locoIdx).stepLowerBound(maxTrainSpeed);
        if(item.maxStep == 0)
            item.maxStep = 126;
        stepCache.append(item);
    }

    LocoStepCache& firstLocoRef = stepCache.first();
    int currentLocoIdx = 0;

    bool beginNewRound = true;
    bool canCompareToLastInserted = false;
    double minAcceptedSpeed = 0;
    double maxAcceptedSpeed = 0;

    while(firstLocoRef.currentStep <= firstLocoRef.maxStep)
    {
        const LocoSpeedMapping& mapping = locoMappings.at(currentLocoIdx);
        LocoStepCache& item = stepCache[currentLocoIdx];

        if(currentLocoIdx == 0)
        {
            item.currentStep++;
            item.currentSpeed = mapping.getSpeedForStep(item.currentStep);
            minAcceptedSpeed = item.currentSpeed - MAX_SPEED_DIFF;
            maxAcceptedSpeed = item.currentSpeed + MAX_SPEED_DIFF;
            item.minSpeedSoFar = item.maxSpeedSoFar = item.currentSpeed;

            for(int otherLocoIdx = 1; otherLocoIdx < locoMappings.size(); otherLocoIdx++)
            {
                const LocoSpeedMapping& otherMapping = locoMappings.at(otherLocoIdx);
                LocoStepCache& otherItem = stepCache[otherLocoIdx];

                otherItem.minAcceptedStep = otherMapping.stepLowerBound(minAcceptedSpeed);
                otherItem.maxAcceptedStep = otherMapping.stepUpperBound(maxAcceptedSpeed);
                if(otherItem.minAcceptedStep == 0)
                    otherItem.minAcceptedStep = 126;
                if(otherItem.maxAcceptedStep == 0)
                    otherItem.maxAcceptedStep = 126;
            }

            // Go to next loco
            currentLocoIdx++;
            beginNewRound = true;

            // First locomotive step changed, so we do not compare
            // new entries with old ones. We leave duplication removal for later
            canCompareToLastInserted = false; // First loco step changed
            continue;
        }

        const LocoStepCache& prevItem = stepCache.at(currentLocoIdx - 1);

        if(beginNewRound)
        {
            item.currentStep = item.minAcceptedStep;
            beginNewRound = false;
        }
        else
        {
            item.currentStep++;
        }

        if(item.currentStep > item.maxAcceptedStep)
        {
            // Go up to previous loco
            currentLocoIdx--;
            continue;
        }

        item.minSpeedSoFar = prevItem.minSpeedSoFar;
        item.maxSpeedSoFar = prevItem.maxSpeedSoFar;
        item.currentSpeed = mapping.getSpeedForStep(item.currentStep);
        if(item.currentSpeed < item.minSpeedSoFar)
            item.minSpeedSoFar = item.currentSpeed;
        if(item.currentSpeed > item.maxSpeedSoFar)
            item.maxSpeedSoFar = item.currentSpeed;

        const double maxDiff = (item.maxSpeedSoFar - item.minSpeedSoFar);
        if(maxDiff > MAX_SPEED_DIFF)
        {
            // Go to next step
            continue;
        }

        if(currentLocoIdx < LAST_LOCO)
        {
            // Go to next loco
            currentLocoIdx++;
            beginNewRound = true;
            continue;
        }

        // We are last loco, save speed tuple
        SpeedEntry entry;
        entry.stepForLoco.resize(NUM_LOCOS);

        double speedSum = 0;
        for(int locoIdx = 0; locoIdx < NUM_LOCOS; locoIdx++)
        {
            const LocoStepCache& otherItem = stepCache.at(locoIdx);

            entry.stepForLoco[locoIdx] = otherItem.currentStep;
            speedSum += otherItem.currentSpeed;
        }

        entry.maxDiff = maxDiff;
        entry.avgSpeed = speedSum / double(NUM_LOCOS);

        if(canCompareToLastInserted)
        {
            if(entries.last().maxDiff > entry.maxDiff)
            {
                // We are better than previous match, replace it
                entries.last() = entry;
            }

            // If not better than previous, no point in addint it
            continue;
        }

        // First good match of new step combination
        entries.append(entry);
        canCompareToLastInserted = true;
    }

    if(entries.empty())
        return; // Error?

    // Remove duplicated steps, keep match with lower maxDiff
    for(int locoIdx = 0; locoIdx < NUM_LOCOS; locoIdx++)
    {
        const SpeedEntry& firstEntry = entries.first();
        int currentStep = firstEntry.stepForLoco.at(locoIdx);
        int firstTableIdx = 0;
        int bestEntryIdx = 0;
        double bestEntryDiff = firstEntry.maxDiff;

        for(int tableIdx = 1; tableIdx < entries.size(); tableIdx++)
        {
            SpeedEntry entry = entries.at(tableIdx);
            if(entry.stepForLoco.at(locoIdx) == currentStep)
            {
                if(entry.maxDiff < bestEntryDiff)
                {
                    bestEntryIdx = tableIdx;
                    bestEntryDiff = entry.maxDiff;
                }
                continue;
            }

            // We reached next step
            // Cut all previous step entries, keep only best
            for(int i = firstTableIdx; i < bestEntryIdx; i++)
            {
                entries.removeAt(firstTableIdx); //TODO: optimize with ranges
            }

            // Shift all indexes
            int idxShift = bestEntryIdx - firstTableIdx;
            bestEntryIdx = firstTableIdx;
            tableIdx -= idxShift;

            for(int i = bestEntryIdx + 1; i < tableIdx; i++)
            {
                entries.removeAt(bestEntryIdx + 1); //TODO: optimize with ranges
            }

            // Now best entry is at first index, we are just after it
            tableIdx = bestEntryIdx + 1;

            // Re-init
            firstTableIdx = tableIdx;
            bestEntryIdx = firstTableIdx;
            currentStep = entry.stepForLoco.at(locoIdx);
            bestEntryDiff = entry.maxDiff;
        }

        if(firstTableIdx < (entries.size() - 1))
        {
            // Cut all last step entries, keep only best
            for(int i = firstTableIdx; i < bestEntryIdx; i++)
            {
                entries.removeAt(firstTableIdx); //TODO: optimize with ranges
            }

            // Shift all indexes
            bestEntryIdx = firstTableIdx;
            int oldSize = entries.size();

            for(int i = bestEntryIdx + 1; i < oldSize; i++)
            {
                entries.removeAt(bestEntryIdx + 1); //TODO: optimize with ranges
            }
        }
    }

    qDebug() << entries.size();

    QFile f("/home/filippo/Documenti/SPEED_MAPPINGS/Nuovi_SPEED_MAPPINGS/test_triple_2.txt");
    f.open(QFile::WriteOnly);

    QTextStream txt(&f);

    txt << "TEST: nLoco: " << NUM_LOCOS << " nEntries: " << entries.size() << Qt::endl;
    txt << "Nomi:" << Qt::endl;

    for(int locoIdx = 0; locoIdx < NUM_LOCOS; locoIdx++)
    {
        txt << locoIdx << " " << locoMappings.at(locoIdx).name() << Qt::endl;
    }

    txt << "TABLE:" << Qt::endl;

    txt << qSetRealNumberPrecision(5) << Qt::fixed;

    for(int tableIdx = 1; tableIdx < entries.size(); tableIdx++)
    {
        const SpeedEntry& entry = entries.at(tableIdx);

        txt << qSetFieldWidth(3) << tableIdx << "\t";
        txt << qSetFieldWidth(10) << entry.avgSpeed << "\t" << entry.maxDiff << "\t";

        txt << qSetFieldWidth(3);

        for(int locoIdx = 0; locoIdx < NUM_LOCOS; locoIdx++)
        {
            if(locoIdx > 0) txt << "\t";
            txt << entry.stepForLoco.at(locoIdx);
        }

        txt << Qt::endl;
    }

    txt.flush();
    f.flush();
    f.close();
}

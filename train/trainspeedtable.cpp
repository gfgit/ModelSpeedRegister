#include "trainspeedtable.h"

#include "locospeedmapping.h"

TrainSpeedTable::Entry TrainSpeedTable::nullEntry = TrainSpeedTable::Entry();

TrainSpeedTable::TrainSpeedTable()
{

}

TrainSpeedTable::ClosestMatchRet TrainSpeedTable::getClosestMatch(int locoIdx, int step) const
{
    if(step == 0)
    {
        return {NULL_TABLE_ENTRY, nullEntry};
    }

    if(locoIdx < 0)
        return {NULL_TABLE_ENTRY, nullEntry}; // Error

    for(int i = 0; i < mEntries.size(); i++)
    {
        const Entry &e = mEntries.at(i);
        const int candidateStep = e.getStepForLoco(locoIdx);
        if(candidateStep == step)
            return {i, e}; // Exact match

        if(candidateStep > step)
        {
            // We got closest higher step
            // Check if lower step is even closer
            if(i > 0)
            {
                const Entry &prev = mEntries.at(i - 1);
                if((step - prev.getStepForLoco(locoIdx)) < (candidateStep - step))
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
        return {mEntries.size() - 1, *mEntries.rbegin()};

    return {NULL_TABLE_ENTRY, nullEntry}; // Empty table
}

TrainSpeedTable::ClosestMatchRet TrainSpeedTable::getClosestMatch(double speed) const
{
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
            return {NULL_TABLE_ENTRY, nullEntry};
        }
    }

    // Return highest match if available
    if(!mEntries.empty())
        return {mEntries.size() - 1, *mEntries.rbegin()};

    return {NULL_TABLE_ENTRY, nullEntry}; // Empty table
}

const TrainSpeedTable::Entry& TrainSpeedTable::getEntryAt(int idx) const
{
    // NULL_TABLE_ENTRY returns null Entry as expected!
    if(idx == NULL_TABLE_ENTRY)
        return nullEntry;
    return mEntries.at(idx);
}

TrainSpeedTable TrainSpeedTable::buildTable(const std::vector<LocoSpeedMapping> &locoMappings)
{
    // TODO: prefer pushing/pulling, more than 2 locos

    TrainSpeedTable table;

    const int NUM_LOCOS = locoMappings.size();
    const int LAST_LOCO = NUM_LOCOS - 1;
    table.locoCount = NUM_LOCOS;

    if(NUM_LOCOS < 2)
        return table; // Error?

    double maxTrainSpeed = locoMappings.at(0).getSpeedForStep(126);
    for(int locoIdx = 1; locoIdx < NUM_LOCOS; locoIdx++)
    {
        double maxSpeed = locoMappings.at(locoIdx).getSpeedForStep(126);
        if(maxSpeed < maxTrainSpeed)
            maxTrainSpeed = maxSpeed;
    }

    struct LocoStepCache
    {
        int currentStep = 0;
        int minAcceptedStep = 0;
        int maxAcceptedStep = 0;

        double minSpeedSoFar = 0;
        double maxSpeedSoFar = 0;
        double currentSpeed = 0;
    };

    const double MAX_SPEED_DIFF = 0.005;
    maxTrainSpeed += MAX_SPEED_DIFF;

    std::vector<LocoStepCache> stepCache;
    stepCache.resize(NUM_LOCOS, LocoStepCache());

    // These 2 vector are in sync.
    // Diff vector will be discarded in the end
    // Only entries are stored in the final table
    std::vector<Entry>& entries = table.mEntries;
    std::vector<double> diffVector;

    entries.reserve(200);
    diffVector.reserve(200);

    int firstLocoMaxStep = locoMappings.at(0).stepLowerBound(maxTrainSpeed);
    if(firstLocoMaxStep == 0)
        firstLocoMaxStep = 126;

    int currentLocoIdx = 0;

    bool beginNewRound = true;
    bool canCompareToLastInserted = false;

    while(stepCache[0].currentStep <= firstLocoMaxStep)
    {
        const LocoSpeedMapping& mapping = locoMappings.at(currentLocoIdx);
        LocoStepCache& item = stepCache[currentLocoIdx];

        if(currentLocoIdx == 0)
        {
            item.currentStep++;
            item.currentSpeed = mapping.getSpeedForStep(item.currentStep);

            const double minAcceptedSpeed = item.currentSpeed - MAX_SPEED_DIFF;
            const double maxAcceptedSpeed = item.currentSpeed + MAX_SPEED_DIFF;
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
        Entry entry;
        entry.stepForLoco_.reset(new uint8_t[NUM_LOCOS]);

        double speedSum = 0;
        for(int locoIdx = 0; locoIdx < NUM_LOCOS; locoIdx++)
        {
            const LocoStepCache& otherItem = stepCache.at(locoIdx);

            entry.stepForLoco_[locoIdx] = otherItem.currentStep;
            speedSum += otherItem.currentSpeed;
        }

        entry.avgSpeed = speedSum / double(NUM_LOCOS);

        if(canCompareToLastInserted)
        {
            if(*diffVector.rbegin() > maxDiff)
            {
                // We are better than previous match, replace it

                std::memcpy(entries.rbegin()->stepForLoco_.get(),
                            entry.stepForLoco_.get(),
                            NUM_LOCOS * sizeof(uint8_t));
                entries.rbegin()->avgSpeed = entry.avgSpeed;

                *diffVector.rbegin() = maxDiff;
            }

            // If not better than previous, no point in addint it
            continue;
        }

        // First good match of new step combination
        entries.push_back(std::move(entry));
        diffVector.push_back(maxDiff);
        canCompareToLastInserted = true;
    }

    if(entries.empty())
        return table; // Error?

    // Remove duplicated steps, keep match with lower maxDiff
    for(int locoIdx = 0; locoIdx < NUM_LOCOS; locoIdx++)
    {
        const Entry& firstEntry = entries.at(0);
        double bestEntryDiff = diffVector.at(0);
        int bestEntryIdx = 0;
        int firstTableIdx = 0;
        int currentStep = firstEntry.stepForLoco_[locoIdx];

        for(int tableIdx = 1; tableIdx < entries.size(); tableIdx++)
        {
            int step = entries.at(tableIdx).stepForLoco_[locoIdx];
            if(step == currentStep)
            {
                const double maxDiff = diffVector.at(tableIdx);
                if(maxDiff < bestEntryDiff)
                {
                    bestEntryIdx = tableIdx;
                    bestEntryDiff = maxDiff;
                }
                continue;
            }

            // We reached next step
            // Cut all previous step entries, keep only best
            if(firstTableIdx < bestEntryIdx)
            {
                // Best entry is not erased
                entries.erase(entries.begin() + firstTableIdx,
                              entries.begin() + bestEntryIdx);
                diffVector.erase(diffVector.begin() + firstTableIdx,
                              diffVector.begin() + bestEntryIdx);
            }

            // Shift all indexes
            int idxShift = bestEntryIdx - firstTableIdx;
            bestEntryIdx = firstTableIdx;
            tableIdx -= idxShift;

            // Remove all after best entry up to last of this step
            int firstToErase = bestEntryIdx + 1;
            if(firstToErase < tableIdx)
            {
                // Best entry is not erased
                entries.erase(entries.begin() + firstToErase,
                              entries.begin() + tableIdx);
                diffVector.erase(diffVector.begin() + firstToErase,
                              diffVector.begin() + tableIdx);
            }

            // Now best entry is at first index, we are just after it
            tableIdx = bestEntryIdx + 1;

            // Re-init
            firstTableIdx = tableIdx;
            bestEntryIdx = firstTableIdx;
            currentStep = step;
            bestEntryDiff = diffVector.at(tableIdx);
        }

        if(firstTableIdx < (entries.size() - 1))
        {
            // Cut all last step entries, keep only best
            if(firstTableIdx < bestEntryIdx)
            {
                // Best entry is not erased
                entries.erase(entries.begin() + firstTableIdx,
                              entries.begin() + bestEntryIdx);
                diffVector.erase(diffVector.begin() + firstTableIdx,
                              diffVector.begin() + bestEntryIdx);
            }

            // Shift all indexes
            bestEntryIdx = firstTableIdx;

            // Remove all after best entry
            int firstToErase = bestEntryIdx + 1;
            entries.erase(entries.begin() + firstToErase,
                          entries.end());
            diffVector.erase(diffVector.begin() + firstToErase,
                          diffVector.end());
        }
    }

    // Save up some memory
    table.mEntries.shrink_to_fit();

    return table;
}

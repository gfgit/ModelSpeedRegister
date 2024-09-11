#ifndef TRAINSPEEDTABLE_H
#define TRAINSPEEDTABLE_H

#include <memory>
#include <vector>

class LocoSpeedMapping;

class TrainSpeedTable
{
public:
    enum
    {
        NULL_TABLE_ENTRY = -1
    };

    struct Entry
    {
        Entry() = default;

        // Disable copy from lvalue.
        Entry(const Entry&) = delete;
        Entry& operator=(const Entry&) = delete;

        // Move contructor
        // NOTE: needed to store std::unique_ptr in QVector
        Entry(Entry &&other)
        {
            stepForLoco_ = std::move(other.stepForLoco_);
            avgSpeed = std::move(other.avgSpeed);
        }

        Entry&
        operator=(Entry&& other)
        {
            stepForLoco_ = std::move(other.stepForLoco_);
            avgSpeed = std::move(other.avgSpeed);
            return *this;
        }

        std::unique_ptr<uint8_t[]> stepForLoco_;
        double avgSpeed = 0;

        inline uint8_t getStepForLoco(int idx) const
        {
            if(stepForLoco_)
                return stepForLoco_[idx];
            return 0;
        }
    };

    TrainSpeedTable();

    typedef std::pair<int, const Entry& > ClosestMatchRet;

    ClosestMatchRet getClosestMatch(int locoIdx, int step) const;

    ClosestMatchRet getClosestMatch(double speed) const;

    inline int count() const { return mEntries.size(); }
    const Entry& getEntryAt(int idx) const;

    static TrainSpeedTable buildTable(const std::vector<LocoSpeedMapping>& locoMappings);

private:
    std::vector<Entry> mEntries;
    int locoCount = 0;

    static Entry nullEntry;
};

#endif // TRAINSPEEDTABLE_H

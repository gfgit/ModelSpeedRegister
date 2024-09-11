#include "locospeedmapping.h"

LocoSpeedMapping::LocoSpeedMapping(const QString& name_, int address_, const std::array<double, 126>& arr)
    : mName(name_)
    , mAddress(address_)
    , mSpeed(arr)
{

}

double LocoSpeedMapping::getSpeedForStep(int step) const
{
    if(step <= 0 || step > 126)
        return 0;
    return mSpeed.at(step - 1); // We do not store zero so index is step - 1
}

int LocoSpeedMapping::stepUpperBound(double speed) const
{
    auto it = std::upper_bound(mSpeed.begin(), mSpeed.end(), speed);
    if(it != mSpeed.end())
    {
        int idx = std::distance(mSpeed.begin(), it);
        int step = idx + 1; // We do not store zero so step is index + 1
        return step;
    }
    return 0;
}

int LocoSpeedMapping::stepLowerBound(double speed) const
{
    auto it = std::lower_bound(mSpeed.begin(), mSpeed.end(), speed);
    if(it != mSpeed.end())
    {
        int idx = std::distance(mSpeed.begin(), it);
        int step = idx + 1; // We do not store zero so step is index + 1
        return step;
    }
    return 0;
}

int LocoSpeedMapping::address() const
{
    return mAddress;
}

QString LocoSpeedMapping::name() const
{
    return mName;
}

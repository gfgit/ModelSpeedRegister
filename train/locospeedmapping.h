#ifndef LOCOSPEEDMAPPING_H
#define LOCOSPEEDMAPPING_H

#include <QString>
#include <array>

class LocoSpeedMapping
{
public:
    LocoSpeedMapping(const QString& name_ = QString(), int address_ = 0, const std::array<double, 126>& arr = {});

    double getSpeedForStep(int step) const;

    int stepUpperBound(double speed) const;
    int stepLowerBound(double speed) const;

    int address() const;

    QString name() const;

private:
    QString mName;
    int mAddress = 0;

    std::array<double, 126> mSpeed;
};

#endif // LOCOSPEEDMAPPING_H

#ifndef LOCOINFO_H
#define LOCOINFO_H

#include <QString>
#include <array>

class LocoInfo
{
public:
    LocoInfo();

    QString name;
    int dccAddress;

    QString fileName;

    std::array<double, 126> forwardSpeedTable;
    std::array<double, 126> reverseSpeedTable;

    double accelerationRate;
    double decelerationRate;
};

#endif // LOCOINFO_H

#ifndef RAWSPEEDCURVEIO_H
#define RAWSPEEDCURVEIO_H

class QLineSeries;
class QString;

class RawSpeedCurveIO
{
public:
    static bool readCurveFromFile(const QString& fileName, QLineSeries *series);
    static bool saveCurveToFile(const QString& fileName, QLineSeries *series);
};

#endif // RAWSPEEDCURVEIO_H

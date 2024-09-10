#include "rawspeedcurveio.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QLineSeries>

bool RawSpeedCurveIO::readCurveFromFile(const QString &fileName, QLineSeries *series)
{
    QFile f(fileName);
    if(!f.open(QFile::ReadOnly))
        return false;

    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());

    QJsonObject obj = doc.object();

    QString name = obj.value(QLatin1String("name")).toString();
    if(name.isEmpty())
        name = f.fileName();

    QJsonValue tmp = obj.value(QLatin1String("curve_array"));
    if(tmp.isArray())
    {
        QJsonArray curve = tmp.toArray();
        if(curve.size() != 127)
            return false;

        series->clear();

        int step = 0;
        for(auto val : curve)
        {
            series->append(step, val.toDouble());
            step++;
        }
    }
    else if(tmp = obj.value(QLatin1String("speed_mapping")); tmp.isArray())
    {
        series->clear();

        QJsonArray sparseTable = tmp.toArray();

        double lastSpeed = 0;
        int lastStep = 0;

        for(QJsonValue v : sparseTable)
        {
            QJsonObject stepObj = v.toObject();
            int step = stepObj.value(QLatin1String("step")).toInt();
            double speed = stepObj.value(QLatin1String("speed")).toDouble();

            if(step > lastStep + 1)
            {
                // Linear interpolation of steps inbetween
                int numSteps = step - lastStep;
                double increment = (speed - lastSpeed) / double(numSteps);

                for(int i = 1; i < numSteps; i++)
                {
                    double calculatedSpeed = lastSpeed + increment * double(i);
                    int calculatedStep = lastStep + i;
                    series->append(calculatedStep, calculatedSpeed);
                }
            }

            lastStep = step;
            lastSpeed = speed;

            series->append(step, speed);
        }
    }
    else
        return false;

    series->setName(name);

    return true;
}

bool RawSpeedCurveIO::saveCurveToFile(const QString &fileName, QLineSeries *series)
{
    QFile f(fileName);
    if(!f.open(QFile::WriteOnly))
        return false;

    QJsonObject obj;
    obj[QLatin1String("name")] = series->name();

    QJsonArray arr;
    for(int step = 0; step <= 126 && step < series->count(); step++)
    {
        arr.append(QJsonValue(series->at(step).y()));
    }

    obj[QLatin1String("curve_array")] = arr;

    QJsonDocument doc(obj);
    f.write(doc.toJson());

    return true;
}

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

    QJsonArray arr = obj.value(QLatin1String("curve_array")).toArray();
    if(arr.size() != 127)
        return false;

    series->setName(name);
    series->clear();

    int step = 0;
    for(auto val : arr)
    {
        series->append(step, val.toDouble());
        step++;
    }

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

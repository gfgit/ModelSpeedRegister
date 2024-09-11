#include "mainwindow.h"

#include <QApplication>

#include <QDebug>
#include "recorder/series/movingaverageseries.h"

class MockSeries : public IDataSeries
{
public:
    MockSeries(QObject *parent = nullptr) : IDataSeries(parent) {}

    DataSeriesType getType() const override { return DataSeriesType::Unknown; };
    int getPointCount() const override { return mPoints.size(); };
    QPointF getPointAt(int index) const override { return mPoints.value(index, QPointF()); };
    QString getPointTooltip(int index) const override { return QString(); };


    void addPoint(int index, const QPointF& p)
    {
        mPoints.insert(index, p);
        emit pointAdded(index, p);
    }

    void updatePoint(int index, const QPointF& p)
    {
        mPoints[index] = p;
        emit pointChanged(index, p);
    }

    void removePoint(int index)
    {
        mPoints.removeAt(index);
        emit pointRemoved(index);
    }

private:
    QVector<QPointF> mPoints;
};

void printSeries(IDataSeries *s)
{
    qDebug() << "Series:" << s->name() << "\n"
                "Count:" << s->getPointCount();

    for(int i = 0; i < s->getPointCount(); i++)
    {
        qDebug() << i << "val:" << s->getPointAt(i);
    }
}

int main(int argc, char *argv[])
{
    // MockSeries mock;

    // MovingAverageSeries avg(3);
    // avg.setSource(&mock);

    // mock.addPoint(0, {0, 1});

    // qDebug() << "ROUND 1";
    // printSeries(&mock);
    // qDebug() << "";
    // printSeries(&avg);
    // qDebug() << "\n";


    // mock.addPoint(1, {0, 1});
    // mock.addPoint(2, {0, 1});

    // qDebug() << "ROUND 2";
    // printSeries(&mock);
    // qDebug() << "";
    // printSeries(&avg);
    // qDebug() << "\n";


    // mock.updatePoint(1, {0, 4});

    // qDebug() << "ROUND 3";
    // printSeries(&mock);
    // qDebug() << "";
    // printSeries(&avg);
    // qDebug() << "\n";


    // mock.addPoint(1, {0, 4});

    // qDebug() << "ROUND 4";
    // printSeries(&mock);
    // qDebug() << "";
    // printSeries(&avg);
    // qDebug() << "\n";


    // mock.removePoint(1);

    // qDebug() << "ROUND 5";
    // printSeries(&mock);
    // qDebug() << "";
    // printSeries(&avg);
    // qDebug() << "\n";


    // mock.removePoint(1);

    // qDebug() << "ROUND 6";
    // printSeries(&mock);
    // qDebug() << "";
    // printSeries(&avg);
    // qDebug() << "\n";



    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}

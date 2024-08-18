#ifndef DUMMYSPEEDSENSOR_H
#define DUMMYSPEEDSENSOR_H

#include <QElapsedTimer>

#include "ispeedsensor.h"

class DummySpeedSensor : public ISpeedSensor
{
    Q_OBJECT
public:
    explicit DummySpeedSensor(QObject *parent = nullptr);
    ~DummySpeedSensor();

    void timerEvent(QTimerEvent *e) override;

    QVector<double> speedCurve() const;

public slots:
    void start();
    void stop();
    void simulateSpeedStep(int step);

private:
    void generateRandomSpeedCurve();

private:
    int mTimerId = 0;
    QVector<double> mSpeedCurve;
    int mCurrentStep = 0;

    QElapsedTimer mElapsedTimer;

    double travelledMillimeters = 0;
    double lastSpeedMetersPerSecond = 0;
    qint64 lastSpeedStart = 0;
};

#endif // DUMMYSPEEDSENSOR_H

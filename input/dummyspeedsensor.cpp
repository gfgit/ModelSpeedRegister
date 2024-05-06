#include "dummyspeedsensor.h"

#include <QTimerEvent>
#include <QRandomGenerator64>

DummySpeedSensor::DummySpeedSensor(QObject *parent)
    : ISpeedSensor{parent}
{

}

DummySpeedSensor::~DummySpeedSensor()
{
    stop();
}

void DummySpeedSensor::timerEvent(QTimerEvent *e)
{
    if(e->timerId() == mTimerId)
    {
        double currentSpeed = mSpeedCurve.at(mCurrentStep);
        const double MAX_OSCILLATION = 2.0;
        double noise = QRandomGenerator::global()->bounded(MAX_OSCILLATION) - MAX_OSCILLATION / 2;
        currentSpeed += noise;
        emit speedReading(currentSpeed, LocomotiveDirection::Forward, mElapsedTimer.elapsed());
        return;
    }

    ISpeedSensor::timerEvent(e);
}

void DummySpeedSensor::start()
{
    stop();

    generateRandomSpeedCurve();
    mTimerId = startTimer(1000);
    mElapsedTimer.start();
}

void DummySpeedSensor::stop()
{
    if(mTimerId)
    {
        killTimer(mTimerId);
        mTimerId = 0;
    }
}

void DummySpeedSensor::simulateSpeedStep(int step)
{
    mCurrentStep = qBound(0, step, 126);
}

void DummySpeedSensor::generateRandomSpeedCurve()
{
    mSpeedCurve.resize(127);
    mSpeedCurve[0] = 0;
    const double MAX_INCREMENT = 5.0;

    for(int i = 1; i < 127; i++)
    {
        double increment = QRandomGenerator::global()->bounded(MAX_INCREMENT);
        mSpeedCurve[i] = mSpeedCurve[i - 1] + increment;
    }
}

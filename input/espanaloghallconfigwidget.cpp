#include "espanaloghallconfigwidget.h"
#include "espanaloghallsensor.h"

#include <QVBoxLayout>
#include <QGridLayout>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>

ESPAnalogHallConfigWidget::ESPAnalogHallConfigWidget(QWidget *parent)
    : QWidget{parent}
{
    QVBoxLayout *lay = new QVBoxLayout(this);

    mConnectedCheck = new QCheckBox(tr("Connected"));
    lay->addWidget(mConnectedCheck);
    connect(mConnectedCheck, &QCheckBox::toggled, this, [this](bool val)
    {
        setMonitoring(false);
        if(mSensor)
            mSensor->setState(val);
    });

    mDebugOutputCheck = new QCheckBox(tr("Debug Output"));
    lay->addWidget(mDebugOutputCheck);
    connect(mDebugOutputCheck, &QCheckBox::toggled, this, [this](bool val)
    {
        if(mSensor)
            mSensor->setDebugOutput(val);
    });

    mMonitorBut = new QPushButton;
    lay->addWidget(mMonitorBut);
    setMonitoring(false);
    connect(mMonitorBut, &QPushButton::clicked, this, [this]()
    {
        setMonitoring(!mMonitoring); // Toggle
    });

    mResetTravelBut = new QPushButton(tr("Reset Travel"));
    lay->addWidget(mResetTravelBut);
    connect(mResetTravelBut, &QPushButton::clicked, this, [this]()
    {
        if(mSensor)
            mSensor->resetTravelledCount();
    });

    mSuggestThresholdBut = new QPushButton(tr("Calc thresholds"));
    lay->addWidget(mSuggestThresholdBut);
    connect(mSuggestThresholdBut, &QPushButton::clicked, this, [this]()
    {
        int delta = (mSensorMax - mSensorMin) / 8;
        mHighEntrance->setValue(mSensorMin + delta);
        mHighExit->setValue(mSensorMin + delta * 3);
        mLowEntrance->setValue(mSensorMin + delta * 7);
        mLowExit->setValue(mSensorMin + delta * 5);
    });

    mSetThresholdBut = new QPushButton(tr("Set thresholds"));
    lay->addWidget(mSetThresholdBut);
    connect(mSetThresholdBut, &QPushButton::clicked, this, [this]()
    {
        if(mSensor)
        {
            int high1 = mHighEntrance->value();
            int high2 = mHighExit->value();
            int low1 = mLowEntrance->value();
            int low2 = mLowExit->value();
            mSensor->setThresholds(high1, high2, low1, low2);
        }
    });

    mMinLabel = new QLabel;
    mMaxLabel = new QLabel;
    mHighEntrance = new QSpinBox;
    mHighExit = new QSpinBox;
    mLowEntrance = new QSpinBox;
    mLowExit = new QSpinBox;

    mHighEntrance->setRange(0, 1000);
    mHighExit->setRange(0, 1000);
    mLowEntrance->setRange(0, 1000);
    mLowExit->setRange(0, 1000);

    QGridLayout *gridLay = new QGridLayout;
    lay->addLayout(gridLay);

    gridLay->addWidget(mMinLabel, 0, 0);
    gridLay->addWidget(mHighEntrance, 0, 1);
    gridLay->addWidget(mHighExit, 0, 2);
    gridLay->addWidget(mMaxLabel, 1, 0);
    gridLay->addWidget(mLowEntrance, 1, 1);
    gridLay->addWidget(mLowExit, 1, 2);
}

ESPAnalogHallSensor *ESPAnalogHallConfigWidget::sensor() const
{
    return mSensor;
}

void ESPAnalogHallConfigWidget::setSensor(ESPAnalogHallSensor *newSensor)
{
    if(mSensor)
    {
        disconnect(mSensor, &ESPAnalogHallSensor::destroyed,
                   this, &ESPAnalogHallConfigWidget::onSensorDestroyed);
        disconnect(mSensor, &ESPAnalogHallSensor::sensorMinMaxChanged,
                   this, &ESPAnalogHallConfigWidget::onSensorMinMaxChanged);
    }

    mSensor = newSensor;

    if(mSensor)
    {
        connect(mSensor, &ESPAnalogHallSensor::destroyed,
                this, &ESPAnalogHallConfigWidget::onSensorDestroyed);
        connect(mSensor, &ESPAnalogHallSensor::sensorMinMaxChanged,
                this, &ESPAnalogHallConfigWidget::onSensorMinMaxChanged);
    }

    setMonitoring(false);
}

void ESPAnalogHallConfigWidget::onSensorDestroyed()
{
    setSensor(nullptr);
}

void ESPAnalogHallConfigWidget::onSensorMinMaxChanged(int sensorMin, int sensorMax)
{
    if(!mMonitoring)
        return;

    mSensorMin = sensorMin;
    mSensorMax = sensorMax;

    mMinLabel->setText(QString::number(mSensorMin));
    mMaxLabel->setText(QString::number(mSensorMax));
}

void ESPAnalogHallConfigWidget::setMonitoring(bool val)
{
    mMonitoring = val;
    mMonitorBut->setText(mMonitoring ? tr("End Monitoring") : tr("Start Monitoring"));

    if(mSensor)
    {
        if(mMonitoring)
        {
            mSensor->resetSensorMinMax();
            mSensor->setDebugOutput(true);
        }
        else if(!mDebugOutputCheck->isChecked())
            mSensor->setDebugOutput(false);
    }
}

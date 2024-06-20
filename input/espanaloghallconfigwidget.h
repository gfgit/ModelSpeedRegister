#ifndef ESPANALOGHALLCONFIGWIDGET_H
#define ESPANALOGHALLCONFIGWIDGET_H

#include <QWidget>

class ESPAnalogHallSensor;

class QPushButton;
class QCheckBox;
class QLabel;
class QSpinBox;

class ESPAnalogHallConfigWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ESPAnalogHallConfigWidget(QWidget *parent = nullptr);

    ESPAnalogHallSensor *sensor() const;
    void setSensor(ESPAnalogHallSensor *newSensor);

private slots:
    void onSensorDestroyed();
    void onSensorMinMaxChanged(int sensorMin, int sensorMax);

    void setMonitoring(bool val);

private:
    ESPAnalogHallSensor *mSensor = nullptr;

    int mSensorMin = -1;
    int mSensorMax = -1;
    bool mMonitoring = false;

    QCheckBox *mConnectedCheck;
    QCheckBox *mDebugOutputCheck;
    QPushButton *mMonitorBut;
    QPushButton *mResetTravelBut;
    QPushButton *mSuggestThresholdBut;
    QPushButton *mSetThresholdBut;
    QLabel *mMinLabel;
    QLabel *mMaxLabel;

    QSpinBox *mHighEntrance;
    QSpinBox *mHighExit;
    QSpinBox *mLowEntrance;
    QSpinBox *mLowExit;
};

#endif // ESPANALOGHALLCONFIGWIDGET_H

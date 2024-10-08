#include "locostatuswidget.h"

#include "locomotive.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QSpinBox>

LocoStatusWidget::LocoStatusWidget(QWidget *parent)
    : QWidget{parent}
{
    QVBoxLayout *lay = new QVBoxLayout(this);

    mLabel = new QLabel;
    lay->addWidget(mLabel);

    mSpinBox = new QSpinBox;
    mSpinBox->setRange(0, 126);
    lay->addWidget(mSpinBox);

    connect(mSpinBox, &QSpinBox::valueChanged, this,
            [this](int v)
    {
        if(!mLocomotive)
            return;

        mLocomotive->driveLoco(v, mLocomotive->direction());
    });

    QFont f = font();
    f.setPointSize(20);
    mLabel->setFont(f);
    mSpinBox->setFont(f);

    // Refresh text
    updateStatus();
    updateGeometry();
}

void LocoStatusWidget::setLocomotive(Locomotive *newLocomotive)
{
    if(mLocomotive)
        disconnect(mLocomotive, &Locomotive::changed, this, &LocoStatusWidget::updateStatus);

    mLocomotive = newLocomotive;

    if(mLocomotive)
        connect(mLocomotive, &Locomotive::changed, this, &LocoStatusWidget::updateStatus);

    // Refresh text
    updateStatus();
}

void LocoStatusWidget::updateStatus()
{
    QString name;
    int speedStep = 0;
    LocomotiveDirection direction = LocomotiveDirection::Forward;
    int targetSpeedStep = 0;
    LocomotiveDirection targetDirection = LocomotiveDirection::Forward;
    double metersPerSecond = 0.0;

    if(mLocomotive)
    {
        auto& speedMapping = mLocomotive->speedMapping();
        name = speedMapping.name();
        speedStep = mLocomotive->speedStep();
        direction = mLocomotive->direction();
        targetSpeedStep = mLocomotive->targetSpeedStep();
        targetDirection = mLocomotive->targetDirection();
        metersPerSecond = speedMapping.getSpeedForStep(speedStep);
    }

    const double realKmH = metersPerSecond * 87.0 * 3.6;

    mSpinBox->blockSignals(true);
    mSpinBox->setValue(speedStep);
    mSpinBox->blockSignals(false);

    QString speedStepStr;
    if(speedStep == EMERGENCY_STOP)
        speedStepStr = tr("Stop");
    else
        speedStepStr = QString::number(speedStep).rightJustified(4);

    QString targetSpeedStepStr;
    if(targetSpeedStep == EMERGENCY_STOP)
        targetSpeedStepStr = tr("Stop");
    else
        targetSpeedStepStr = QString::number(targetSpeedStep).rightJustified(4);

    mLabel->setText(QLatin1String("%1\n"
                          "%2 %3 - %4 %5\n"
                          "%6 m/s\n"
                          "%7 km/h")
                .arg(name)
                .arg(speedStepStr).arg(direction == LocomotiveDirection::Forward ? 'F' : 'R')
                .arg(targetSpeedStepStr).arg(targetDirection == LocomotiveDirection::Forward ? 'F' : 'R')
                .arg(metersPerSecond, 0, 'f', 4)
                .arg(realKmH, 0, 'f', 2));
}

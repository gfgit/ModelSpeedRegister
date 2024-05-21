#include "locostatuswidget.h"

#include "locomotive.h"

LocoStatusWidget::LocoStatusWidget(QWidget *parent)
    : QLabel{parent}
{
    QFont f = font();
    f.setPointSize(20);
    setFont(f);

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

    setText(QLatin1String("%1\n"
                          "%2 %3 - %4 %5\n"
                          "%6 m/s\n"
                          "%7 km/h")
                .arg(name)
                .arg(speedStep).arg(direction == LocomotiveDirection::Forward ? 'F' : 'R')
                .arg(targetSpeedStep).arg(targetDirection == LocomotiveDirection::Forward ? 'F' : 'R')
                .arg(metersPerSecond, 0, 'f', 2)
                .arg(realKmH, 0, 'f', 2));
}

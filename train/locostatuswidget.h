#ifndef LOCOSTATUSWIDGET_H
#define LOCOSTATUSWIDGET_H

#include <QWidget>

#include "locospeedmapping.h"

class Locomotive;

class QLabel;
class QSpinBox;

class LocoStatusWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LocoStatusWidget(QWidget *parent = nullptr);

    void setLocomotive(Locomotive *newLocomotive);

private slots:
    void updateStatus();

private:
    Locomotive *mLocomotive = nullptr;

    QLabel *mLabel;
    QSpinBox *mSpinBox;
};

#endif // LOCOSTATUSWIDGET_H

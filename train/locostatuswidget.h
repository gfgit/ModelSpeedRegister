#ifndef LOCOSTATUSWIDGET_H
#define LOCOSTATUSWIDGET_H

#include "locospeedmapping.h"
#include <QLabel>

class Locomotive;

class LocoStatusWidget : public QLabel
{
    Q_OBJECT
public:
    explicit LocoStatusWidget(QWidget *parent = nullptr);

    void setLocomotive(Locomotive *newLocomotive);

private slots:
    void updateStatus();

private:
    Locomotive *mLocomotive = nullptr;
};

#endif // LOCOSTATUSWIDGET_H

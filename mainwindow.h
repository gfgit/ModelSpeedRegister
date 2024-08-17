#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class RecordingManager;
class LocomotiveRecordingView;

class LocoSpeedCurve;
class LocoSpeedCurveView;

class DummySpeedSensor;
class ESPAnalogHallSensor;
class ICommandStation;

class QTabWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    LocomotiveRecordingView *mRecView;
    RecordingManager *mRecManager;

    LocoSpeedCurve *mSpeedCurve;
    LocoSpeedCurveView *mSpeedCurveView;

    // ESPAnalogHallSensor *mSpeedSensor;
    DummySpeedSensor *mSpeedSensor;
    ICommandStation *mCommandStation;

    QTabWidget *mTabWidget;
};
#endif // MAINWINDOW_H

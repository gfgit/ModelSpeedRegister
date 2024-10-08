#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class RecordingManager;
class LocomotiveRecordingView;

class LocoSpeedCurveView;

class DummySpeedSensor;
class ESPAnalogHallSensor;
class ICommandStation;

class QTabWidget;

class QLabel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void startTest();
    void onRecMgrStateChanged(int newState);

private:
    Ui::MainWindow *ui;

    LocomotiveRecordingView *mRecView;
    RecordingManager *mRecManager;

    LocoSpeedCurveView *mSpeedCurveView;

    ESPAnalogHallSensor *mSpeedSensor;
    // DummySpeedSensor *mSpeedSensor;
    ICommandStation *mCommandStation;

    QTabWidget *mTabWidget;

    QLabel *testStatusLabel;
};
#endif // MAINWINDOW_H

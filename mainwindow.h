#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class RecordingManager;
class LocomotiveRecordingView;

class DummySpeedSensor;
class DummyCommandStation;

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

    DummySpeedSensor *mSpeedSensor;
    DummyCommandStation *mCommandStation;
};
#endif // MAINWINDOW_H

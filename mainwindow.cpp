#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "recorder/recordingmanager.h"
#include "view/locomotiverecordingview.h"

#include "recorder/locospeedcurve.h"
#include "view/locospeedcurveview.h"

#include "input/dummyspeedsensor.h"
#include "commandstation/dummycommandstation.h"
#include "commandstation/backends/z21commandstation.h"

#include <QHBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    mRecView = new LocomotiveRecordingView;
    mSpeedCurveView = new LocoSpeedCurveView;

    QWidget *w = new QWidget;

    // Grap gesture for zoom
    w->grabGesture(Qt::PanGesture);
    w->grabGesture(Qt::PinchGesture);

    QHBoxLayout *lay = new QHBoxLayout(w);
    lay->addWidget(mRecView);
    lay->addWidget(mSpeedCurveView);
    setCentralWidget(w);

    mRecManager = new RecordingManager(this);
    mSpeedCurve = new LocoSpeedCurve(this);
    mSpeedCurve->setRecording(mRecManager->currentRecording());

    mSpeedSensor = new DummySpeedSensor(this);
    mCommandStation = new DummyCommandStation;
    //mCommandStation = new Z21CommandStation(this);

    connect(mCommandStation, &DummyCommandStation::locomotiveSpeedFeedback, mSpeedSensor,
            [sensor = mSpeedSensor](int /*address*/, int speedStep)
            {
                sensor->simulateSpeedStep(speedStep);
            });

    mRecManager->setCommandStation(mCommandStation);
    mRecManager->setSpeedSensor(mSpeedSensor);

    mRecView->setRecording(mRecManager->currentRecording());
    mSpeedCurveView->setSpeedCurve(mSpeedCurve);

    connect(ui->actionStart, &QAction::triggered, this,
            [this]()
            {
                mSpeedSensor->start();
                mRecManager->start();
                mSpeedCurveView->setTargedSpeedCurve(mSpeedSensor->speedCurve());
            });

    connect(ui->actionStop, &QAction::triggered, this,
            [this]()
            {
                mSpeedSensor->stop();
                mRecManager->stop();
            });
}

MainWindow::~MainWindow()
{
    delete ui;
}


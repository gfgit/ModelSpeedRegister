#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "recorder/recordingmanager.h"
#include "view/locomotiverecordingview.h"

#include "input/dummyspeedsensor.h"
#include "commandstation/dummycommandstation.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    mRecView = new LocomotiveRecordingView;
    setCentralWidget(mRecView);

    mRecManager = new RecordingManager(this);

    mSpeedSensor = new DummySpeedSensor;
    mCommandStation = new DummyCommandStation;

    connect(mCommandStation, &DummyCommandStation::locomotiveSpeedFeedback, mSpeedSensor,
            [sensor = mSpeedSensor](int /*address*/, int speedStep)
            {
                sensor->simulateSpeedStep(speedStep);
            });

    mRecManager->setCommandStation(mCommandStation);
    mRecManager->setSpeedSensor(mSpeedSensor);

    mRecView->setRecording(mRecManager->currentRecording());

    connect(ui->actionStart, &QAction::triggered, this,
            [this]()
            {
                mSpeedSensor->start();
                mRecManager->start();
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


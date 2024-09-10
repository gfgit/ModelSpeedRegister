#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "recorder/recordingmanager.h"
#include "view/locomotiverecordingview.h"

#include "view/locospeedcurveview.h"
#include "view/starttestdlg.h"

#include "input/dummyspeedsensor.h"
#include "commandstation/dummycommandstation.h"
#include "input/espanaloghallsensor.h"
#include "commandstation/backends/z21commandstation.h"

#include "input/espanaloghallconfigwidget.h"

#include <QHBoxLayout>

#include <QTabWidget>
#include <QLabel>

#include <QPointer>
#include <QInputDialog>

#include "view/traintab.h"
#include "train/locomotivepool.h"

#include "recorder/series/movingaverageseries.h"
#include "recorder/series/rawsensordataseries.h"
#include "recorder/series/totalstepaverageseries.h"
#include "recorder/series/receivedspeedstepseries.h"
#include "recorder/series/requestedspeedstepseries.h"
#include "recorder/series/sensortravelleddistanceseries.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    testStatusLabel = new QLabel(this);
    statusBar()->addPermanentWidget(testStatusLabel);

    mSpeedSensor = new ESPAnalogHallSensor(this);
    // mSpeedSensor = new DummySpeedSensor(this);
    // mCommandStation = new DummyCommandStation;
    mCommandStation = new Z21CommandStation(this);

    LocomotivePool *mPool = new LocomotivePool(this);
    mPool->setCommandStation(mCommandStation);

    mRecView = new LocomotiveRecordingView;
    mSpeedCurveView = new LocoSpeedCurveView;

    mTabWidget = new QTabWidget;
    setCentralWidget(mTabWidget);

    QWidget *recordingPage = new QWidget;

    // Grap gesture for zoom
    recordingPage->grabGesture(Qt::PanGesture);
    recordingPage->grabGesture(Qt::PinchGesture);

    QHBoxLayout *recLay = new QHBoxLayout(recordingPage);
    recLay->addWidget(mRecView);
    recLay->addWidget(mSpeedCurveView);

    mTabWidget->addTab(recordingPage, tr("Recording"));

    TrainTab *trainTab = new TrainTab(mPool);
    mTabWidget->addTab(trainTab, tr("Train"));

    ESPAnalogHallConfigWidget *mConfig = new ESPAnalogHallConfigWidget;
    mConfig->setSensor(mSpeedSensor);
    mTabWidget->addTab(mConfig, tr("ESP Sensor"));

    mRecManager = new RecordingManager(this);

    // connect(mCommandStation, &DummyCommandStation::locomotiveSpeedFeedback, mSpeedSensor,
    //         [this](int /*address*/, int speedStep)
    //         {
    //             mSpeedSensor->simulateSpeedStep(speedStep);
    //         });

    mRecManager->setCommandStation(mCommandStation);
    mRecManager->setSpeedSensor(mSpeedSensor);

    mRecView->setRecMgr(mRecManager);
    mSpeedCurveView->setRecMgr(mRecManager);
    //mSpeedCurveView->setSpeedCurve(mSpeedCurve);

    // Sync label with intial state
    onRecMgrStateChanged(int(mRecManager->state()));

    connect(mRecManager, &RecordingManager::stateChanged,
            this, &MainWindow::onRecMgrStateChanged);

    connect(ui->actionStart, &QAction::triggered,
            this, &MainWindow::startTest);

    connect(ui->actionStop, &QAction::triggered, this,
            [this]()
            {
                mRecManager->stop();
            });

    connect(ui->actionNext_Step, &QAction::triggered, this,
            [this]()
            {
                mRecManager->goToNextStep();
            });

    connect(ui->actionOther_5_seconds, &QAction::triggered, this,
            [this]()
            {
                mRecManager->setCustomTimeForCurrentStep(5000);
            });

    connect(ui->actionEmergency_Stop, &QAction::triggered, this,
            [this]()
            {
                mRecManager->emergencyStop();
                //mSpeedSensor->stop();
            });

    connect(ui->actionAdd_Moving_Average, &QAction::triggered, this,
            [this]()
    {
        QString name = QInputDialog::getText(this, tr("Moving Average"), tr("Name:"));
        if(name.isEmpty())
            return;

        int windowSz = QInputDialog::getInt(this, tr("Moving Average"), tr("Window size:"), 0, 1, 100, 2);

        MovingAverageSeries *mv = new MovingAverageSeries(windowSz, mRecManager);
        mv->setName(name);
        mv->setSource(mRecManager->rawSensorSeries());
        mRecManager->registerSeries(mv);
    });

    connect(ui->actionAdd_Total_Average, &QAction::triggered, this,
            [this]()
    {
        QString name = QInputDialog::getText(this, tr("Total Step Average"), tr("Name:"));
        if(name.isEmpty())
            return;

        int accelMillis = QInputDialog::getInt(this, tr("Total Step Average"), tr("Acceleration Millis:"), 1000, 0, 10000, 100);

        TotalStepAverageSeries *s = new TotalStepAverageSeries(mRecManager);
        s->setName(name);
        s->setAccelerationMilliseconds(accelMillis);
        s->setTravelledSource(mRecManager->sensorTravelledSeries());
        s->setRecvStepSeries(mRecManager->recvStepSeries());
        s->setReqStepSeries(mRecManager->reqStepSeries());
        mRecManager->registerSeries(s);
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::startTest()
{
    QPointer<StartTestDlg> dlg = new StartTestDlg(this);
    dlg->setLocoAddress(mRecManager->getLocomotiveDCCAddress());
    dlg->setDefaultStepTime(mRecManager->defaultStepTimeMillis());
    dlg->setStartingDCCStep(mRecManager->startingDCCStep());

    if(dlg->exec() != QDialog::Accepted || !dlg)
        return;

    mRecManager->setLocomotiveDCCAddress(dlg->getLocoAddress());
    mRecManager->setDefaultStepTimeMillis(dlg->getDefaultStepTime());
    mRecManager->setStartingDCCStep(dlg->getStartingDCCStep());

    mSpeedSensor->resetTravelledCount();

    //mSpeedSensor->start();
    bool started = mRecManager->start();


    // if(started)
    // {
    //     mSpeedCurveView->setTargedSpeedCurve(mSpeedSensor->speedCurve());
    // }
    // else
    // {
    //     mSpeedSensor->stop();
    // }

    delete dlg;
}

void MainWindow::onRecMgrStateChanged(int /*newState*/)
{
    QString stateName;

    switch(mRecManager->state())
    {
    case RecordingManager::State::Stopped:
        stateName = tr("Test Stopped");
        break;
    case RecordingManager::State::Running:
        stateName = tr("Test RUNNING");
        break;
    case RecordingManager::State::WaitingToStop:
        stateName = tr("Stopping Test...");
        break;
    }

    testStatusLabel->setText(stateName);

    // if(mRecManager->state() == RecordingManager::State::Stopped)
    //     mSpeedSensor->stop();
}

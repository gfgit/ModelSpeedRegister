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
#include <QCheckBox>

#include <QPointer>
#include <QInputDialog>

#include "train/locostatuswidget.h"
#include "train/locomotive.h"
#include "train/locomotivepool.h"
#include "train/train.h"

#include "recorder/series/movingaverageseries.h"
#include "recorder/series/rawsensordataseries.h"
#include "recorder/series/totalstepaverageseries.h"
#include "recorder/series/receivedspeedstepseries.h"
#include "recorder/series/requestedspeedstepseries.h"
#include "recorder/series/sensortravelleddistanceseries.h"

LocoSpeedMapping mappingD753("D753",
                             47,
                             {
                                 0,       // 1
                                 0,
                                 0,
                                 0,
                                 0,
                                 0.0127, // 6 START
                                 0.0177,
                                 0.0228,
                                 0.0254,
                                 0.0279, // 10
                                 0.0305,
                                 0.0331,
                                 0.0369,
                                 0.0406,
                                 0.0432,
                                 0.0459,
                                 0.0497,
                                 0.0535,
                                 0.0560,
                                 0.0585, // 20
                                 0.0624,
                                 0.0662,
                                 0.0701,
                                 0.0740,
                                 0.0777,
                                 0.0814,
                                 0.0873,
                                 0.0932,
                                 0.0969,
                                 0.1006,
                                 0.1056,
                                 0.1105,
                                 0.1142,
                                 0.1178,
                                 0.1230,
                                 0.1281,
                                 0.1333,
                                 0.1386,
                                 0.1436,
                                 0.1487,
                                 0.1535,
                                 0.1583,
                                 0.1635,
                                 0.1686,
                                 0.1711,
                                 0.1737,
                                 0.1788,
                                 0.1840,
                                 0.1902,
                                 0.1964,
                                 0.1989,
                                 0.2013,
                                 0.2063,
                                 0.2113,
                                 0.2139,
                                 0.2165,
                                 0.2229,
                                 0.2292,
                                 0.2354,
                                 0.2416,
                                 0.2442,
                                 0.2468,
                                 0.2530,
                                 0.2591,
                                 0.2630,
                                 0.2670,
                                 0.2721,
                                 0.2772,
                                 0.2836,
                                 0.2900,
                                 0.2900,
                                 0.2900,
                                 0.3011,
                                 0.3123,
                                 0.3161,
                                 0.3199,
                                 0.3262,
                                 0.3325,
                                 0.3390,
                                 0.3454,
                                 0.3491,
                                 0.3528,
                                 0.3605,
                                 0.3681,
                                 0.3732,
                                 0.3782,
                                 0.3846,
                                 0.3910,
                                 0.3986,
                                 0.4062,
                                 0.4099,
                                 0.4137,
                                 0.4214,
                                 0.4290,
                                 0.4328,
                                 0.4366,
                                 0.4454,
                                 0.4542,
                                 0.4631,
                                 0.4720, // 100
                                 0.4720,
                                 0.4721,
                                 0.4834,
                                 0.4947,
                                 0.4998,
                                 0.5049,
                                 0.5125,
                                 0.5201,
                                 0.5290,
                                 0.5379, // 110
                                 0.5414,
                                 0.5449,
                                 0.5552,
                                 0.5654,
                                 0.5706,
                                 0.5758,
                                 0.5847,
                                 0.5936,
                                 0.6024,
                                 0.6112, // 120
                                 0.6151,
                                 0.6190, // 122 END
                                 0.6300,
                                 0.6400,
                                 0.6500,
                                 0.7000  // 126
                             });

LocoSpeedMapping mappingD445("D445",
                             46,
                             {
                                 0,       // 1
                                 0,
                                 0,
                                 0,
                                 0,
                                 0,
                                 0,
                                 0,
                                 0,
                                 0.0227, // 10 START
                                 0.0247,
                                 0.0267,
                                 0.0297,
                                 0.0326,
                                 0.0346,
                                 0.0366,
                                 0.0392,
                                 0.0419,
                                 0.0443,
                                 0.0467, // 20
                                 0.0501,
                                 0.0534,
                                 0.0560,
                                 0.0586,
                                 0.0620,
                                 0.0654,
                                 0.0709,
                                 0.0763,
                                 0.0791,
                                 0.0818, // 30
                                 0.0844,
                                 0.0871,
                                 0.0893,
                                 0.0916,
                                 0.0940,
                                 0.0964,
                                 0.0991,
                                 0.1018,
                                 0.1062,
                                 0.1105, // 40
                                 0.1156,
                                 0.1206,
                                 0.1205,
                                 0.1203,
                                 0.1264,
                                 0.1326, // 46 END
                                 0.1400,
                                 0.1500,
                                 0.1600,
                                 0.1700, // 50
                                 0.1800,
                                 0.1900,
                                 0.2000,
                                 0.2100,
                                 0.2200,
                                 0.2300,
                                 0.2400,
                                 0.2500,
                                 0.2600,
                                 0.2700,
                                 0.2800,
                                 0.2900,
                                 0.3000,
                                 0.3100,
                                 0.3200,
                                 0.3300,
                                 0.3400,
                                 0.3500,
                                 0.3600,
                                 0.3700,
                                 0.3800,
                                 0.3900,
                                 0.4000,
                                 0.4100,
                                 0.4200,
                                 0.4300,
                                 0.4400,
                                 0.4500,
                                 0.4600,
                                 0.4700,
                                 0.4800,
                                 0.4900,
                                 0.5000,
                                 0.5100,
                                 0.5200,
                                 0.5300,
                                 0.5400,
                                 0.5500,
                                 0.5600,
                                 0.5700,
                                 0.5800,
                                 0.5900,
                                 0.6000,
                                 0.6100,
                                 0.6200,
                                 0.6300,
                                 0.6400,
                                 0.6500,
                                 0.6600,
                                 0.6700, // 100
                                 0.6800,
                                 0.6900,
                                 0.7000,
                                 0.7100,
                                 0.7200,
                                 0.7300,
                                 0.7400,
                                 0.7500,
                                 0.7600,
                                 0.7700,
                                 0.7800,
                                 0.7900,
                                 0.8000,
                                 0.8100,
                                 0.8200,
                                 0.8300,
                                 0.8400,
                                 0.8500,
                                 0.8600,
                                 0.8700, // 120
                                 0.8800,
                                 0.8900,
                                 0.9000,
                                 0.9100,
                                 0.9200,
                                 0.9300,
                             });

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

    QWidget *monitorPage = new QWidget;
    LocoStatusWidget *status1 = new LocoStatusWidget;
    LocoStatusWidget *status2 = new LocoStatusWidget;
    QCheckBox *trainActiveCheck = new QCheckBox(tr("Train Active"));
    QCheckBox *locoAInvertCheck = new QCheckBox(tr("Invert Loco A"));
    QCheckBox *locoBInvertCheck = new QCheckBox(tr("Invert Loco B"));

    QHBoxLayout *monitorLay = new QHBoxLayout(monitorPage);
    monitorLay->addWidget(status1);
    monitorLay->addWidget(status2);
    monitorLay->addWidget(trainActiveCheck);
    monitorLay->addWidget(locoAInvertCheck);
    monitorLay->addWidget(locoBInvertCheck);
    mTabWidget->addTab(monitorPage, tr("Monitor"));

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

    LocomotivePool *mPool = new LocomotivePool(this);
    mPool->setCommandStation(mCommandStation);

    Locomotive *locoD753 = new Locomotive(mPool);
    locoD753->setAddress(47);
    locoD753->setSpeedMapping(mappingD753);
    mPool->addLoco(locoD753);

    Locomotive *locoD445 = new Locomotive(mPool);
    locoD445->setAddress(46);
    locoD445->setSpeedMapping(mappingD445);
    mPool->addLoco(locoD445);

    status1->setLocomotive(locoD753);
    status2->setLocomotive(locoD445);

    Train *train = Train::createTrain(locoD753, locoD445);
    train->setParent(this);

    connect(trainActiveCheck, &QCheckBox::toggled, train, &Train::setActive);
    connect(locoAInvertCheck, &QCheckBox::toggled, train, &Train::setLocoAInvert);
    connect(locoBInvertCheck, &QCheckBox::toggled, train, &Train::setLocoBInvert);
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

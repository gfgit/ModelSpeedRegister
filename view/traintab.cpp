#include "traintab.h"

#include <QFrame>
#include <QCheckBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QScrollArea>

#include <QContextMenuEvent>

#include <QPointer>
#include <QMenu>

#include <QFileDialog>

#include "../train/locomotive.h"
#include "../train/locostatuswidget.h"
#include "../train/locomotivepool.h"
#include "../train/train.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#include <QMessageBox>

bool loadSpeedCurve(const QString& fileName, Locomotive *loco)
{
    QFile f(fileName);
    if(!f.open(QFile::ReadOnly))
        return false;

    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());

    QJsonObject obj = doc.object();

    QString name = obj.value(QLatin1String("name")).toString();
    if(name.isEmpty())
    {
        name = f.fileName();
        int i = name.lastIndexOf('/');
        if(i >= 0 && i < (name.size() - 1))
            name = name.mid(i + 1);
    }

    std::array<double, 126> speedTable;

    QJsonValue tmp = obj.value(QLatin1String("curve_array"));
    if(tmp.isArray())
    {
        QJsonArray curve = tmp.toArray();
        if(curve.size() != 127 && curve.size() != 126)
            return false;

        int step = 1;
        if(curve.size() == 127)
            step = 0;

        for(auto val : curve)
        {
            if(step > 0)
            {
                speedTable[step - 1] = val.toDouble();
            }
            step++;
        }
    }
    else if(tmp = obj.value(QLatin1String("speed_mapping")); tmp.isArray())
    {
        QJsonArray sparseTable = tmp.toArray();

        double lastSpeed = 0;
        int lastStep = 0;

        for(QJsonValue v : sparseTable)
        {
            QJsonObject stepObj = v.toObject();
            int step = stepObj.value(QLatin1String("step")).toInt();
            double speed = stepObj.value(QLatin1String("speed")).toDouble();

            if(step > lastStep + 1)
            {
                // Linear interpolation of steps inbetween
                int numSteps = step - lastStep;
                double increment = (speed - lastSpeed) / double(numSteps);

                for(int i = 1; i < numSteps; i++)
                {
                    double calculatedSpeed = lastSpeed + increment * double(i);
                    int calculatedStep = lastStep + i;

                    if(step > 0 && step <= 126)
                        speedTable[calculatedStep - 1] = calculatedSpeed;
                }
            }

            lastStep = step;
            lastSpeed = speed;

            if(step > 0 && step <= 126)
                speedTable[step - 1] = speed;
        }
    }
    else
        return false;

    int address = obj.value(QLatin1String("dcc_address")).toInt();
    LocoSpeedMapping mapping(name, address, speedTable);
    loco->setSpeedMapping(mapping);
    loco->setAddress(address);

    return true;
}

TrainTab::TrainTab(LocomotivePool *pool, QWidget *parent)
    : QWidget{parent}
    , mPool(pool)
{
    mTrain = new Train(this);

    QVBoxLayout *lay = new QVBoxLayout(this);

    QPushButton *addBut = new QPushButton(tr("Add Loco"));
    lay->addWidget(addBut);
    connect(addBut, &QPushButton::clicked, this, &TrainTab::addNewLoco);

    QCheckBox *trainActiveCB = new QCheckBox(tr("Train Active"));
    lay->addWidget(trainActiveCB);
    connect(trainActiveCB, &QCheckBox::toggled, mTrain,
            [this, trainActiveCB](bool val)
    {
        if(!mTrain->setActive(val))
        {
            trainActiveCB->setChecked(mTrain->isActive());
            QMessageBox::warning(this, tr("Cannot Deactivate Train"),
                                tr("Please set speed to zero first."));
        }
    });

    mScrollArea = new QScrollArea;
    mScrollArea->setWidgetResizable(true);
    mScrollArea->setWidget(new QWidget);

    mGridLay = new QGridLayout(mScrollArea->widget());
    lay->addWidget(mScrollArea);
}

void TrainTab::addNewLoco()
{
    if(mTrain->isActive())
    {
        QMessageBox::warning(this, tr("Cannot Add Loco"),
                             tr("Please first deactivate Train."));
        return;
    }

    Item *item = createItem();
    mItems.append(item);

    int i = mGridLay->count();
    int row = i / GRID_COLS;
    int col = i % GRID_COLS;

    mGridLay->addWidget(item->frame, row, col);
}

bool TrainTab::eventFilter(QObject *watched, QEvent *e)
{
    if(e->type() != QEvent::ContextMenu)
        return QWidget::eventFilter(watched, e);

    for(int i = 0; i < mItems.count(); i++)
    {
        Item *item = mItems.at(i);
        if(watched != item->frame && watched != item->widget)
            continue;

        QWidget *p = static_cast<QWidget*>(watched);
        QContextMenuEvent *ev = static_cast<QContextMenuEvent*>(e);

        QPointer<QMenu> menu = new QMenu(p);

        QAction *actDel = menu->addAction(tr("Remove"));
        connect(actDel, &QAction::triggered, this,
                [this, item]()
        {
            removeItem(item);
        });

        QAction *actLoad = menu->addAction(tr("Load Speed Curve"));
        connect(actLoad, &QAction::triggered, this,
                [this, item]()
        {
            if(mTrain->isActive())
            {
                QMessageBox::warning(this, tr("Cannot Load Curve"),
                                     tr("Please first deactivate Train."));
                return;
            }

            QString f = QFileDialog::getOpenFileName(this, tr("Open Speed Curve"));
            if(f.isEmpty())
                return;

            loadSpeedCurve(f, item->loco);

            // Force Train update
            mTrain->removeLoco(item->loco);
            mTrain->addLoco(item->loco);
            mTrain->updateSpeedTable();
        });

        menu->exec(ev->globalPos());
        if(!menu)
            return true;

        delete menu;
        return true;
    }

    return QWidget::eventFilter(watched, e);
}

TrainTab::Item *TrainTab::createItem()
{
    Item *item = new Item;
    item->loco = new Locomotive(this);
    mPool->addLoco(item->loco);

    item->frame = new QFrame(mScrollArea);
    QVBoxLayout *frameLay = new QVBoxLayout(item->frame);
    item->widget = new LocoStatusWidget;
    item->widget->setLocomotive(item->loco);
    frameLay->addWidget(item->widget);

    item->frame->setContextMenuPolicy(Qt::DefaultContextMenu);
    item->widget->setContextMenuPolicy(Qt::DefaultContextMenu);
    item->frame->setFrameShape(QFrame::Box);
    item->frame->installEventFilter(this);
    item->widget->installEventFilter(this);

    item->invertCB = new QCheckBox;
    frameLay->addWidget(item->invertCB);

    connect(item->loco, &Locomotive::nameChanged,
            item->invertCB, [this, item](const QString& name)
    {
        item->invertCB->setText(tr("Invert %1 dir").arg(name));
    });

    connect(item->invertCB, &QCheckBox::toggled,
            mTrain, [this, item](bool val)
    {
        int locoIdx = mTrain->getLocoIdx(item->loco);
        if(locoIdx == -1)
            return;

        mTrain->setLocoInvertDir(locoIdx, val);
    });

    return item;
}

void TrainTab::removeItem(Item *item)
{
    if(mTrain->isActive())
    {
        QMessageBox::warning(this, tr("Cannot Remove Loco"),
                             tr("Please first deactivate Train."));
        return;
    }

    if(!mTrain->removeLoco(item->loco))
        return;

    int i = mGridLay->indexOf(item->frame);
    if(i >= 0)
    {
        mGridLay->removeWidget(item->frame);

        // Reposition next items
        for(; i < mGridLay->count(); i++)
        {
            int row = i / GRID_COLS;
            int col = i % GRID_COLS;

            auto layItem = mGridLay->takeAt(i);
            mGridLay->addItem(layItem, row, col);
        }
    }

    mItems.removeOne(item);

    delete item->frame;
    delete item->loco;
}

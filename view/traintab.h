#ifndef TRAINTAB_H
#define TRAINTAB_H

#include <QWidget>

#include <QVector>

class Locomotive;
class LocomotivePool;
class Train;

class LocoStatusWidget;
class QCheckBox;
class QFrame;
class QGridLayout;
class QScrollArea;

class TrainTab : public QWidget
{
    Q_OBJECT
public:
    TrainTab(LocomotivePool *pool, QWidget *parent = nullptr);

private slots:
    void addNewLoco();

private:
    bool eventFilter(QObject *watched, QEvent *e) override;

    struct Item
    {
        QFrame *frame;
        Locomotive *loco;
        LocoStatusWidget *widget;
        QCheckBox *invertCB;
    };

    Item *createItem();
    void removeItem(Item *item);

private:
    static const int GRID_COLS = 4;

    QVector<Item *> mItems;

    QVector<Locomotive> mLocomotives;

    Train *mTrain;

    LocomotivePool *mPool;

    QScrollArea *mScrollArea;
    QGridLayout *mGridLay;
};

#endif // TRAINTAB_H

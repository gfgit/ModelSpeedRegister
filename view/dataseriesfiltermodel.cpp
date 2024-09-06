#include "dataseriesfiltermodel.h"

#include "../recorder/idataseries.h"
#include "../recorder/recordingmanager.h"

#include "../chart/chart.h"

#include "dataseriesgraph.h"
#include <QValueAxis>

void setupAxisTicksOnZoom(QValueAxis *axis)
{
    QObject::connect(axis, &QValueAxis::rangeChanged, axis,
                     [axis](qreal min, qreal max)
    {
        qreal length = max - min;
        if(length < 15)
        {
            axis->setTickType(QValueAxis::TicksDynamic);
            axis->setTickInterval(5);
            axis->setMinorTickCount(4);
        }
        else if(length < 55)
        {
            axis->setTickType(QValueAxis::TicksDynamic);
            axis->setTickInterval(10);
            axis->setMinorTickCount(4);
        }
        else
        {
            axis->setTickType(QValueAxis::TicksFixed);
            axis->setMinorTickCount(0);
            if(length < 300)
                axis->setTickInterval(50);
            else if(length < 500)
                axis->setTickInterval(100);
            else if(length < 1500)
                axis->setTickInterval(1000);
            else if(length < 15000)
                axis->setTickInterval(10000);
            else
            {
                length = qFloor(length / 1000) * 1000;
                axis->setTickInterval(length);
            }
        }
    });
}

DataSeriesFilterModel::DataSeriesFilterModel(Chart *chart, QObject *parent)
    : QAbstractTableModel(parent)
    , mRecMgr(nullptr)
    , mChart(chart)
{
    mTimeAxis = new QValueAxis(this);
    mTimeAxis->setRange(0, 20);
    mTimeAxis->setLabelFormat("%.0f");
    mTimeAxis->setTitleText("Time (sec)");
    mTimeAxis->setTickType(QValueAxis::TicksDynamic);
    mTimeAxis->setTickInterval(10);
    mTimeAxis->setMinorTickCount(4);
    setupAxisTicksOnZoom(mTimeAxis);

    mSpeedAxis = new QValueAxis(this);
    mSpeedAxis->setRange(0, 1.0);
    mSpeedAxis->setLabelFormat("%.1f");
    mSpeedAxis->setTitleText("Speed (m/s)");
    mSpeedAxis->setTickType(QValueAxis::TicksDynamic);
    mSpeedAxis->setTickInterval(0.1);
    mSpeedAxis->setMinorTickCount(1);

    mStepAxis = new QValueAxis(this);
    mStepAxis->setRange(0, 127);
    mStepAxis->setLabelFormat("%.0f");
    mStepAxis->setTitleText("Step");

    mTravelledAxis = new QValueAxis(this);
    mTravelledAxis->setRange(0, 50000);
    mTravelledAxis->setLabelFormat("%.1f");
    mTravelledAxis->setTitleText("Distance (mm)");
    setupAxisTicksOnZoom(mTravelledAxis);

    mChart->addAxis(mTimeAxis, Qt::AlignBottom);
    mChart->addAxis(mSpeedAxis, Qt::AlignLeft);
    mChart->addAxis(mStepAxis, Qt::AlignRight);
    mChart->addAxis(mTravelledAxis, Qt::AlignRight);
}

DataSeriesFilterModel::~DataSeriesFilterModel()
{
    // Items are deleted by QChart and QObject parent
    mItems.clear();
}

QVariant DataSeriesFilterModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Vertical || role != Qt::DisplayRole)
        return QAbstractTableModel::headerData(section, orientation, role);

    switch (section)
    {
    case Name:
        return tr("Name");
    case Color:
        return tr("Color");
    case Type:
        return tr("Type");
    default:
        break;
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

int DataSeriesFilterModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : mItems.count();
}

int DataSeriesFilterModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : NCols;
}

QVariant DataSeriesFilterModel::data(const QModelIndex &idx, int role) const
{
    if (!idx.isValid() || idx.row() >= mItems.count() || idx.column() >= NCols)
        return QVariant();

    const DataSeriesGraph* item = mItems.at(idx.row());

    switch (role)
    {
    case Qt::DisplayRole:
    {
        if(idx.column() == Name)
            return item->dataSeries()->name();
        else if(idx.column() == Type)
            return IDataSeries::trType(item->dataSeries()->getType());
        break;
    }
    case Qt::EditRole:
    {
        if(idx.column() == Name)
            return item->dataSeries()->name();
        break;
    }
    case Qt::CheckStateRole:
    {
        if(idx.column() == Name)
            return item->isVisible() ? Qt::Checked : Qt::Unchecked;
        break;
    }
    case Qt::BackgroundRole:
    {
        if(idx.column() == Color)
            return item->color();
        break;
    }
    default:
        break;
    }
    return QVariant();
}

bool DataSeriesFilterModel::setData(const QModelIndex &idx, const QVariant &value, int role)
{
    if (!idx.isValid() || idx.row() >= mItems.count() || idx.column() >= NCols)
        return false;

    DataSeriesGraph* item = mItems[idx.row()];

    switch (role)
    {
    case Qt::EditRole:
    {
        if(idx.column() != Name)
            return false;

        item->dataSeries()->setName(value.toString().simplified());
        item->setName(item->dataSeries()->name());
        break;
    }
    case Qt::CheckStateRole:
    {
        if(idx.column() != Name)
            return false;

        Qt::CheckState cs = value.value<Qt::CheckState>();
        if(cs == Qt::Checked)
            item->show();
        else
            item->hide();
        break;
    }
    default:
        return false;
    }

    emit dataChanged(idx, idx);
    return true;
}

Qt::ItemFlags DataSeriesFilterModel::flags(const QModelIndex &idx) const
{
    if (!idx.isValid() || idx.row() >= mItems.count() || idx.column() >= NCols)
        return Qt::ItemFlags();

    Qt::ItemFlags f = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    if(idx.column() == Name)
    {
        f.setFlag(Qt::ItemIsEditable);
        f.setFlag(Qt::ItemIsUserCheckable);
    }

    return f;
}

QColor DataSeriesFilterModel::getColorAt(const QModelIndex &idx) const
{
    if (!idx.isValid() || idx.row() >= mItems.count() || idx.column() >= NCols)
        return QColor();

    const DataSeriesGraph* item = mItems.at(idx.row());
    return item->color();
}

void DataSeriesFilterModel::setColorAt(const QModelIndex &idx, const QColor& color)
{
    if (!idx.isValid() || idx.row() >= mItems.count() || idx.column() >= NCols)
        return;

    DataSeriesGraph* item = mItems[idx.row()];
    item->setColor(color);
    emit dataChanged(idx, idx);
}

RecordingManager *DataSeriesFilterModel::recMgr() const
{
    return mRecMgr;
}

void DataSeriesFilterModel::setRecMgr(RecordingManager *newRecMgr)
{
    beginResetModel();

    if(mRecMgr)
    {
        disconnect(mRecMgr, &RecordingManager::seriesRegistered,
                   this, &DataSeriesFilterModel::onSeriesRegistered);
        disconnect(mRecMgr, &RecordingManager::seriesUnregistered,
                   this, &DataSeriesFilterModel::onSeriesUnregistered);
    }

    qDeleteAll(mItems);
    mItems.clear();

    mRecMgr = newRecMgr;

    if(mRecMgr)
    {
        connect(mRecMgr, &RecordingManager::seriesRegistered,
                this, &DataSeriesFilterModel::onSeriesRegistered);
        connect(mRecMgr, &RecordingManager::seriesUnregistered,
                this, &DataSeriesFilterModel::onSeriesUnregistered);

        for(auto series : mRecMgr->getSeries())
        {
            onSeriesRegistered(series);
        }
    }

    endResetModel();
}

void DataSeriesFilterModel::setAxisRange(const QRectF &r)
{
    mTimeAxis->setRange(r.left(), r.right());
    mSpeedAxis->setRange(r.bottom(), r.top());
}

QRectF DataSeriesFilterModel::getAxisRange() const
{
    QRectF r;
    r.setTop(mSpeedAxis->max());
    r.setBottom(mSpeedAxis->min());
    r.setLeft(mTimeAxis->min());
    r.setRight(mTimeAxis->max());
    return r;
}

void DataSeriesFilterModel::onSeriesRegistered(IDataSeries *s)
{
    if(s->getType() == DataSeriesType::CurveMapping)
        return; // Curve mappings are not shown here

    int row = mItems.size();
    beginInsertRows(QModelIndex(), row, row);

    DataSeriesGraph *item = new DataSeriesGraph(s, this);
    mChart->addSeries(item);
    mItems.append(item);

    // Setup graph
    item->attachAxis(mTimeAxis);

    switch (item->dataSeries()->getType())
    {
    case DataSeriesType::RequestedSpeedStep:
        item->setColor(Qt::black);
        item->attachAxis(mStepAxis);
        connect(item->dataSeries(), &IDataSeries::pointAdded, this,
                [this](int, const QPointF& pt)
        {
            if(mAxisRangeFollowsChanges && pt.x() + 5 > mTimeAxis->max())
                mTimeAxis->setMax(pt.x() + 10);
        });
        break;
    case DataSeriesType::ReceivedSpeedStep:
        item->setColor(Qt::cyan);
        item->attachAxis(mStepAxis);
        break;
    case DataSeriesType::SensorRawData:
        item->setColor(Qt::green);
        item->attachAxis(mSpeedAxis);
        connect(item->dataSeries(), &IDataSeries::pointAdded, this,
                [this](int, const QPointF& pt)
        {
            if(mAxisRangeFollowsChanges && pt.y() + 0.3 > mSpeedAxis->max())
                mSpeedAxis->setMax(pt.y() + 0.5);
        });
        break;
    case DataSeriesType::MovingAverage:
        item->setColor(Qt::red);
        item->attachAxis(mSpeedAxis);
        break;
    case DataSeriesType::TotalStepAverage:
        item->setColor(Qt::darkGreen);
        item->attachAxis(mSpeedAxis);
        break;
    case DataSeriesType::TravelledDistance:
        item->setColor(Qt::darkGray);
        item->attachAxis(mTravelledAxis);
        break;
    default:
        break;
    }

    endInsertRows();
}

void DataSeriesFilterModel::onSeriesUnregistered(IDataSeries *s)
{
    if(s->getType() == DataSeriesType::CurveMapping)
        return; // Curve mappings are not shown here

    int row = -1;
    for(int i = 0; i < mItems.size(); i++)
    {
        if(mItems[i]->dataSeries() != s)
            continue;

        row = i;
        break;
    }

    if(row == -1)
        return;

    beginRemoveRows(QModelIndex(), row, row);

    delete mItems.takeAt(row);

    endRemoveRows();
}

bool DataSeriesFilterModel::axisRangeFollowsChanges() const
{
    return mAxisRangeFollowsChanges;
}

void DataSeriesFilterModel::setAxisRangeFollowsChanges(bool newAxisRangeFollowsChanges)
{
    mAxisRangeFollowsChanges = newAxisRangeFollowsChanges;
}

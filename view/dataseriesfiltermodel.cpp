#include "dataseriesfiltermodel.h"

#include "../recorder/idataseries.h"
#include "../recorder/recordingmanager.h"

#include "../chart/chart.h"

#include <QLineSeries>
#include <QValueAxis>

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
    mTimeAxis->setTickInterval(1);

    mSpeedAxis = new QValueAxis(this);
    mSpeedAxis->setRange(0, 1.5);
    mSpeedAxis->setLabelFormat("%.1f");
    mSpeedAxis->setTitleText("Speed (m/s)");

    mStepAxis = new QValueAxis(this);
    mStepAxis->setRange(0, 127);
    mStepAxis->setLabelFormat("%.0f");
    mStepAxis->setTitleText("Step");

    mChart->addAxis(mTimeAxis, Qt::AlignBottom);
    mChart->addAxis(mSpeedAxis, Qt::AlignLeft);
    mChart->addAxis(mStepAxis, Qt::AlignRight);
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
            if(pt.x() + 5 > mTimeAxis->max())
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
            if(pt.y() + 5 > mSpeedAxis->max())
                mSpeedAxis->setMax(pt.y() + 10);
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
    default:
        break;
    }

    endInsertRows();
}

void DataSeriesFilterModel::onSeriesUnregistered(IDataSeries *s)
{
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

DataSeriesGraph::DataSeriesGraph(IDataSeries *s, QObject *parent)
    : QLineSeries(parent)
    , mDataSeries(s)
{
    connect(mDataSeries, &IDataSeries::pointAdded, this, &DataSeriesGraph::pointAdded);
    connect(mDataSeries, &IDataSeries::pointRemoved, this, &DataSeriesGraph::pointRemoved);
    connect(mDataSeries, &IDataSeries::pointChanged, this, &DataSeriesGraph::pointChanged);

    setName(mDataSeries->name());
}

IDataSeries *DataSeriesGraph::dataSeries() const
{
    return mDataSeries;
}

void DataSeriesGraph::pointAdded(int index, const QPointF &point)
{
    insert(index, point);
}

void DataSeriesGraph::pointRemoved(int index)
{
    removePoints(index, 1);
}

void DataSeriesGraph::pointChanged(int index, const QPointF &newPoint)
{
    replace(index, newPoint);
}

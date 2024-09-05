#include "speedcurvetablemodel.h"

#include "../recorder/recordingmanager.h"
#include "../recorder/series/dataseriescurvemapping.h"

#include "dataseriesgraph.h"

#include <QTimerEvent>

#include "../chart/chart.h"
#include <QValueAxis>

SpeedCurveTableModel::SpeedCurveTableModel(Chart *chart, QObject *parent)
    : QAbstractTableModel(parent)
    , mRecMgr(nullptr)
    , mChart(chart)
{
    mStepAxis = new QValueAxis(this);
    mStepAxis->setRange(0, 126);
    mStepAxis->setLabelFormat("%.0f");
    mStepAxis->setTitleText("Step");
    mStepAxis->setTickType(QValueAxis::TicksFixed);
    mStepAxis->setTickInterval(1);

    mSpeedAxis = new QValueAxis(this);
    mSpeedAxis->setRange(0, 1.5);
    mSpeedAxis->setLabelFormat("%.1f");
    mSpeedAxis->setTitleText("Speed (m/s)");

    mChart->addAxis(mStepAxis, Qt::AlignBottom);
    mChart->addAxis(mSpeedAxis, Qt::AlignLeft);
}

SpeedCurveTableModel::~SpeedCurveTableModel()
{
    if(mRecalculationTimerId)
    {
        killTimer(mRecalculationTimerId);
        mRecalculationTimerId = 0;
    }

    // Item mGraph is deleted by QChart
    mSeries.clear();
}

QVariant SpeedCurveTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(mState == State::WaitingForRecalculation || section < 0)
        return QAbstractTableModel::headerData(section, orientation, role);

    if(orientation == Qt::Horizontal)
    {
        if(role == Qt::DisplayRole && section < mSeries.size())
            return mSeries.at(section).mGraph->name();
    }
    else
    {
        if(section == SpecialRows::VisibilityCheckBox)
        {
            if(role == Qt::DisplayRole)
                return tr("Vs:");
        }

        int step = getStepForRow(section);

        if(step >= 0)
        {
            if(role == Qt::DisplayRole)
                return step;

            if(role == Qt::BackgroundRole)
            {
                if(section == mStepStart[step])
                {
                    // Mark step start in light red
                    return QColor(qRgb(240, 108, 108));
                }
            }
        }
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

int SpeedCurveTableModel::rowCount(const QModelIndex &parent) const
{
    if(parent.isValid())
        return 0;

    if(mState == State::WaitingForRecalculation)
        return 1;

    return mLastRow + 1;
}

int SpeedCurveTableModel::columnCount(const QModelIndex &parent) const
{
    if(parent.isValid())
        return 0;

    if(mState == State::WaitingForRecalculation)
        return 1;

    return mSeries.size();
}

QVariant SpeedCurveTableModel::data(const QModelIndex &idx, int role) const
{
    if (!idx.isValid())
        return QVariant();

    if(mState == State::WaitingForRecalculation)
    {
        if(role == Qt::DisplayRole)
        {
            return tr("Loading...");
        }

        return QVariant();
    }

    if (idx.row() > mLastRow || idx.column() >= mSeries.size())
        return QVariant();

    DataSeriesGraph *series = mSeries.at(idx.column()).mGraph;

    if(idx.row() == SpecialRows::VisibilityCheckBox)
    {
        if(role == Qt::CheckStateRole)
        {
            return series->isVisible() ? Qt::Checked : Qt::Unchecked;
        }
        else if(role == Qt::BackgroundRole)
        {
            return series->color();
        }

        return QVariant();
    }

    int step = getStepForRow(idx.row());
    if(step < 0)
        return QVariant();

    int indexInSeries = idx.row() - mStepStart[step];

    int baseStepIndex = getFirstIndexForStep(series, step);
    if(baseStepIndex < 0)
        return QVariant();

    int lastStepIndex = getFirstIndexForStep(series, step + 1);

    indexInSeries += baseStepIndex;
    if(indexInSeries >= series->count() || indexInSeries >= lastStepIndex)
        return QVariant();

    if(role == Qt::DisplayRole)
        return series->at(indexInSeries).y();

    // FIXME: Implement other roles
    return QVariant();
}

bool SpeedCurveTableModel::setData(const QModelIndex &idx, const QVariant &value, int role)
{
    if(mState == State::WaitingForRecalculation)
        return false;

    if (idx.row() > mLastRow || idx.column() >= mSeries.size())
        return false;

    DataSeriesGraph *series = mSeries.at(idx.column()).mGraph;

    if(idx.row() == SpecialRows::VisibilityCheckBox && role == Qt::CheckStateRole)
    {
        Qt::CheckState cs = value.value<Qt::CheckState>();
        if(cs == Qt::Checked)
            series->show();
        else
            series->hide();

        emit dataChanged(idx, idx);
        return true;
    }

    return false;
}

Qt::ItemFlags SpeedCurveTableModel::flags(const QModelIndex &idx) const
{
    if (!idx.isValid())
        return Qt::NoItemFlags;

    if(mState == State::WaitingForRecalculation)
        return Qt::ItemIsEnabled;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
}

RecordingManager *SpeedCurveTableModel::recMgr() const
{
    return mRecMgr;
}

void SpeedCurveTableModel::setRecMgr(RecordingManager *newRecMgr)
{
    beginResetModel();

    if(mRecMgr)
    {
        disconnect(mRecMgr, &RecordingManager::seriesRegistered,
                   this, &SpeedCurveTableModel::onSeriesRegistered);
        disconnect(mRecMgr, &RecordingManager::seriesUnregistered,
                   this, &SpeedCurveTableModel::onSeriesUnregistered);
    }

    for(int i = mSeries.size() - 1; i >= 0; i--)
    {
        const DataSeriesColumn& col = mSeries.at(i);
        if(col.mSeries)
        {
            // It comes from recording manager, remove it
            delete col.mGraph;
            mSeries.removeAt(i);
        }
    }

    mRecMgr = newRecMgr;

    if(mRecMgr)
    {
        connect(mRecMgr, &RecordingManager::seriesRegistered,
                this, &SpeedCurveTableModel::onSeriesRegistered);
        connect(mRecMgr, &RecordingManager::seriesUnregistered,
                this, &SpeedCurveTableModel::onSeriesUnregistered);

        for(auto series : mRecMgr->getSeries())
        {
            onSeriesRegistered(series);
        }
    }

    endResetModel();
}

QColor SpeedCurveTableModel::getSeriesColor(int column) const
{
    if(column < 0 || column >= mSeries.count())
        return QColor();

    return mSeries.at(column).mGraph->color();
}

void SpeedCurveTableModel::setSeriesColor(int column, const QColor &color)
{
    if(column < 0 || column >= mSeries.count())
        return;

    mSeries.at(column).mGraph->setColor(color);
    QModelIndex idx = index(SpecialRows::VisibilityCheckBox, column);
    emit dataChanged(idx, idx);
}

void SpeedCurveTableModel::onSeriesRegistered(IDataSeries *s)
{
    if(s->getType() != DataSeriesType::CurveMapping)
        return;

    beginSetState(State::WaitingForRecalculation);

    DataSeriesColumn col;
    col.mSeries = static_cast<DataSeriesCurveMapping *>(s);
    col.mGraph = new DataSeriesGraph(s, this);

    mChart->addSeries(col.mGraph);
    col.mGraph->attachAxis(mStepAxis);
    col.mGraph->attachAxis(mSpeedAxis);

    connect(col.mSeries, &IDataSeries::pointAdded, this, &SpeedCurveTableModel::onSeriesChanged);
    connect(col.mSeries, &IDataSeries::pointRemoved, this, &SpeedCurveTableModel::onSeriesPointRemoved);
    connect(col.mSeries, &IDataSeries::pointChanged, this, &SpeedCurveTableModel::onSeriesChanged);

    mSeries.append(col);

    endSetState();
}

void SpeedCurveTableModel::onSeriesUnregistered(IDataSeries *s)
{
    if(s->getType() != DataSeriesType::CurveMapping)
        return;

    beginSetState(State::WaitingForRecalculation);

    for(int i = mSeries.size() - 1; i >= 0; i--)
    {
        const DataSeriesColumn& col = mSeries.at(i);
        if(col.mSeries == s)
        {
            // Remove it
            delete col.mGraph;
            mSeries.removeAt(i);
            break;
        }
    }

    endSetState();
}

void SpeedCurveTableModel::onSeriesChanged(int, const QPointF &pt)
{
    if(mAxisRangeFollowsChanges && pt.y() + 1 > mSpeedAxis->max())
        mSpeedAxis->setMax(pt.y() + 2);

    beginSetState(State::WaitingForRecalculation);
    endSetState();
}

void SpeedCurveTableModel::onSeriesPointRemoved()
{
    beginSetState(State::WaitingForRecalculation);
    endSetState();
}

void SpeedCurveTableModel::timerEvent(QTimerEvent *e)
{
    if(e->timerId() == mRecalculationTimerId && mRecalculationTimerId)
    {
        beginSetState(State::Normal);

        recalculate();

        endSetState();
        return;
    }

    QAbstractTableModel::timerEvent(e);
}

void SpeedCurveTableModel::beginSetState(State newState)
{
    if(newState == State::WaitingForRecalculation && mState == newState)
        return;

    if(mRecalculationTimerId)
    {
        killTimer(mRecalculationTimerId);
        mRecalculationTimerId = 0;
    }

    beginResetModel();

    mState = newState;
    if(mState == State::WaitingForRecalculation)
    {
        mState = State::BeginWaitInternal;
    }
}

void SpeedCurveTableModel::endSetState()
{
    if(mState == State::WaitingForRecalculation)
    {
        // Model was already waiting, no reset happened
        return;
    }

    if(mState == State::BeginWaitInternal)
    {
        mState = State::WaitingForRecalculation;
        mRecalculationTimerId = startTimer(500);
    }
    endResetModel();
}

void SpeedCurveTableModel::recalculate()
{
    int stepCount[126 + 1] = {0};
    for(int i = 0; i <= 126; i++)
    {
        // At least 1 row per step
        stepCount[i] = 1;
    }

    // Get highest number of rows per each step
    for(const DataSeriesColumn& col : mSeries)
    {
        int step = 0;
        int rows = 0;

        for(const QPointF& pt : col.mGraph->points())
        {
            if(pt.x() > step)
            {
                if(stepCount[step] < rows)
                    stepCount[step] = rows;

                step = pt.x();
                rows = 0;

                if(step > 126)
                    break;
            }

            rows++;
        }
    }

    // One row for checkboxes
    mLastRow = 1;

    for(int i = 0; i <= 126; i++)
    {
        mStepStart[i] = mLastRow;
        mLastRow += stepCount[i];
    }

    // Remove last empty row
    mLastRow--;
}

int SpeedCurveTableModel::getFirstIndexForStep(DataSeriesGraph *s, int step) const
{
    int i = 0;
    for(; i < s->count(); i++)
    {
        if(s->at(i).x() == step)
        {
            return i;
        }
    }

    return -1;
}

bool SpeedCurveTableModel::axisRangeFollowsChanges() const
{
    return mAxisRangeFollowsChanges;
}

void SpeedCurveTableModel::setAxisRangeFollowsChanges(bool newAxisRangeFollowsChanges)
{
    mAxisRangeFollowsChanges = newAxisRangeFollowsChanges;
}

void SpeedCurveTableModel::setAxisRange(const QRectF &r)
{
    mStepAxis->setRange(r.left(), r.right());
    mSpeedAxis->setRange(r.bottom(), r.top());
}

QRectF SpeedCurveTableModel::getAxisRange() const
{
    QRectF r;
    r.setTop(mSpeedAxis->max());
    r.setBottom(mSpeedAxis->min());
    r.setLeft(mStepAxis->min());
    r.setRight(mStepAxis->max());
    return r;
}

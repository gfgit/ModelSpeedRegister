#ifndef LOCOSPEEDCURVE_H
#define LOCOSPEEDCURVE_H

#include <QObject>
#include <QMultiHash>

class LocomotiveRecording;
class RecordingItem;

class LocoSpeedCurve : public QObject
{
    Q_OBJECT
public:
    explicit LocoSpeedCurve(QObject *parent = nullptr);

    LocomotiveRecording *recording() const;

    void setRecording(LocomotiveRecording *newRecording);

    QMultiHash<int, double> speedCurve() const;

signals:
    void speedCurveChanged(int step, const QList<double>& values);

private slots:
    void onItemChanged(int index, const RecordingItem& oldItem);

private:
    LocomotiveRecording *mRecording = nullptr;

    QMultiHash<int, double> mSpeedCurve;
};

#endif // LOCOSPEEDCURVE_H

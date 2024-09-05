#ifndef STARTTESTDLG_H
#define STARTTESTDLG_H

#include <QDialog>

class QSpinBox;

class StartTestDlg : public QDialog
{
    Q_OBJECT
public:
    explicit StartTestDlg(QWidget *parent = nullptr);

    int getLocoAddress() const;
    void setLocoAddress(int newLocoAddress);

    int getStartingDCCStep() const;
    void setStartingDCCStep(int newStartingDCCStep);

    int getDefaultStepTime() const;
    void setDefaultStepTime(int newDefaultStepTime);

private:
    QSpinBox *mLocoAddress;
    QSpinBox *mDefaultTimerForStep;
    QSpinBox *mStartingDCCStep;
};

#endif // STARTTESTDLG_H

#include "starttestdlg.h"

#include <QFormLayout>
#include <QSpinBox>
#include <QDialogButtonBox>

StartTestDlg::StartTestDlg(QWidget *parent)
    : QDialog{parent}
{
    setWindowTitle(tr("Start New Test"));

    QFormLayout *lay = new QFormLayout(this);

    mLocoAddress = new QSpinBox;
    mLocoAddress->setRange(1, 9999);
    lay->addRow(tr("Loco Address:"), mLocoAddress);

    mDefaultTimerForStep = new QSpinBox;
    mDefaultTimerForStep->setRange(1000, 10000);
    mDefaultTimerForStep->setSuffix(tr(" ms"));
    lay->addRow(tr("Step duration:"), mDefaultTimerForStep);

    mStartingDCCStep = new QSpinBox;
    mStartingDCCStep->setRange(1, 126);
    lay->addRow(tr("Start from DCC Step:"), mStartingDCCStep);

    QDialogButtonBox *box =
            new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                 Qt::Horizontal,
                                 this);
    lay->addWidget(box);
    connect(box, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(box, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

int StartTestDlg::getLocoAddress() const
{
    return mLocoAddress->value();
}

void StartTestDlg::setLocoAddress(int newLocoAddress)
{
    mLocoAddress->setValue(newLocoAddress);
}

int StartTestDlg::getStartingDCCStep() const
{
    return mStartingDCCStep->value();
}

void StartTestDlg::setStartingDCCStep(int newStartingDCCStep)
{
    mStartingDCCStep->setValue(newStartingDCCStep);
}

int StartTestDlg::getDefaultStepTime() const
{
    return mDefaultTimerForStep->value();
}

void StartTestDlg::setDefaultStepTime(int newDefaultStepTime)
{
    mDefaultTimerForStep->setValue(newDefaultStepTime);
}

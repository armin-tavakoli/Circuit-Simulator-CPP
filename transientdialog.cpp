#include "transientdialog.h"
#include "ValueParser.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QLabel>

TransientAnalysisDialog::TransientAnalysisDialog(QWidget *parent)
        : QDialog(parent)
{
    setWindowTitle(tr("Transient Analysis Settings"));

    stopTimeEdit = new QLineEdit("1m");
    startTimeEdit = new QLineEdit("0");
    timeStepEdit = new QLineEdit("1u");

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow(new QLabel(tr("Stop Time:")), stopTimeEdit);
    formLayout->addRow(new QLabel(tr("Time to start saving data:")), startTimeEdit);
    formLayout->addRow(new QLabel(tr("Time Step:")), timeStepEdit);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);
}

double TransientAnalysisDialog::getStopTime() const
{
    return parseValue(stopTimeEdit->text().toStdString());
}

double TransientAnalysisDialog::getStartTime() const
{
    return parseValue(startTimeEdit->text().toStdString());
}

double TransientAnalysisDialog::getTimeStep() const
{
    return parseValue(timeStepEdit->text().toStdString());
}

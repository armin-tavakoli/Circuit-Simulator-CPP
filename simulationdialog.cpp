#include "simulationdialog.h"
#include "ValueParser.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QTabWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QWidget>

SimulationDialog::SimulationDialog(QWidget *parent)
        : QDialog(parent)
{
    setWindowTitle(tr("Simulation Settings"));
    setMinimumWidth(400);

    tabWidget = new QTabWidget;
    createTransientTab();
    createAcSweepTab();

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tabWidget);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);
}

void SimulationDialog::createTransientTab()
{
    QWidget *transientTab = new QWidget;
    QFormLayout *formLayout = new QFormLayout;

    stopTimeEdit = new QLineEdit("1m");
    startTimeEdit = new QLineEdit("0");
    timeStepEdit = new QLineEdit("1u");

    formLayout->addRow(new QLabel(tr("Stop Time:")), stopTimeEdit);
    formLayout->addRow(new QLabel(tr("Time to start saving data:")), startTimeEdit);
    formLayout->addRow(new QLabel(tr("Time Step:")), timeStepEdit);

    transientTab->setLayout(formLayout);
    tabWidget->addTab(transientTab, tr("Transient"));
}

void SimulationDialog::createAcSweepTab()
{
    QWidget *acTab = new QWidget;
    QFormLayout *formLayout = new QFormLayout;

    startFreqEdit = new QLineEdit("1");
    stopFreqEdit = new QLineEdit("1k");
    numPointsEdit = new QLineEdit("100");
    sweepTypeCombo = new QComboBox;
    sweepTypeCombo->addItems({"Linear", "Decade", "Octave"});

    formLayout->addRow(new QLabel(tr("Start Frequency (Hz):")), startFreqEdit);
    formLayout->addRow(new QLabel(tr("Stop Frequency (Hz):")), stopFreqEdit);
    formLayout->addRow(new QLabel(tr("Number of Points:")), numPointsEdit);
    formLayout->addRow(new QLabel(tr("Sweep Type:")), sweepTypeCombo);

    acTab->setLayout(formLayout);
    tabWidget->addTab(acTab, tr("AC Sweep"));
}

int SimulationDialog::getCurrentTabIndex() const { return tabWidget->currentIndex(); }

double SimulationDialog::getStopTime() const { return parseValue(stopTimeEdit->text().toStdString()); }
double SimulationDialog::getStartTime() const { return parseValue(startTimeEdit->text().toStdString()); }
double SimulationDialog::getTimeStep() const { return parseValue(timeStepEdit->text().toStdString()); }

double SimulationDialog::getStartFreq() const { return parseValue(startFreqEdit->text().toStdString()); }
double SimulationDialog::getStopFreq() const { return parseValue(stopFreqEdit->text().toStdString()); }
int SimulationDialog::getNumPoints() const { return numPointsEdit->text().toInt(); }
std::string SimulationDialog::getSweepType() const { return sweepTypeCombo->currentText().toStdString(); }

#ifndef SIMULATIONDIALOG_H
#define SIMULATIONDIALOG_H

#include <QDialog>
#include <string>

class QTabWidget;
class QLineEdit;
class QComboBox;
class QDialogButtonBox;

class SimulationDialog : public QDialog
{
Q_OBJECT

public:
    explicit SimulationDialog(QWidget *parent = nullptr);

    int getCurrentTabIndex() const;

    // Transient getters
    double getStopTime() const;
    double getStartTime() const;
    double getTimeStep() const;

    // AC Sweep getters
    double getStartFreq() const;
    double getStopFreq() const;
    int getNumPoints() const;
    std::string getSweepType() const;


private:
    void createTransientTab();
    void createAcSweepTab();

    QTabWidget *tabWidget;
    QDialogButtonBox *buttonBox;

    // Transient widgets
    QLineEdit *stopTimeEdit;
    QLineEdit *startTimeEdit;
    QLineEdit *timeStepEdit;

    // AC Sweep widgets
    QLineEdit *startFreqEdit;
    QLineEdit *stopFreqEdit;
    QLineEdit *numPointsEdit;
    QComboBox *sweepTypeCombo;
};

#endif // SIMULATIONDIALOG_H

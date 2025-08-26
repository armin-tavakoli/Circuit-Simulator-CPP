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


    double getStopTime() const;
    double getStartTime() const;
    double getTimeStep() const;


    double getStartFreq() const;
    double getStopFreq() const;
    int getNumPoints() const;
    std::string getSweepType() const;

    double getBaseFreq() const;
    double getStartPhase() const;
    double getStopPhase() const;
    int getNumPointsPhase() const;

    std::string getDcSourceName() const;
    double getDcStartValue() const;
    double getDcStopValue() const;
    double getDcIncrement() const;


private:
    void createTransientTab();
    void createAcSweepTab();
    void createPhaseSweepTab();
    void createDcSweepTab();

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

    QLineEdit *baseFreqEdit;
    QLineEdit *startPhaseEdit;
    QLineEdit *stopPhaseEdit;
    QLineEdit *numPointsPhaseEdit;

    QLineEdit *dcSourceEdit;
    QLineEdit *startValueEdit;
    QLineEdit *stopValueEdit;
    QLineEdit *incrementEdit;
};

#endif // SIMULATIONDIALOG_H

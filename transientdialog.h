#ifndef TRANSIENTDIALOG_H
#define TRANSIENTDIALOG_H

#include <QDialog>

// Forward declarations for UI elements
class QLineEdit;
class QDialogButtonBox;

class TransientAnalysisDialog : public QDialog
{
Q_OBJECT

public:
    explicit TransientAnalysisDialog(QWidget *parent = nullptr);

    double getStopTime() const;
    double getStartTime() const;
    double getTimeStep() const;

private:
    QLineEdit *stopTimeEdit;
    QLineEdit *startTimeEdit;
    QLineEdit *timeStepEdit;
    QDialogButtonBox *buttonBox;
};

#endif
// TransientAnalysisDialog.h
#ifndef TRANSIENTANALYSISDIALOG_H
#define TRANSIENTANALYSISDIALOG_H

#include <QDialog>

// Forward declarations
class QLineEdit;
class QDialogButtonBox;

class TransientAnalysisDialog : public QDialog {
Q_OBJECT

public:
    explicit TransientAnalysisDialog(QWidget *parent = nullptr);

    // توابع عمومی برای دریافت مقادیر وارد شده توسط کاربر
    double stopTime() const;
    double startTime() const;
    double maxTimestep() const;

private:
    QLineEdit *stopTimeEdit;
    QLineEdit *startTimeEdit;
    QLineEdit *maxTimestepEdit;
    QDialogButtonBox *buttonBox;
};

#endif // TRANSIENTANALYSISDIALOG_H
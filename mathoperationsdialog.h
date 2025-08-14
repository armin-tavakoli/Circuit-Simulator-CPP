#ifndef MATHOPERATIONSDIALOG_H
#define MATHOPERATIONSDIALOG_H

#include <QDialog>
#include <QStringList>

class QComboBox;
class QLineEdit;

class MathOperationsDialog : public QDialog
{
Q_OBJECT

public:
    explicit MathOperationsDialog(const QStringList& signalNames, QWidget *parent = nullptr);

    QString getFirstSignal() const;
    QString getSecondSignal() const;
    QString getOperation() const;
    QString getResultSignalName() const;

private:
    QComboBox *m_firstSignalCombo;
    QComboBox *m_secondSignalCombo;
    QComboBox *m_operationCombo;
    QLineEdit *m_resultNameEdit;
};

#endif //MATHOPERATIONSDIALOG_H
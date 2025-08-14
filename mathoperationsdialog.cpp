#include "mathoperationsdialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QDialogButtonBox>

MathOperationsDialog::MathOperationsDialog(const QStringList& signalNames, QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Math Operations");

    m_firstSignalCombo = new QComboBox();
    m_firstSignalCombo->addItems(signalNames);

    m_secondSignalCombo = new QComboBox();
    m_secondSignalCombo->addItems(signalNames);

    m_operationCombo = new QComboBox();
    m_operationCombo->addItems({"+", "-"}); // فعلا فقط جمع و تفریق

    m_resultNameEdit = new QLineEdit("Result1");

    QFormLayout *formLayout = new QFormLayout();
    formLayout->addRow("First Signal:", m_firstSignalCombo);
    formLayout->addRow("Operation:", m_operationCombo);
    formLayout->addRow("Second Signal:", m_secondSignalCombo);
    formLayout->addRow("Result Name:", m_resultNameEdit);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);
}

QString MathOperationsDialog::getFirstSignal() const { return m_firstSignalCombo->currentText(); }
QString MathOperationsDialog::getSecondSignal() const { return m_secondSignalCombo->currentText(); }
QString MathOperationsDialog::getOperation() const { return m_operationCombo->currentText(); }
QString MathOperationsDialog::getResultSignalName() const { return m_resultNameEdit->text(); }
#include "SaveSubcircuitDialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QLabel>
#include <QStringList>
#include <stdexcept>

SaveSubcircuitDialog::SaveSubcircuitDialog(QWidget *parent)
        : QDialog(parent)
{
    setWindowTitle(tr("Save as Subcircuit"));
    setMinimumWidth(350);

    fileNameEdit = new QLineEdit(this);
    fileNameEdit->setPlaceholderText(tr("e.g., op_amp"));

    portsEdit = new QLineEdit(this);
    portsEdit->setPlaceholderText(tr("e.g., 1, 2"));

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow(new QLabel(tr("Subcircuit Name:")), fileNameEdit);
    formLayout->addRow(new QLabel(tr("External Ports (comma-separated):")), portsEdit);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);
}

QString SaveSubcircuitDialog::getFileName() const
{
    return fileNameEdit->text();
}

std::vector<int> SaveSubcircuitDialog::getExternalPorts() const
{
    std::vector<int> ports;
    QString text = portsEdit->text();
    QStringList portStrings = text.split(',', Qt::SkipEmptyParts);

    for (const QString& str : portStrings) {
        bool ok;
        int portNum = str.trimmed().toInt(&ok);
        if (ok) {
            ports.push_back(portNum);
        } else {
            throw std::invalid_argument("Invalid port number format. Please use comma-separated integers.");
        }
    }
    return ports;
}

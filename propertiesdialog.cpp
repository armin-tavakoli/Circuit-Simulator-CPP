#include "propertiesdialog.h"
#include "ValueParser.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QLabel>

PropertiesDialog::PropertiesDialog(const std::map<std::string, double>& properties, QWidget *parent)
        : QDialog(parent)
{
    setWindowTitle(tr("Set Component Properties"));
    m_layout = new QVBoxLayout(this);
    QFormLayout *formLayout = new QFormLayout();

    for (const auto& pair : properties) {
        QString label = QString::fromStdString(pair.first);
        QLineEdit *lineEdit = new QLineEdit(QString::number(pair.second));
        formLayout->addRow(new QLabel(label), lineEdit);
        m_lineEdits.push_back({pair.first, lineEdit});
    }

    m_layout->addLayout(formLayout);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &PropertiesDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &PropertiesDialog::reject);

    m_layout->addWidget(buttonBox);
}

std::map<std::string, double> PropertiesDialog::getProperties() const
{
    std::map<std::string, double> newProperties;
    for (const auto& pair : m_lineEdits) {
        std::string propName = pair.first;
        QString valueStr = pair.second->text();
        newProperties[propName] = parseValue(valueStr.toStdString());
    }
    return newProperties;
}

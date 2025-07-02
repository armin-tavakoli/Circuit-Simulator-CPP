#ifndef PROPERTIESDIALOG_H
#define PROPERTIESDIALOG_H

#include <QDialog>
#include <map>
#include <string>
#include <vector>

class QLineEdit;
class QVBoxLayout;

class PropertiesDialog : public QDialog
{
Q_OBJECT

public:
    explicit PropertiesDialog(const std::map<std::string, double>& properties, QWidget *parent = nullptr);
    std::map<std::string, double> getProperties() const;

private:
    std::vector<std::pair<std::string, QLineEdit*>> m_lineEdits;
    QVBoxLayout *m_layout;
};

#endif // PROPERTIESDIALOG_H

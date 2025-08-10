#ifndef SAVESUBCIRCUITDIALOG_H
#define SAVESUBCIRCUITDIALOG_H

#include <QDialog>
#include <QString>
#include <vector>

class QLineEdit;
class QDialogButtonBox;

class SaveSubcircuitDialog : public QDialog
{
Q_OBJECT

public:
    explicit SaveSubcircuitDialog(QWidget *parent = nullptr);

    QString getFileName() const;
    std::vector<int> getExternalPorts() const;

private:
    QLineEdit *fileNameEdit;
    QLineEdit *portsEdit;
    QDialogButtonBox *buttonBox;
};

#endif // SAVESUBCIRCUITDIALOG_H

#ifndef PLOTSELECTIONDIALOG_H
#define PLOTSELECTIONDIALOG_H

#include <QDialog>
#include <QStringList>

class QListWidget;
class QDialogButtonBox;

class PlotSelectionDialog : public QDialog
{
Q_OBJECT

public:
    explicit PlotSelectionDialog(const QStringList &availablePlots, QWidget *parent = nullptr);

    QStringList getSelectedPlots() const;

private:
    QListWidget *m_listWidget;
    QDialogButtonBox *m_buttonBox;
};

#endif

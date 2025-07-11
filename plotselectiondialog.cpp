#include "plotselectiondialog.h"
#include <QVBoxLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QDialogButtonBox>

PlotSelectionDialog::PlotSelectionDialog(const QStringList &availablePlots, QWidget *parent)
        : QDialog(parent)
{
    setWindowTitle(tr("Select Plots to Display"));
    setMinimumWidth(300);

    m_listWidget = new QListWidget(this);
    for (const QString &plotName : availablePlots) {
        QListWidgetItem *item = new QListWidgetItem(plotName, m_listWidget);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable); // Make item checkable
        item->setCheckState(Qt::Unchecked); // Default to unchecked
    }

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &PlotSelectionDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &PlotSelectionDialog::reject);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_listWidget);
    layout->addWidget(m_buttonBox);
    setLayout(layout);
}

QStringList PlotSelectionDialog::getSelectedPlots() const
{
    QStringList selectedPlots;
    for (int i = 0; i < m_listWidget->count(); ++i) {
        QListWidgetItem *item = m_listWidget->item(i);
        if (item->checkState() == Qt::Checked) {
            selectedPlots.append(item->text());
        }
    }
    return selectedPlots;
}

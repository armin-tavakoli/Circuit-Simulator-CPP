#include "SubCircuitItem.h"
#include "terminalitem.h"
#include "SubCircuit.h"
#include "mainwindow.h"
#include <QPainter>
#include <QString>
#include <QFileInfo>
#include <QGraphicsSceneContextMenuEvent>
#include <QApplication>
#include <QMenu>

SubCircuitItem::SubCircuitItem(Component *component, QGraphicsItem *parent)
        : ComponentItem(component, parent)
{
    if (m_terminal1) m_terminal1->setPos(-60, 0);
    if (m_terminal2) m_terminal2->setPos(60, 0);
}

QRectF SubCircuitItem::boundingRect() const
{
    return QRectF(-65, -40, 130, 80);
}

void SubCircuitItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    QPen pen(isSelected() ? Qt::cyan : Qt::white, 2);
    painter->setPen(pen);
    painter->setBrush(Qt::darkGray);
    painter->drawRect(-50, -30, 100, 60);

    painter->drawLine(-60, 0, -50, 0);
    painter->drawLine(50, 0, 60, 0);

    painter->setPen(Qt::yellow);
    painter->setFont(QFont("Arial", 10));

    QRectF nameRect(-50, -28, 100, 20);
    painter->drawText(nameRect, Qt::AlignCenter, QString::fromStdString(getComponent()->getName()));

    if (auto sub = dynamic_cast<SubCircuit*>(getComponent())) {
        QRectF defNameRect(-50, 0, 100, 20);
        QString fullPath = QString::fromStdString(sub->getDefinitionFile());
        QFileInfo fileInfo(fullPath);
        painter->drawText(defNameRect, Qt::AlignCenter, fileInfo.baseName());
    }
}

void SubCircuitItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu menu;
    QAction *editAction = menu.addAction("Edit Subcircuit");
    connect(editAction, &QAction::triggered, this, &SubCircuitItem::onEditSubcircuit);

    menu.exec(event->screenPos());
}

void SubCircuitItem::onEditSubcircuit()
{
    if (auto sub = dynamic_cast<SubCircuit*>(getComponent()))
    {
        MainWindow* mainWindow = nullptr;
                foreach(QWidget *widget, QApplication::topLevelWidgets()) {
                if (widget->inherits("MainWindow")) {
                    mainWindow = qobject_cast<MainWindow*>(widget);
                    break;
                }
            }

        if (mainWindow) {
            QString filePath = QString::fromStdString(sub->getDefinitionFile());
            mainWindow->openFileInNewTab(filePath);
        }
    }
}

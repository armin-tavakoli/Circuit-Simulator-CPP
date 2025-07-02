#include "capacitoritem.h"
#include "terminalitem.h"
#include <QPainter>
#include <QString>

CapacitorItem::CapacitorItem(Component *component, QGraphicsItem *parent)
        : ComponentItem(component, parent)
{
    m_terminal1->setPos(-30, 0);
    m_terminal2->setPos(30, 0);
}

QRectF CapacitorItem::boundingRect() const
{
    return QRectF(-30, -30, 60, 60);
}

void CapacitorItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    QPen pen(isSelected() ? Qt::cyan : Qt::white, 2);
    painter->setPen(pen);
    painter->drawLine(-30, 0, -5, 0);
    painter->drawLine(-5, -10, -5, 10);
    painter->drawLine(5, -10, 5, 10);
    painter->drawLine(5, 0, 30, 0);

    painter->setPen(Qt::yellow);
    // Display component name
    QRectF nameRect(-30, -30, 60, 20);
    painter->drawText(nameRect, Qt::AlignCenter, QString::fromStdString(getComponent()->getName()));

    // Display component value
    QRectF valueRect(-30, 10, 60, 20);
    painter->drawText(valueRect, Qt::AlignCenter, QString::fromStdString(getComponent()->getDisplayValue()));
}

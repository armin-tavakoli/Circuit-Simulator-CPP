#include "grounditem.h"
#include "terminalitem.h"
#include <QPainter>

GroundItem::GroundItem(QGraphicsItem *parent)
        : ComponentItem(nullptr, parent)
{
    m_terminal1->setPos(0, 0);
    m_terminal2->setVisible(false);
}

QRectF GroundItem::boundingRect() const
{
    return QRectF(-20, 0, 40, 30);
}

void GroundItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    QPen pen(isSelected() ? Qt::cyan : Qt::white, 2);
    painter->setPen(pen);
    painter->drawLine(0, 0, 0, 10);
    painter->drawLine(-20, 10, 20, 10);
    painter->drawLine(-15, 15, 15, 15);
    painter->drawLine(-10, 20, 10, 20);
}

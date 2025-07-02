#include "resistoritem.h"
#include "terminalitem.h"
#include <QPainter>
#include <QString>

ResistorItem::ResistorItem(Component *component, QGraphicsItem *parent)
        : ComponentItem(component, parent)
{
    m_terminal1->setPos(-40, 0);
    m_terminal2->setPos(40, 0);
}

QRectF ResistorItem::boundingRect() const
{
    return QRectF(-40, -30, 80, 60);
}

void ResistorItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    QPen pen(isSelected() ? Qt::cyan : Qt::white, 2);
    painter->setPen(pen);
    QPolygonF polyline;
    polyline << QPointF(-40, 0) << QPointF(-30, 0)
             << QPointF(-25, -8) << QPointF(-15, 8)
             << QPointF(-5, -8) << QPointF(5, 8)
             << QPointF(15, -8) << QPointF(25, 8)
             << QPointF(30, 0) << QPointF(40, 0);
    painter->drawPolyline(polyline);

    painter->setPen(Qt::yellow);
    // Display component name
    QRectF nameRect(-40, -30, 80, 20);
    painter->drawText(nameRect, Qt::AlignCenter, QString::fromStdString(getComponent()->getName()));

    // Display component value
    QRectF valueRect(-40, 10, 80, 20);
    painter->drawText(valueRect, Qt::AlignCenter, QString::fromStdString(getComponent()->getDisplayValue()));
}

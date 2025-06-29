#include "inductoritem.h"
#include "terminalitem.h"
#include <QPainter>
#include <QString>
#include <QtMath>

InductorItem::InductorItem(Component *component, QGraphicsItem *parent)
        : ComponentItem(component, parent)
{
    m_terminal1->setPos(-40, 0);
    m_terminal2->setPos(40, 0);
}

QRectF InductorItem::boundingRect() const
{
    return QRectF(-40, -30, 80, 60);
}

void InductorItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    QPen pen(isSelected() ? Qt::cyan : Qt::white, 2);
    painter->setPen(pen);
    painter->drawLine(-40, 0, -30, 0);
    painter->drawArc(QRectF(-30, -10, 20, 20), 180 * 16, -180 * 16);
    painter->drawArc(QRectF(-10, -10, 20, 20), 180 * 16, -180 * 16);
    painter->drawArc(QRectF(10, -10, 20, 20), 180 * 16, -180 * 16);
    painter->drawLine(30, 0, 40, 0);
    painter->setPen(Qt::yellow);
    QRectF textRect(-40, -30, 80, 20);
    painter->drawText(textRect, Qt::AlignCenter, QString::fromStdString(getComponent()->getName()));
}
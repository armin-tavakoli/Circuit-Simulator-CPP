#include "currentsourceitem.h"
#include "terminalitem.h"
#include <QPainter>
#include <QString>
#include "Component.h"

CurrentSourceItem::CurrentSourceItem(Component *component, QGraphicsItem *parent)
        : ComponentItem(component, parent)
{
    m_terminal1->setPos(0, -40);
    m_terminal2->setPos(0, 40);
}

QRectF CurrentSourceItem::boundingRect() const
{
    return QRectF(-30, -40, 60, 80);
}

void CurrentSourceItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    QPen pen(isSelected() ? Qt::cyan : Qt::white, 2);
    painter->setPen(pen);
    painter->drawEllipse(QPointF(0, 0), 20, 20);
    painter->drawLine(0, -10, 0, 10);
    painter->drawLine(0, -10, -5, -3);
    painter->drawLine(0, -10, 5, -3);
    painter->drawLine(0, -40, 0, -20);
    painter->drawLine(0, 20, 0, 40);
    painter->setPen(Qt::yellow);
    painter->setFont(QFont("Arial", 10));
    QRectF nameRect(-30, -40, 60, 20);
    painter->drawText(nameRect, Qt::AlignCenter, QString::fromStdString(getComponent()->getName()));
    if (auto cs = dynamic_cast<CurrentSource*>(getComponent())) {
        QString valueText = QString::number(cs->get_dc_value()) + "A";
        QRectF valueRect(-30, 20, 60, 20);
        painter->drawText(valueRect, Qt::AlignCenter, valueText);
    }
}

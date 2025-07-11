#include "terminalitem.h"
#include "componentitem.h"
#include "polylinewireitem.h"
#include <QPainter>

TerminalItem::TerminalItem(ComponentItem *parent, int terminalId)
        : QGraphicsItem(parent), parentComponent(parent), id(terminalId)
{
    setAcceptHoverEvents(true);
}

QRectF TerminalItem::boundingRect() const { return QRectF(-4, -4, 8, 8); }

void TerminalItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option); Q_UNUSED(widget);
    painter->setBrush(isHovered ? Qt::yellow : Qt::red);
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(boundingRect());
}

void TerminalItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    isHovered = true;
    update();
    QGraphicsItem::hoverEnterEvent(event);
}

void TerminalItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    isHovered = false;
    update();
    QGraphicsItem::hoverLeaveEvent(event);
}

void TerminalItem::addWire(PolylineWireItem *wire) { if (!m_wires.contains(wire)) m_wires.append(wire); }
void TerminalItem::removeWire(PolylineWireItem *wire) { m_wires.removeAll(wire); }
const QList<PolylineWireItem*>& TerminalItem::getWires() const { return m_wires; }
void TerminalItem::updateConnectedWires() { for (PolylineWireItem* wire : m_wires) wire->updatePosition(); }

ComponentItem* TerminalItem::getParentComponent() const { return parentComponent; }
int TerminalItem::getId() const { return id; }

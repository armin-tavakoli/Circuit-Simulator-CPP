#include "componentitem.h"
#include "terminalitem.h"
#include <cmath>
#include <QDebug>
#include <QTimer> // Required for QTimer::singleShot

ComponentItem::ComponentItem(Component *component, QGraphicsItem *parent)
        : QGraphicsObject(parent), logicalComponent(component)
{
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);

    m_terminal1 = new TerminalItem(this, 0);
    m_terminal2 = new TerminalItem(this, 1);
}

// The destructor is defaulted in the header, no need for a body here.

QRectF ComponentItem::boundingRect() const
{
    // Specific components will override this.
    return QRectF();
}

void ComponentItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(painter);
    Q_UNUSED(option);
    Q_UNUSED(widget);
}

Component* ComponentItem::getComponent() const
{
    return logicalComponent;
}

QVariant ComponentItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionChange && scene()) {
        QPointF newPos = value.toPointF();
        const int gridSize = 20;
        qreal x = round(newPos.x() / gridSize) * gridSize;
        qreal y = round(newPos.y() / gridSize) * gridSize;
        QPointF snappedPos(x, y);

        // This is a workaround to update wires *after* the move has been committed.
        // It schedules the update to happen as soon as the event loop is free.
        QTimer::singleShot(0, this, [this]() {
            if (m_terminal1) m_terminal1->updateConnectedWires();
            if (m_terminal2 && m_terminal2->isVisible()) m_terminal2->updateConnectedWires();
        });

        return snappedPos;
    }
    return QGraphicsItem::itemChange(change, value);
}

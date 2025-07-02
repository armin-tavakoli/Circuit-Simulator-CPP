#include "polylinewireitem.h"
#include "terminalitem.h"
#include <QPainter>
#include <QPen>
#include <QVector>

PolylineWireItem::PolylineWireItem(TerminalItem *startTerminal, QGraphicsItem *parent)
        : QGraphicsObject(parent), m_startTerminal(startTerminal), m_endTerminal(nullptr)
{
    setZValue(1); // Wires should be behind junctions
    if (m_startTerminal) {
        m_points.append(m_startTerminal->scenePos());
        m_startTerminal->addWire(this);
    }
}

PolylineWireItem::~PolylineWireItem()
{
    if (m_startTerminal) {
        m_startTerminal->removeWire(this);
    }
    if (m_endTerminal) {
        m_endTerminal->removeWire(this);
    }
}

void PolylineWireItem::addPoint(const QPointF &point)
{
    if (m_points.isEmpty() || m_points.last() != point) {
        m_points.append(point);
        prepareGeometryChange();
    }
}

void PolylineWireItem::setEndTerminal(TerminalItem *endTerminal)
{
    m_endTerminal = endTerminal;
    if (m_endTerminal) {
        addPoint(m_endTerminal->scenePos());
        m_endTerminal->addWire(this);
    }
}

QPointF PolylineWireItem::lastPoint() const
{
    return m_points.isEmpty() ? QPointF() : m_points.last();
}

void PolylineWireItem::updatePosition()
{
    if (!m_startTerminal) return;

    QPointF newStartPos = m_startTerminal->scenePos();
    if (m_points.first() != newStartPos) {
        m_points.first() = newStartPos;
        if (m_points.size() > 1) {
            // The first segment is horizontal, so update the Y of the adjacent corner
            m_points[1].setY(newStartPos.y());
        }
    }

    if (m_endTerminal) {
        QPointF newEndPos = m_endTerminal->scenePos();
        if (m_points.last() != newEndPos) {
            m_points.last() = newEndPos;
            if (m_points.size() > 1) {
                // The last segment is vertical, so update the X of the adjacent corner
                m_points[m_points.size() - 2].setX(newEndPos.x());
            }
        }
    }

    prepareGeometryChange();
    update();
}

QRectF PolylineWireItem::boundingRect() const
{
    return QPolygonF(m_points.toVector()).boundingRect().adjusted(-2, -2, 2, 2);
}

void PolylineWireItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    QPen pen(Qt::green, 2);
    painter->setPen(pen);

    if (m_points.size() >= 2) {
        painter->drawPolyline(QPolygonF(m_points.toVector()));
    }
}

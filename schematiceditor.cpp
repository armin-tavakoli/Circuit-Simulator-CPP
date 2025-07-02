#include "schematiceditor.h"
#include <QPainter>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QDebug>
#include "terminalitem.h"
#include "polylinewireitem.h"
#include "junctionitem.h"

SchematicEditor::SchematicEditor(QWidget *parent)
        : QGraphicsView(parent)
{
    setScene(new QGraphicsScene(this));
    scene()->setSceneRect(-5000, -5000, 10000, 10000);
    setRenderHint(QPainter::Antialiasing);
    setMouseTracking(true);
    m_wiringState = WiringState::NotWiring;
}

void SchematicEditor::drawBackground(QPainter *painter, const QRectF &rect)
{
    QGraphicsView::drawBackground(painter, rect);
    const int gridSize = 20;
    QPen pen(Qt::darkGray, 0);
    pen.setStyle(Qt::DotLine);
    painter->setPen(pen);
    qreal left = int(rect.left()) - (int(rect.left()) % gridSize);
    for (qreal x = left; x < rect.right(); x += gridSize)
        painter->drawLine(QPointF(x, rect.top()), QPointF(x, rect.bottom()));
    qreal top = int(rect.top()) - (int(rect.top()) % gridSize);
    for (qreal y = top; y < rect.bottom(); y += gridSize)
        painter->drawLine(QPointF(rect.left(), y), QPointF(rect.right(), y));
}

// Helper function to find a terminal at a specific position
TerminalItem* SchematicEditor::getTerminalAt(const QPoint& pos)
{
    for (QGraphicsItem* item : items(pos)) {
        if (auto terminal = dynamic_cast<TerminalItem*>(item)) {
            return terminal;
        }
    }
    return nullptr;
}

// Helper function to find a wire at a specific position
PolylineWireItem* SchematicEditor::getWireAt(const QPoint& pos)
{
    for (QGraphicsItem* item : items(pos)) {
        if (auto wire = dynamic_cast<PolylineWireItem*>(item)) {
            return wire;
        }
    }
    return nullptr;
}

// Helper function to snap a point to the nearest grid point
QPointF SchematicEditor::snapToGrid(const QPointF& pos)
{
    const int gridSize = 20;
    qreal x = round(pos.x() / gridSize) * gridSize;
    qreal y = round(pos.y() / gridSize) * gridSize;
    return QPointF(x, y);
}

// Slot to enable/disable wiring mode
void SchematicEditor::toggleWiringMode(bool enabled)
{
    if (enabled) {
        m_wiringState = WiringState::NotWiring; // Wait for the first click to start
    } else {
        // Cancel any ongoing wiring process
        if (m_currentWire) {
            scene()->removeItem(m_currentWire);
            delete m_currentWire;
            m_currentWire = nullptr;
        }
        for(auto segment : m_tempPreviewSegments) {
            scene()->removeItem(segment);
            delete segment;
        }
        m_tempPreviewSegments.clear();
        m_wiringState = WiringState::NotWiring;
    }
}

// Handles mouse movement to draw temporary wire previews
void SchematicEditor::mouseMoveEvent(QMouseEvent *event)
{
    if (m_wiringState == WiringState::DrawingWire) {
        for(auto segment : m_tempPreviewSegments) {
            scene()->removeItem(segment);
            delete segment;
        }
        m_tempPreviewSegments.clear();

        QPointF currentPos = snapToGrid(mapToScene(event->pos()));
        QPointF startPos = m_currentWire->lastPoint();
        QPointF cornerPos(currentPos.x(), startPos.y());

        QPen tempPen(Qt::white, 2, Qt::DashLine);
        auto seg1 = new QGraphicsLineItem(QLineF(startPos, cornerPos));
        auto seg2 = new QGraphicsLineItem(QLineF(cornerPos, currentPos));
        seg1->setPen(tempPen);
        seg2->setPen(tempPen);
        scene()->addItem(seg1);
        scene()->addItem(seg2);
        m_tempPreviewSegments.append(seg1);
        m_tempPreviewSegments.append(seg2);
    }
    QGraphicsView::mouseMoveEvent(event);
}

// Handles mouse clicks to start, create corners, or end wires
void SchematicEditor::mousePressEvent(QMouseEvent *event)
{
    // Cancel wiring with a right-click at any point during the process
    if (event->button() == Qt::RightButton && m_wiringState == WiringState::DrawingWire) {
        toggleWiringMode(false);
        return;
    }

    if (m_wiringState == WiringState::NotWiring) {
        // State before starting a wire.
        if (event->button() == Qt::LeftButton) {
            if (auto startTerminal = getTerminalAt(event->pos())) {
                m_currentWire = new PolylineWireItem(startTerminal);
                scene()->addItem(m_currentWire);
                m_wiringState = WiringState::DrawingWire;
            }
        }
    } else if (m_wiringState == WiringState::DrawingWire) {
        // State while a wire is being drawn.
        if (event->button() == Qt::LeftButton) {
            QPointF snappedPos = snapToGrid(mapToScene(event->pos()));

            if (auto endTerminal = getTerminalAt(event->pos())) {
                // Case 1: Finish the wire on a terminal.
                if (endTerminal != m_currentWire->getStartTerminal()) {
                    QPointF lastP = m_currentWire->lastPoint();
                    m_currentWire->addPoint(QPointF(snappedPos.x(), lastP.y()));
                    m_currentWire->setEndTerminal(endTerminal);
                    registerLogicalConnection(m_currentWire->getStartTerminal(), endTerminal);
                }
                m_currentWire = nullptr;
                m_wiringState = WiringState::NotWiring; // Exit drawing mode.
            } else if (auto clickedWire = getWireAt(event->pos())) {
                // Case 2: Finish on another wire, creating a junction.
                scene()->addItem(new JunctionItem(snappedPos));
                auto junctionTerminal = new TerminalItem(nullptr, -1); // Dummy terminal for the junction
                junctionTerminal->setPos(snappedPos);
                scene()->addItem(junctionTerminal);

                QPointF lastP = m_currentWire->lastPoint();
                m_currentWire->addPoint(QPointF(snappedPos.x(), lastP.y()));
                m_currentWire->setEndTerminal(junctionTerminal);

                // Register logical connections for the new node
                registerLogicalConnection(m_currentWire->getStartTerminal(), junctionTerminal);
                registerLogicalConnection(clickedWire->getStartTerminal(), junctionTerminal);

                m_currentWire = nullptr;
                m_wiringState = WiringState::NotWiring;
            } else {
                // Case 3: Add a corner point in empty space.
                QPointF lastP = m_currentWire->lastPoint();
                m_currentWire->addPoint(QPointF(snappedPos.x(), lastP.y()));
                m_currentWire->addPoint(snappedPos);
            }
        }
    }

    // Clear the temporary preview segments after any click.
    for(auto segment : m_tempPreviewSegments) {
        scene()->removeItem(segment);
        delete segment;
    }
    m_tempPreviewSegments.clear();

    QGraphicsView::mousePressEvent(event);
}

void SchematicEditor::mouseReleaseEvent(QMouseEvent *event)
{
    QGraphicsView::mouseReleaseEvent(event);
}

// Manages the logical connection between terminals
void SchematicEditor::registerLogicalConnection(TerminalItem* term1, TerminalItem* term2)
{
    // This function can be further improved, but the core logic is here.
    int node1_idx = -1, node2_idx = -1;
    for (size_t i = 0; i < m_logicalNodes.size(); ++i) {
        if (m_logicalNodes[i].count(term1)) node1_idx = i;
        if (m_logicalNodes[i].count(term2)) node2_idx = i;
    }
    if (node1_idx != -1 && node2_idx != -1) {
        if (node1_idx != node2_idx) {
            m_logicalNodes[node1_idx].insert(m_logicalNodes[node2_idx].begin(), m_logicalNodes[node2_idx].end());
            m_logicalNodes.erase(m_logicalNodes.begin() + node2_idx);
        }
    } else if (node1_idx != -1) {
        m_logicalNodes[node1_idx].insert(term2);
    } else if (node2_idx != -1) {
        m_logicalNodes[node2_idx].insert(term1);
    } else {
        m_logicalNodes.push_back({term1, term2});
    }
    qDebug() << "Logical connection registered.";
}

#include "schematiceditor.h"
#include <QPainter>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QDebug>
#include <QKeyEvent>
#include <QSet>
#include <cmath>
#include "terminalitem.h"
#include "polylinewireitem.h"
#include "junctionitem.h"
#include "componentitem.h"
#include "Circuit.h"
#include "grounditem.h"
#include "resistoritem.h"
#include "capacitoritem.h"
#include "inductoritem.h"
#include "voltagesourceitem.h"
#include "currentsourceitem.h"
#include "dependentsourceitems.h"
#include "SubCircuit.h"
#include "SubCircuitItem.h"


SchematicEditor::SchematicEditor(Circuit* circuit, QWidget *parent)
        : QGraphicsView(parent), m_circuit(circuit)
{
    setScene(new QGraphicsScene(this));
    scene()->setSceneRect(-5000, -5000, 10000, 10000);
    setRenderHint(QPainter::Antialiasing);
    setMouseTracking(true);
    m_wiringState = WiringState::NotWiring;
}

void SchematicEditor::populateSceneFromCircuit()
{
    scene()->clear();
    m_logicalNodes.clear();
    if (!m_circuit) return;

    std::map<std::string, ComponentItem*> componentItemMap;

    const auto& components = m_circuit->getComponents();
    for (const auto& logic_comp_ptr : components) {
        Component* logic_comp = logic_comp_ptr.get();
        if (!logic_comp) continue;

        ComponentItem* newItem = nullptr;

        if (auto r = dynamic_cast<Resistor*>(logic_comp)) { newItem = new ResistorItem(r); }
        else if (auto c = dynamic_cast<Capacitor*>(logic_comp)) { newItem = new CapacitorItem(c); }
        else if (auto l = dynamic_cast<Inductor*>(logic_comp)) { newItem = new InductorItem(l); }
        else if (auto vcvs = dynamic_cast<VCVS*>(logic_comp)) { newItem = new VCVSItem(vcvs); }
        else if (auto vccs = dynamic_cast<VCCS*>(logic_comp)) { newItem = new VCCSItem(vccs); }
        else if (auto ccvs = dynamic_cast<CCVS*>(logic_comp)) { newItem = new CCCSItem(ccvs); }
        else if (auto cccs = dynamic_cast<CCCS*>(logic_comp)) { newItem = new CCCSItem(cccs); }
        else if (auto is = dynamic_cast<CurrentSource*>(logic_comp)) { newItem = new CurrentSourceItem(is); }
        else if (auto gnd = dynamic_cast<Ground*>(logic_comp)) { newItem = new GroundItem(gnd); }
        else if (auto sub = dynamic_cast<SubCircuit*>(logic_comp)) { newItem = new SubCircuitItem(sub); }
        else if (auto vs = dynamic_cast<VoltageSource*>(logic_comp)) { newItem = new VoltageSourceItem(vs); }


        if (newItem) {
            newItem->setPos(logic_comp->getPosition());
            scene()->addItem(newItem);
            componentItemMap[logic_comp->getName()] = newItem;
        }
    }

    for (const auto& wireInfo : m_circuit->getWires()) {
        if (componentItemMap.count(wireInfo.startCompName) && componentItemMap.count(wireInfo.endCompName)) {
            ComponentItem* startCompItem = componentItemMap.at(wireInfo.startCompName);
            TerminalItem* startTerm = (wireInfo.startTerminalId == 0) ? startCompItem->terminal1() : startCompItem->terminal2();

            ComponentItem* endCompItem = componentItemMap.at(wireInfo.endCompName);
            TerminalItem* endTerm = (wireInfo.endTerminalId == 0) ? endCompItem->terminal1() : endCompItem->terminal2();

            if (startTerm && endTerm) {
                auto newWire = new PolylineWireItem(startTerm);
                for (size_t i = 1; i < wireInfo.points.size(); ++i) {
                    const auto& p = wireInfo.points[i];
                    newWire->addPoint(QPointF(p.x, p.y));
                }
                newWire->setEndTerminal(endTerm);
                scene()->addItem(newWire);
                registerLogicalConnection(startTerm, endTerm);
            }
        }
    }
}

void SchematicEditor::updateCircuitWires()
{
    if (!m_circuit) return;
    m_circuit->getWires().clear();
    for (QGraphicsItem* item : scene()->items()) {
        if (auto wireItem = dynamic_cast<PolylineWireItem*>(item)) {
            WireInfo info;
            TerminalItem* startTerm = wireItem->getStartTerminal();
            TerminalItem* endTerm = wireItem->getEndTerminal();
            if (startTerm && endTerm && startTerm->getParentComponent() && endTerm->getParentComponent()) {
                info.startCompName = startTerm->getParentComponent()->getComponent()->getName();
                info.startTerminalId = startTerm->getId();
                info.endCompName = endTerm->getParentComponent()->getComponent()->getName();
                info.endTerminalId = endTerm->getId();
                for(const QPointF& p : wireItem->getPoints()) {
                    info.points.push_back({p.x(), p.y()});
                }
                m_circuit->getWires().push_back(info);
            }
        }
    }
}

void SchematicEditor::updateBackendNodes()
{
    if (!m_circuit) return;
    std::map<TerminalItem*, int> terminalNodeMap;
    int nextNodeId = 1;
    for (const auto& nodeSet : m_logicalNodes) {
        bool isGroundNode = false;
        for (TerminalItem* term : nodeSet) {
            if (dynamic_cast<GroundItem*>(term->getParentComponent())) {
                isGroundNode = true;
                break;
            }
        }
        int currentNodeId = isGroundNode ? 0 : nextNodeId++;
        for (TerminalItem* term : nodeSet) {
            terminalNodeMap[term] = currentNodeId;
        }
    }
    for (QGraphicsItem* item : scene()->items()) {
        if (auto compItem = dynamic_cast<ComponentItem*>(item)) {
            if (auto logicComp = compItem->getComponent()) {
                TerminalItem* t1 = compItem->terminal1();
                TerminalItem* t2 = compItem->terminal2();
                int n1 = terminalNodeMap.count(t1) ? terminalNodeMap.at(t1) : -1;
                int n2 = (t2 && t2->isVisible()) ? (terminalNodeMap.count(t2) ? terminalNodeMap.at(t2) : -1) : n1;

                logicComp->setNodes({n1, n2});
            }
        }
    }
    qDebug() << "Backend nodes updated!";
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

void SchematicEditor::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton && m_wiringState == WiringState::DrawingWire) {
        cancelWiring();
        return;
    }

    if (m_wiringState == WiringState::NotWiring) {
        if (event->button() == Qt::LeftButton) {
            if (auto startTerminal = getTerminalAt(event->pos())) {
                m_currentWire = new PolylineWireItem(startTerminal);
                scene()->addItem(m_currentWire);
                m_wiringState = WiringState::DrawingWire;
            }
        }
    } else if (m_wiringState == WiringState::DrawingWire) {
        if (event->button() == Qt::LeftButton) {
            QPointF snappedPos = snapToGrid(mapToScene(event->pos()));
            if (auto endTerminal = getTerminalAt(event->pos())) {
                if (endTerminal != m_currentWire->getStartTerminal()) {
                    TerminalItem* startTerm = m_currentWire->getStartTerminal();
                    QPointF lastP = m_currentWire->lastPoint();
                    m_currentWire->addPoint(QPointF(snappedPos.x(), lastP.y()));
                    m_currentWire->setEndTerminal(endTerminal);

                    m_currentWire = nullptr;
                    m_wiringState = WiringState::NotWiring;

                    registerLogicalConnection(startTerm, endTerminal);
                } else {
                    cancelWiring();
                }
            } else {
                QPointF lastP = m_currentWire->lastPoint();
                m_currentWire->addPoint(QPointF(snappedPos.x(), lastP.y()));
                m_currentWire->addPoint(snappedPos);
            }
        }
    }

    clearPreviewSegments();
    QGraphicsView::mousePressEvent(event);
}

void SchematicEditor::mouseMoveEvent(QMouseEvent *event)
{
    if (m_wiringState == WiringState::DrawingWire) {
        clearPreviewSegments();

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

void SchematicEditor::toggleWiringMode(bool enabled)
{
    if (!enabled) {
        cancelWiring();
    }
}

void SchematicEditor::cancelWiring()
{
    if (m_currentWire) {
        scene()->removeItem(m_currentWire);
        delete m_currentWire;
        m_currentWire = nullptr;
    }
    clearPreviewSegments();
    m_wiringState = WiringState::NotWiring;
}

void SchematicEditor::clearPreviewSegments()
{
    for(auto segment : m_tempPreviewSegments) {
        scene()->removeItem(segment);
        delete segment;
    }
    m_tempPreviewSegments.clear();
}

void SchematicEditor::registerLogicalConnection(TerminalItem* term1, TerminalItem* term2)
{
    if (!term1 || !term2) return;
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
    qDebug() << "Logical connection registered. Total nodes:" << m_logicalNodes.size();
}

TerminalItem* SchematicEditor::getTerminalAt(const QPoint& pos)
{
    for (QGraphicsItem* item : items(pos)) {
        if (auto terminal = dynamic_cast<TerminalItem*>(item)) {
            return terminal;
        }
    }
    return nullptr;
}

QPointF SchematicEditor::snapToGrid(const QPointF& pos)
{
    const int gridSize = 20;
    qreal x = round(pos.x() / gridSize) * gridSize;
    qreal y = round(pos.y() / gridSize) * gridSize;
    return QPointF(x, y);
}

void SchematicEditor::mouseReleaseEvent(QMouseEvent *event)
{
    QGraphicsView::mouseReleaseEvent(event);
}

void SchematicEditor::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete) {
        QList<QGraphicsItem*> selected = scene()->selectedItems();
        if (selected.isEmpty()) {
            QGraphicsView::keyPressEvent(event);
            return;
        }

        QSet<PolylineWireItem*> wiresToDelete;

        for (QGraphicsItem* item : selected) {
            if (auto compItem = qgraphicsitem_cast<ComponentItem*>(item)) {
                for (auto wire : compItem->terminal1()->getWires()) {
                    wiresToDelete.insert(wire);
                }
                if (compItem->terminal2()->isVisible()) {
                    for (auto wire : compItem->terminal2()->getWires()) {
                        wiresToDelete.insert(wire);
                    }
                }
            } else if (auto wireItem = qgraphicsitem_cast<PolylineWireItem*>(item)) {
                wiresToDelete.insert(wireItem);
            }
        }

        for (PolylineWireItem* wire : wiresToDelete) {
            scene()->removeItem(wire);
            delete wire;
        }

        for (QGraphicsItem* item : selected) {
            if (auto compItem = qgraphicsitem_cast<ComponentItem*>(item)) {
                if (compItem->getComponent()) {
                    m_circuit->removeComponent(compItem->getComponent()->getName());
                }
                scene()->removeItem(compItem);
                delete compItem;
            }
        }
    } else {
        QGraphicsView::keyPressEvent(event);
    }
}

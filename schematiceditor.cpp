#include "schematiceditor.h"
#include "mainwindow.h"
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
#include "nodelabelitem.h"


SchematicEditor::SchematicEditor(Circuit* circuit, QWidget *parent)
        : QGraphicsView(parent), m_circuit(circuit)
{
    setScene(new QGraphicsScene(this));
    scene()->setBackgroundBrush(QColor(40, 40, 40));
    scene()->setSceneRect(-5000, -5000, 10000, 10000);
    setRenderHint(QPainter::Antialiasing);
    setMouseTracking(true);
    m_editorState = EditorState::Normal;
}

void SchematicEditor::setEditorMode(EditorState newState) {
    if (m_editorState == EditorState::Wiring && newState != EditorState::Wiring) {
        cancelWiring();
    }
    m_editorState = newState;
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

TerminalItem* SchematicEditor::getTerminalNear(const QPointF& scenePos) {
    double minDistance = 15.0;
    TerminalItem* closestTerminal = nullptr;

    for (QGraphicsItem* item : items(QRectF(scenePos - QPointF(minDistance, minDistance), QSizeF(minDistance*2, minDistance*2)).toRect())) {
        if (auto term = dynamic_cast<TerminalItem*>(item)) {
            double dist = QLineF(scenePos, term->scenePos()).length();
            if (dist < minDistance) {
                minDistance = dist;
                closestTerminal = term;
            }
        }
    }
    return closestTerminal;
}


void SchematicEditor::updateBackendNodes()
{
    if (!m_circuit) return;
    std::map<std::string, std::set<TerminalItem*>> labelMap;

    for (QGraphicsItem* item : scene()->items()) {
        if (auto label = dynamic_cast<NodeLabelItem*>(item)) {
            std::string labelName = label->toPlainText().toStdString();
            TerminalItem* attachedTerminal = getTerminalNear(label->scenePos());
            if (attachedTerminal) {
                for (const auto& nodeSet : m_logicalNodes) {
                    if (nodeSet.count(attachedTerminal)) {
                        labelMap[labelName].insert(nodeSet.begin(), nodeSet.end());
                        break;
                    }
                }
            }
        }
    }

    std::vector<std::set<TerminalItem*>> finalNodes = m_logicalNodes;
    for (const auto& pair : labelMap) {
        const auto& terminalsToMerge = pair.second;
        if (terminalsToMerge.size() <= 1) continue;

        std::set<int> indicesOfSetsToMerge;
        for (TerminalItem* term : terminalsToMerge) {
            for (size_t i = 0; i < finalNodes.size(); ++i) {
                if (finalNodes[i].count(term)) {
                    indicesOfSetsToMerge.insert(i);
                }
            }
        }

        if (indicesOfSetsToMerge.size() > 1) {
            int targetIndex = *indicesOfSetsToMerge.begin();
            auto it = std::next(indicesOfSetsToMerge.begin());
            for (; it != indicesOfSetsToMerge.end(); ++it) {
                finalNodes[targetIndex].insert(finalNodes[*it].begin(), finalNodes[*it].end());
                finalNodes[*it].clear();
            }
        }
    }
    finalNodes.erase(std::remove_if(finalNodes.begin(), finalNodes.end(), [](const auto& s){ return s.empty(); }), finalNodes.end());

    std::map<TerminalItem*, int> terminalNodeMap;
    int nextNodeId = 1;
    for (const auto& nodeSet : finalNodes) {
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
    qDebug() << "Backend nodes updated! Final node count:" << finalNodes.size();
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
    switch (m_editorState)
    {
        case EditorState::Normal:
            if (event->button() == Qt::LeftButton) {
                if (auto startTerminal = getTerminalAt(event->pos())) {
                    m_currentWire = new PolylineWireItem(startTerminal);
                    scene()->addItem(m_currentWire);
                    m_editorState = EditorState::Wiring;
                } else {
                    QGraphicsView::mousePressEvent(event);
                }
            } else {
                QGraphicsView::mousePressEvent(event);
            }
            break;

        case EditorState::Wiring:
            if (event->button() == Qt::RightButton) {
                cancelWiring();
                return;
            }

            if (event->button() == Qt::LeftButton) {
                QPointF snappedPos = snapToGrid(mapToScene(event->pos()));
                if (auto endTerminal = getTerminalAt(event->pos())) {
                    if (endTerminal != m_currentWire->getStartTerminal()) {
                        TerminalItem* startTerm = m_currentWire->getStartTerminal();
                        QPointF lastP = m_currentWire->lastPoint();
                        m_currentWire->addPoint(QPointF(snappedPos.x(), lastP.y()));
                        m_currentWire->setEndTerminal(endTerminal);

                        registerLogicalConnection(startTerm, endTerminal);

                        m_currentWire = nullptr;
                        m_editorState = EditorState::Normal;
                    } else {
                        cancelWiring();
                    }
                } else {
                    QPointF lastP = m_currentWire->lastPoint();
                    m_currentWire->addPoint(QPointF(snappedPos.x(), lastP.y()));
                    m_currentWire->addPoint(snappedPos);
                }
            }
            break;

        case EditorState::Probing:
            if (event->button() == Qt::LeftButton) {
                QGraphicsItem* item = itemAt(event->pos());
                if (auto compItem = dynamic_cast<ComponentItem*>(item)) {
                    string varName = "I(" + compItem->getComponent()->getName() + ")";
                    if (m_mainWindow) m_mainWindow->plotVariable(QString::fromStdString(varName));
                } else {
                    updateBackendNodes();
                    QPointF scenePos = mapToScene(event->pos());
                    int nodeId = findNodeAt(scenePos);
                    if (nodeId > 0) {
                        string varName = "V(" + to_string(nodeId) + ")";
                        if (m_mainWindow) m_mainWindow->plotVariable(QString::fromStdString(varName));
                    }
                }
            }
            return;
    }

    clearPreviewSegments();
}


void SchematicEditor::mouseMoveEvent(QMouseEvent *event)
{
    if (m_editorState == EditorState::Wiring) {
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
    m_editorState = EditorState::Normal;
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

int SchematicEditor::findNodeAt(const QPointF& scenePos) {
    TerminalItem* closestTerminal = nullptr;
    double minDistance = 100.0;

    for (QGraphicsItem* item : scene()->items()) {
        if (auto term = dynamic_cast<TerminalItem*>(item)) {
            double dist = QLineF(scenePos, term->scenePos()).length();
            if (dist < minDistance) {
                minDistance = dist;
                closestTerminal = term;
            }
        }
    }

    if (closestTerminal && minDistance < 20.0) {
        for (const auto& nodeSet : m_logicalNodes) {
            if (nodeSet.count(closestTerminal)) {
                Component* comp = closestTerminal->getParentComponent()->getComponent();
                int termId = closestTerminal->getId();
                return comp->getNode(termId);
            }
        }
    }
    return -1;
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

void SchematicEditor::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        if (m_editorState == EditorState::Wiring) {
            cancelWiring();
            return;
        }
    }

    if (m_editorState == EditorState::Normal) {
        if (m_mainWindow) {
            switch (event->key()) {
                case Qt::Key_R: m_mainWindow->onAddResistor(); return;
                case Qt::Key_C: m_mainWindow->onAddCapacitor(); return;
                case Qt::Key_L: m_mainWindow->onAddInductor(); return;
                case Qt::Key_V: m_mainWindow->onAddVoltageSource(); return;
                case Qt::Key_I: m_mainWindow->onAddCurrentSource(); return;
                case Qt::Key_G: m_mainWindow->onAddGround(); return;
            }
        }
    }

    if (event->key() == Qt::Key_Delete) {
        const QList<QGraphicsItem*> selectedItems = scene()->selectedItems();
        if (selectedItems.isEmpty()) {
            QGraphicsView::keyPressEvent(event);
            return;
        }
        QSet<PolylineWireItem*> wiresToDelete;
        QList<ComponentItem*> componentsToDelete;
        QList<NodeLabelItem*> labelsToDelete;
        for (QGraphicsItem* item : selectedItems) {
            if (auto compItem = qgraphicsitem_cast<ComponentItem*>(item)) {
                componentsToDelete.append(compItem);
                if (compItem->terminal1()) {
                    for (auto wire : compItem->terminal1()->getWires()) wiresToDelete.insert(wire);
                }
                if (compItem->terminal2() && compItem->terminal2()->isVisible()) {
                    for (auto wire : compItem->terminal2()->getWires()) wiresToDelete.insert(wire);
                }
            } else if (auto wireItem = qgraphicsitem_cast<PolylineWireItem*>(item)) {
                wiresToDelete.insert(wireItem);
            } else if (auto labelItem = qgraphicsitem_cast<NodeLabelItem*>(item)) {
                labelsToDelete.append(labelItem);
            }
        }


        for (PolylineWireItem* wire : wiresToDelete) delete wire;
        for (ComponentItem* comp : componentsToDelete) {
            if (comp->getComponent()) m_circuit->removeComponent(comp->getComponent()->getName());
            delete comp;
        }
        for (NodeLabelItem* label : labelsToDelete) delete label;

        return;
    }
    QGraphicsView::keyPressEvent(event);
}

void SchematicEditor::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (m_editorState == EditorState::Wiring) {
        QPointF snappedPos = snapToGrid(mapToScene(event->pos()));
        QPointF lastP = m_currentWire->lastPoint();
        m_currentWire->addPoint(QPointF(snappedPos.x(), lastP.y()));
        m_currentWire->addPoint(snappedPos);

        m_currentWire = nullptr;
        m_editorState = EditorState::Normal;
        clearPreviewSegments();
        event->accept();
    } else {
        QGraphicsView::mouseDoubleClickEvent(event);
    }
}
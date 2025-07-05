#ifndef SCHEMATICEDITOR_H
#define SCHEMATICEDITOR_H

#include <QGraphicsView>
#include <vector>
#include <set>
#include <QList>
#include <map>

class TerminalItem;
class PolylineWireItem;
class JunctionItem;
class QGraphicsLineItem;
class QMouseEvent;
class Circuit;
class ComponentItem;

class SchematicEditor : public QGraphicsView
{
Q_OBJECT

public:
    explicit SchematicEditor(Circuit* circuit, QWidget *parent = nullptr);

    void populateSceneFromCircuit();
    void updateCircuitWires();
    void updateBackendNodes();

public slots:
    void toggleWiringMode(bool enabled);

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    enum class WiringState {
        NotWiring,
        DrawingWire
    };

    WiringState m_wiringState;
    Circuit* m_circuit;
    PolylineWireItem *m_currentWire = nullptr;
    QList<QGraphicsLineItem*> m_tempPreviewSegments;
    std::vector<std::set<TerminalItem*>> m_logicalNodes;

    TerminalItem* getTerminalAt(const QPoint& pos);
    QPointF snapToGrid(const QPointF& pos);
    void registerLogicalConnection(TerminalItem* term1, TerminalItem* term2);

    void cancelWiring();
    void clearPreviewSegments();
};

#endif // SCHEMATICEDITOR_H

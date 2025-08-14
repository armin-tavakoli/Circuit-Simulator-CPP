#ifndef SCHEMATICEDITOR_H
#define SCHEMATICEDITOR_H

#include <QGraphicsView>
#include <vector>
#include <set>
#include <QList>
#include <map>
#include <QKeyEvent>

// Forward declarations
class TerminalItem;
class PolylineWireItem;
class JunctionItem;
class QGraphicsLineItem;
class QMouseEvent;
class Circuit;
class ComponentItem;
class MainWindow; // <-- Forward declaration for MainWindow

class SchematicEditor : public QGraphicsView
{
Q_OBJECT

public:
    enum class EditorState { Normal, Wiring, Probing };
    explicit SchematicEditor(Circuit* circuit, QWidget *parent = nullptr);
    void populateSceneFromCircuit();
    void updateCircuitWires();
    void updateBackendNodes();
    void setMainWindow(MainWindow* window) { m_mainWindow = window; }
    void setEditorMode(EditorState newState);

public slots:
    void toggleWiringMode(bool enabled);

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:

    EditorState m_editorState;
    Circuit* m_circuit;
    MainWindow* m_mainWindow = nullptr;
    PolylineWireItem *m_currentWire = nullptr;
    QList<QGraphicsLineItem*> m_tempPreviewSegments;
    std::vector<std::set<TerminalItem*>> m_logicalNodes;

    TerminalItem* getTerminalAt(const QPoint& pos);
    TerminalItem* getTerminalNear(const QPointF& scenePos);
    int findNodeAt(const QPointF& scenePos);
    QPointF snapToGrid(const QPointF& pos);
    void registerLogicalConnection(TerminalItem* term1, TerminalItem* term2);
    void cancelWiring();
    void clearPreviewSegments();
};

#endif // SCHEMATICEDITOR_H

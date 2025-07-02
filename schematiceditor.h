#ifndef SCHEMATICEDITOR_H
#define SCHEMATICEDITOR_H

#include <QGraphicsView>
#include <vector>
#include <set>
#include <QList>

// Forward declarations
class TerminalItem;
class PolylineWireItem;
class JunctionItem;
class QGraphicsLineItem;
class QMouseEvent;

class SchematicEditor : public QGraphicsView
{
Q_OBJECT

public:
    explicit SchematicEditor(QWidget *parent = nullptr);

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

    PolylineWireItem *m_currentWire = nullptr;
    QList<QGraphicsLineItem*> m_tempPreviewSegments;

    std::vector<std::set<TerminalItem*>> m_logicalNodes;

    // Helper functions
    TerminalItem* getTerminalAt(const QPoint& pos);
    PolylineWireItem* getWireAt(const QPoint& pos); // <<< این خط اضافه شد
    QPointF snapToGrid(const QPointF& pos);
    void registerLogicalConnection(TerminalItem* term1, TerminalItem* term2);
};

#endif // SCHEMATICEDITOR_H

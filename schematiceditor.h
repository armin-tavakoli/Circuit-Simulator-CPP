#ifndef SCHEMATICEDITOR_H
#define SCHEMATICEDITOR_H

#include <QGraphicsView>
#include <vector>
#include <set>
#include <QList>
#include <map> // برای نگاشت پایانه‌ها به شماره گره

// Forward declarations
class TerminalItem;
class PolylineWireItem;
class JunctionItem;
class QGraphicsLineItem;
class QMouseEvent;
class Circuit; // <<<
class ComponentItem; // <<<

class SchematicEditor : public QGraphicsView
{
Q_OBJECT

public:
    // <<< سازنده تغییر می‌کند >>>
    explicit SchematicEditor(Circuit* circuit, QWidget *parent = nullptr);

    // <<< تابع عمومی جدید برای فراخوانی از بیرون >>>
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
    Circuit* m_circuit; // <<< اشاره‌گر به بک‌اند

    PolylineWireItem *m_currentWire = nullptr;
    QList<QGraphicsLineItem*> m_tempPreviewSegments;

    std::vector<std::set<TerminalItem*>> m_logicalNodes;

    // Helper functions
    TerminalItem* getTerminalAt(const QPoint& pos);
    PolylineWireItem* getWireAt(const QPoint& pos);
    QPointF snapToGrid(const QPointF& pos);
    void registerLogicalConnection(TerminalItem* term1, TerminalItem* term2);
};

#endif // SCHEMATICEDITOR_H

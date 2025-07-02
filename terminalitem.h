#ifndef TERMINALITEM_H
#define TERMINALITEM_H

#include <QGraphicsItem>
#include <QList>

// Forward declarations
class ComponentItem;
class PolylineWireItem;

class TerminalItem : public QGraphicsItem
{
public:
    explicit TerminalItem(ComponentItem *parent, int terminalId);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    // Methods to manage connected wires
    void addWire(PolylineWireItem *wire);
    void removeWire(PolylineWireItem *wire);
    const QList<PolylineWireItem*>& getWires() const;

    // Method to notify connected wires to update their position
    void updateConnectedWires();

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

private:
    ComponentItem *parentComponent;
    int id;
    bool isHovered = false;

    // List of connected wires
    QList<PolylineWireItem*> m_wires;
};

#endif // TERMINALITEM_H

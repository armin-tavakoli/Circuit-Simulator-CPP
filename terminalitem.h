#ifndef TERMINALITEM_H
#define TERMINALITEM_H

#include <QGraphicsItem>
#include <QList>

class ComponentItem;
class PolylineWireItem;

class TerminalItem : public QGraphicsItem
{
public:
    explicit TerminalItem(ComponentItem *parent, int terminalId);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void addWire(PolylineWireItem *wire);
    void removeWire(PolylineWireItem *wire);
    const QList<PolylineWireItem*>& getWires() const;
    void updateConnectedWires();

    // --- توابع getter جدید ---
    ComponentItem* getParentComponent() const;
    int getId() const;

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

private:
    ComponentItem *parentComponent;
    int id;
    bool isHovered = false;
    QList<PolylineWireItem*> m_wires;
};

#endif
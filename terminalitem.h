#ifndef TERMINALITEM_H
#define TERMINALITEM_H

#include <QGraphicsItem>

class ComponentItem;

class TerminalItem : public QGraphicsItem
{
public:
    explicit TerminalItem(ComponentItem *parent, int terminalId);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

private:
    ComponentItem *parentComponent;
    int id;
    bool isHovered = false;
};

#endif
#ifndef COMPONENTITEM_H
#define COMPONENTITEM_H

#include <QGraphicsObject>
#include "Component.h"

class TerminalItem;

class ComponentItem : public QGraphicsObject
{
    Q_OBJECT

public:
    explicit ComponentItem(Component* component, QGraphicsItem *parent = nullptr);
    virtual ~ComponentItem() = default;

    virtual QRectF boundingRect() const;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    Component* getComponent() const;
    TerminalItem* terminal1() const { return m_terminal1; }
    TerminalItem* terminal2() const { return m_terminal2; }

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    TerminalItem *m_terminal1;
    TerminalItem *m_terminal2;

private:
    Component* logicalComponent;
};

#endif
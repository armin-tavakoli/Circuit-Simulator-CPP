#ifndef INDUCTORITEM_H
#define INDUCTORITEM_H

#include "componentitem.h"

class InductorItem : public ComponentItem
{
public:
    explicit InductorItem(Component* component, QGraphicsItem *parent = nullptr);
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
};

#endif
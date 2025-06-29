#ifndef RESISTORITEM_H
#define RESISTORITEM_H

#include "componentitem.h"

class ResistorItem : public ComponentItem
{
public:
    explicit ResistorItem(Component* component, QGraphicsItem *parent = nullptr);
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
};

#endif
#ifndef CAPACITORITEM_H
#define CAPACITORITEM_H

#include "componentitem.h"

class CapacitorItem : public ComponentItem
{
public:
    explicit CapacitorItem(Component* component, QGraphicsItem *parent = nullptr);
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
};

#endif
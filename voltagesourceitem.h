#ifndef VOLTAGESOURCEITEM_H
#define VOLTAGESOURCEITEM_H

#include "componentitem.h"

class VoltageSourceItem : public ComponentItem
{
public:
    explicit VoltageSourceItem(Component* component, QGraphicsItem *parent = nullptr);
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
};

#endif
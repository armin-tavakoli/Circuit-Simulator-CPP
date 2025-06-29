#ifndef CURRENTSOURCEITEM_H
#define CURRENTSOURCEITEM_H

#include "componentitem.h"

class CurrentSourceItem : public ComponentItem
{
public:
    explicit CurrentSourceItem(Component* component, QGraphicsItem *parent = nullptr);
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
};

#endif
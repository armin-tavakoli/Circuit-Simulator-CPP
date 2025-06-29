#ifndef GROUNDITEM_H
#define GROUNDITEM_H

#include "componentitem.h"

class GroundItem : public ComponentItem
{
public:
    explicit GroundItem(QGraphicsItem *parent = nullptr);
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
};

#endif
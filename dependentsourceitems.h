#ifndef DEPENDENTSOURCEITEMS_H
#define DEPENDENTSOURCEITEMS_H

#include "componentitem.h"

class DependentSourceItemBase : public ComponentItem
{
public:
    using ComponentItem::ComponentItem;
    QRectF boundingRect() const override;

protected:
    void drawBase(QPainter *painter);
};

class VCVSItem : public DependentSourceItemBase
{
public:
    using DependentSourceItemBase::DependentSourceItemBase;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
};

class VCCSItem : public DependentSourceItemBase
{
public:
    using DependentSourceItemBase::DependentSourceItemBase;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
};

class CCVSItem : public DependentSourceItemBase
{
public:
    using DependentSourceItemBase::DependentSourceItemBase;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
};

class CCCSItem : public DependentSourceItemBase
{
public:
    using DependentSourceItemBase::DependentSourceItemBase;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
};

#endif
#ifndef SUBCIRCUITITEM_H
#define SUBCIRCUITITEM_H

#include "componentitem.h"
#include <QMenu> // برای منوی راست‌کلیک

class SubCircuitItem : public ComponentItem
{
Q_OBJECT

public:
    explicit SubCircuitItem(Component* component, QGraphicsItem *parent = nullptr);
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

protected:
    // بازنویسی تابع برای مدیریت راست‌کلیک
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

private slots:
    void onEditSubcircuit(); // اسلات برای ویرایش
};

#endif // SUBCIRCUITITEM_H

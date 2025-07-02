#ifndef JUNCTIONITEM_H
#define JUNCTIONITEM_H

#include <QGraphicsEllipseItem>
#include <QPainter>

class JunctionItem : public QGraphicsEllipseItem
{
public:
    JunctionItem(const QPointF& pos, QGraphicsItem *parent = nullptr)
            : QGraphicsEllipseItem(QRectF(pos - QPointF(4,4), QSizeF(8,8)), parent)
    {
        // A small green circle to represent the junction
        setBrush(Qt::green);
        setPen(Qt::NoPen);
        setZValue(2); // Ensure it draws on top of wires
    }
};

#endif // JUNCTIONITEM_H

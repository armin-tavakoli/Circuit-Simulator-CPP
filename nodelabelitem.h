#ifndef NODELABELITEM_H
#define NODELABELITEM_H

#include <QGraphicsTextItem>

class NodeLabelItem : public QGraphicsTextItem
{
public:
    explicit NodeLabelItem(const QString &text, QGraphicsItem *parent = nullptr);

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
};

#endif // NODELABELITEM_H
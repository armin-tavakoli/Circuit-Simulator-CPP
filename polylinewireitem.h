#ifndef POLYLINEWIREITEM_H
#define POLYLINEWIREITEM_H

#include <QGraphicsObject>
#include <QList>
#include <QPointF>

class TerminalItem;

class PolylineWireItem : public QGraphicsObject
{
public:
    PolylineWireItem(TerminalItem *startTerminal, QGraphicsItem *parent = nullptr);
    virtual ~PolylineWireItem();

    void addPoint(const QPointF &point);
    void setEndTerminal(TerminalItem *endTerminal);
    void updatePosition();

    TerminalItem* getStartTerminal() const { return m_startTerminal; }
    TerminalItem* getEndTerminal() const { return m_endTerminal; }
    QPointF lastPoint() const;

    // --- تابع getter جدید ---
    const QList<QPointF>& getPoints() const;

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    TerminalItem *m_startTerminal;
    TerminalItem *m_endTerminal;
    QList<QPointF> m_points;
};

#endif
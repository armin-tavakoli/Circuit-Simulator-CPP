#include "schematiceditor.h"
#include <QPainter>
#include <QGraphicsScene>

SchematicEditor::SchematicEditor(QWidget *parent)
        : QGraphicsView(parent)
{
    setScene(new QGraphicsScene(this));
    scene()->setSceneRect(-5000, -5000, 10000, 10000);

    setRenderHint(QPainter::Antialiasing);
}

void SchematicEditor::drawBackground(QPainter *painter, const QRectF &rect)
{
    QGraphicsView::drawBackground(painter, rect);

    const int gridSize = 20;
    QPen pen(Qt::darkGray, 0);
    pen.setStyle(Qt::DotLine);
    painter->setPen(pen);

    qreal left = int(rect.left()) - (int(rect.left()) % gridSize);
    for (qreal x = left; x < rect.right(); x += gridSize)
        painter->drawLine(QPointF(x, rect.top()), QPointF(x, rect.bottom()));

    qreal top = int(rect.top()) - (int(rect.top()) % gridSize);
    for (qreal y = top; y < rect.bottom(); y += gridSize)
        painter->drawLine(QPointF(rect.left(), y), QPointF(rect.right(), y));
}

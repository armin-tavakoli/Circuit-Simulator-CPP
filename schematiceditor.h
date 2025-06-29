#ifndef SCHEMATICEDITOR_H
#define SCHEMATICEDITOR_H

#include <QGraphicsView>

class SchematicEditor : public QGraphicsView
{
    Q_OBJECT

public:
    explicit SchematicEditor(QWidget *parent = nullptr);

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;
};

#endif
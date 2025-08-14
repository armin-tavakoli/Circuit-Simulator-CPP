#include "nodelabelitem.h"
#include <QInputDialog>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>

NodeLabelItem::NodeLabelItem(const QString &text, QGraphicsItem *parent)
        : QGraphicsTextItem(text, parent)
{
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setDefaultTextColor(Qt::yellow);
}

void NodeLabelItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    QString currentText = toPlainText();
    bool ok;
    QString newText = QInputDialog::getText(event->widget(), tr("Edit Label"),
                                            tr("Label name:"), QLineEdit::Normal,
                                            currentText, &ok);
    if (ok && !newText.isEmpty()) {
        setPlainText(newText);
    }
}

QVariant NodeLabelItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemSelectedChange) {
        // اگر آیتم انتخاب شد، رنگش را عوض کن
        setDefaultTextColor(value.toBool() ? Qt::cyan : Qt::yellow);
    }
    return QGraphicsTextItem::itemChange(change, value);
}
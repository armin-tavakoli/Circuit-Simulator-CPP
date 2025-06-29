#include "terminalitem.h"
#include "componentitem.h"
#include <QPainter>

TerminalItem::TerminalItem(ComponentItem *parent, int terminalId)
// حالا که کامپایلر ارث‌بری را می‌شناسد، این خط به درستی کار می‌کند
        : QGraphicsItem(parent), parentComponent(parent), id(terminalId)
{
    setAcceptHoverEvents(true); // فعال کردن رویداد هاور
}

QRectF TerminalItem::boundingRect() const
{
    // یک مربع کوچک برای دایره
    return QRectF(-4, -4, 8, 8);
}

void TerminalItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    // اگر ماوس روی آن باشد، رنگ را تغییر بده
    if (isHovered) {
        painter->setBrush(Qt::yellow);
    } else {
        painter->setBrush(Qt::red);
    }
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(boundingRect());
}

void TerminalItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    isHovered = true;
    update(); // درخواست رسم مجدد برای تغییر رنگ
    QGraphicsItem::hoverEnterEvent(event);
}

void TerminalItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    isHovered = false;
    update(); // درخواست رسم مجدد برای تغییر رنگ
    QGraphicsItem::hoverLeaveEvent(event);
}

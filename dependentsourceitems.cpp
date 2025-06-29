#include "dependentsourceitems.h"
#include "terminalitem.h"
#include <QPainter>
#include <QString>
#include "Component.h"

QRectF DependentSourceItemBase::boundingRect() const
{
    return QRectF(-40, -50, 80, 100);
}

void DependentSourceItemBase::drawBase(QPainter *painter)
{
    QPolygonF diamond;
    diamond << QPointF(0, -25) << QPointF(25, 0)
            << QPointF(0, 25) << QPointF(-25, 0)
            << QPointF(0, -25);
    painter->drawPolygon(diamond);
    painter->drawLine(0, -40, 0, -25);
    painter->drawLine(0, 25, 0, 40);

    m_terminal1->setPos(0, -40);
    m_terminal2->setPos(0, 40);
}

void VCVSItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    QPen pen(isSelected() ? Qt::cyan : Qt::white, 2);
    painter->setPen(pen);
    drawBase(painter);
    painter->setFont(QFont("Arial", 10, QFont::Bold));
    painter->drawLine(8, -12, 14, -12);
    painter->drawLine(11, -15, 11, -9);
    painter->drawLine(8, 12, 14, 12);
    painter->setPen(Qt::yellow);
    painter->setFont(QFont("Arial", 10));
    QRectF nameRect(-40, -50, 80, 20);
    painter->drawText(nameRect, Qt::AlignCenter, QString::fromStdString(getComponent()->getName()));
    if (auto comp = dynamic_cast<VCVS*>(getComponent())) {
        QRectF gainRect(-40, 30, 80, 20);
        painter->drawText(gainRect, Qt::AlignCenter, "Gain=" + QString::number(comp->get_gain()));
    }
}

void VCCSItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    QPen pen(isSelected() ? Qt::cyan : Qt::white, 2);
    painter->setPen(pen);
    drawBase(painter);
    painter->drawLine(0, -15, 0, 15);
    painter->drawLine(0, 15, -5, 8);
    painter->drawLine(0, 15, 5, 8);
    painter->setPen(Qt::yellow);
    painter->setFont(QFont("Arial", 10));
    QRectF nameRect(-40, -50, 80, 20);
    painter->drawText(nameRect, Qt::AlignCenter, QString::fromStdString(getComponent()->getName()));
    if (auto comp = dynamic_cast<VCCS*>(getComponent())) {
        QRectF gainRect(-40, 30, 80, 20);
        painter->drawText(gainRect, Qt::AlignCenter, "Gain=" + QString::number(comp->get_gain()));
    }
}

void CCVSItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    QPen pen(isSelected() ? Qt::cyan : Qt::white, 2);
    painter->setPen(pen);
    drawBase(painter);
    painter->setFont(QFont("Arial", 10, QFont::Bold));
    painter->drawLine(8, -12, 14, -12);
    painter->drawLine(11, -15, 11, -9);
    painter->drawLine(8, 12, 14, 12);
    painter->setPen(Qt::yellow);
    painter->setFont(QFont("Arial", 10));
    QRectF nameRect(-40, -50, 80, 20);
    painter->drawText(nameRect, Qt::AlignCenter, QString::fromStdString(getComponent()->getName()));
    if (auto comp = dynamic_cast<CCVS*>(getComponent())) {
        QRectF gainRect(-40, 30, 80, 20);
        painter->drawText(gainRect, Qt::AlignCenter, "Gain=" + QString::number(comp->get_gain()));
    }
}

void CCCSItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    QPen pen(isSelected() ? Qt::cyan : Qt::white, 2);
    painter->setPen(pen);
    drawBase(painter);
    painter->drawLine(0, -15, 0, 15);
    painter->drawLine(0, 15, -5, 8);
    painter->drawLine(0, 15, 5, 8);
    painter->setPen(Qt::yellow);
    painter->setFont(QFont("Arial", 10));
    QRectF nameRect(-40, -50, 80, 20);
    painter->drawText(nameRect, Qt::AlignCenter, QString::fromStdString(getComponent()->getName()));
    if (auto comp = dynamic_cast<CCCS*>(getComponent())) {
        QRectF gainRect(-40, 30, 80, 20);
        painter->drawText(gainRect, Qt::AlignCenter, "Gain=" + QString::number(comp->get_gain()));
    }
}
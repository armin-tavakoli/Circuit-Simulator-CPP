// CircuitView.cpp
#include "CircuitView.h"
#include <QGraphicsScene>
#include <QPainter>

CircuitView::CircuitView(QWidget *parent) : QGraphicsView(parent) {
    // یک صحنه گرافیکی جدید ایجاد می‌کنیم
    auto *scene = new QGraphicsScene(this);
    setScene(scene);

    // یک محدوده بزرگ برای صحنه تعریف می‌کنیم تا فضای کافی برای رسم داشته باشیم
    scene->setSceneRect(-5000, -5000, 10000, 10000);

    // کیفیت رندرینگ را برای داشتن خطوط صاف و زیبا بهبود می‌بخشیم
    setRenderHint(QPainter::Antialiasing);
}
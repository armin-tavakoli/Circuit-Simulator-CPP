// CircuitView.h
#ifndef CIRCUITVIEW_H
#define CIRCUITVIEW_H

#include <QGraphicsView>

class CircuitView : public QGraphicsView {
Q_OBJECT

public:
    explicit CircuitView(QWidget *parent = nullptr);
};

#endif // CIRCUITVIEW_H
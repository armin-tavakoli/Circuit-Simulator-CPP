// PlotWindow.h
#ifndef PLOTWINDOW_H
#define PLOTWINDOW_H

#include <QMainWindow>
#include <QtCharts/QChartView> // Add the full header here

// Use the namespace for convenience
QT_CHARTS_USE_NAMESPACE

class PlotWindow : public QMainWindow
{
Q_OBJECT

public:
    explicit PlotWindow(QWidget *parent = nullptr);

private:
    QChartView *chartView; // The compiler now knows the full definition
};

#endif //PLOTWINDOW_H
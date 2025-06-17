// PlotWindow.cpp
#include "PlotWindow.h"

#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QPainter>

// The namespace is already handled by the header file,
// but having it here is also fine.
using namespace QtCharts;

PlotWindow::PlotWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle("Transient Analysis Plot");

    auto *chart = new QChart();
    chart->legend()->hide();
    chart->setTitle("Simple Line Chart Example");

    auto *series = new QLineSeries();
    series->append(0, 6);
    series->append(2, 4);
    series->append(3, 8);
    series->append(7, 4);
    series->append(10, 5);
    *series << QPointF(11, 1) << QPointF(13, 3) << QPointF(17, 6) << QPointF(18, 3) << QPointF(20, 2);

    chart->addSeries(series);
    chart->createDefaultAxes();

    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    setCentralWidget(chartView);
    resize(800, 600);
}
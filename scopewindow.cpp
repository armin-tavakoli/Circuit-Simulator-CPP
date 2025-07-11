#include "scopewindow.h"
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QLogValueAxis> // Included for logarithmic axis
#include <QVBoxLayout>

QT_USE_NAMESPACE

ScopeWindow::ScopeWindow(const std::map<std::string, std::vector<double>>& results, const QString& xAxisTitle, QWidget *parent)
        : QDialog(parent), m_chartView(nullptr)
{
    setWindowTitle(tr("Simulation Scope"));
    setMinimumSize(800, 600);

    setupChart(results, xAxisTitle);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_chartView);
    setLayout(layout);
}

ScopeWindow::~ScopeWindow()
{
}

void ScopeWindow::setupChart(const std::map<std::string, std::vector<double>>& results, const QString& xAxisTitle)
{
    QChart *chart = new QChart();
    chart->setTitle("Simulation Results");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    // Determine the X-axis data (either Time or Frequency)
    const std::vector<double>* xData = nullptr;
    if (results.count("Time")) {
        xData = &results.at("Time");
    } else if (results.count("Frequency")) {
        xData = &results.at("Frequency");
    }

    if (!xData || xData->empty()) {
        return; // No data to plot
    }

    // Create a series for each variable
    for (const auto& pair : results) {
        if (pair.first == "Time" || pair.first == "Frequency") {
            continue;
        }

        QLineSeries *series = new QLineSeries();
        series->setName(QString::fromStdString(pair.first));

        const std::vector<double>& yData = pair.second;
        size_t dataSize = std::min(xData->size(), yData.size());

        for (size_t i = 0; i < dataSize; ++i) {
            series->append((*xData)[i], yData[i]);
        }
        chart->addSeries(series);
    }

    // Customize axes based on analysis type
    if (xAxisTitle.toLower().contains("freq")) {
        // Use logarithmic axis for frequency sweep
        QLogValueAxis *axisX = new QLogValueAxis();
        axisX->setTitleText(xAxisTitle);
        axisX->setLabelFormat("%g");
        axisX->setBase(10.0);
        chart->addAxis(axisX, Qt::AlignBottom);

        QValueAxis *axisY = new QValueAxis();
        axisY->setTitleText("Magnitude");
        chart->addAxis(axisY, Qt::AlignLeft);

        // Attach series to the new axes
        for(auto series : chart->series()) {
            series->attachAxis(axisX);
            series->attachAxis(axisY);
        }
    } else {
        // Use default linear axes for other types (like Transient)
        chart->createDefaultAxes();
        QList<QAbstractAxis*> axes = chart->axes(Qt::Horizontal);
        if (!axes.isEmpty()) {
            axes.first()->setTitleText(xAxisTitle);
        }
    }

    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    m_chartView = new QChartView(chart);
    m_chartView->setRenderHint(QPainter::Antialiasing);
}

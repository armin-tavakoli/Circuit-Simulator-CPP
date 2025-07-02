#include "scopewindow.h"
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QVBoxLayout>

QT_USE_NAMESPACE

ScopeWindow::ScopeWindow(const std::map<std::string, std::vector<double>>& results, QWidget *parent)
        : QDialog(parent), m_chartView(nullptr)
{
    setWindowTitle(tr("Simulation Scope"));
    setMinimumSize(800, 600);

    setupChart(results);

    // Set up the layout for the dialog
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_chartView);
    setLayout(layout);
}

ScopeWindow::~ScopeWindow()
{
    // The chart view and its contents will be deleted automatically
}

void ScopeWindow::setupChart(const std::map<std::string, std::vector<double>>& results)
{
    QChart *chart = new QChart();
    chart->setTitle("Transient Analysis Results");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    // Find the "Time" vector, which will be our X-axis
    auto it = results.find("Time");
    if (it == results.end() || it->second.empty()) {
        // Handle case where there is no time data
        return;
    }
    const std::vector<double>& timeData = it->second;

    // Create a series for each variable (except "Time")
    for (const auto& pair : results) {
        if (pair.first == "Time") {
            continue; // Skip the time vector itself
        }

        QLineSeries *series = new QLineSeries();
        series->setName(QString::fromStdString(pair.first));

        const std::vector<double>& yData = pair.second;
        size_t dataSize = std::min(timeData.size(), yData.size());

        for (size_t i = 0; i < dataSize; ++i) {
            series->append(timeData[i], yData[i]);
        }
        chart->addSeries(series);
    }

    chart->createDefaultAxes();
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    m_chartView = new QChartView(chart);
    m_chartView->setRenderHint(QPainter::Antialiasing);
}

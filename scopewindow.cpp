#include "scopewindow.h"
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QLogValueAxis>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include "mathoperationsdialog.h"
#include <QPushButton>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QtCharts/QLegendMarker>
#include <QColorDialog>
#include <QtCharts/QLegend>
QT_USE_NAMESPACE

ScopeWindow::ScopeWindow(const std::map<std::string, std::vector<double>>& results, const QString& xAxisTitle, QWidget *parent)
        : QDialog(parent), m_chartView(nullptr),
          m_cursor1Line(nullptr), m_cursor2Line(nullptr),
          m_cursor1Text(nullptr), m_cursor2Text(nullptr), m_diffText(nullptr),
          m_cursorState(0)
{
    setWindowTitle(tr("Simulation Scope"));
    setMinimumSize(800, 600);
    setupChart(results, xAxisTitle);

    QVBoxLayout *layout = new QVBoxLayout(this);
    if (m_chartView) {
        layout->addWidget(m_chartView);
        setMouseTracking(true);
        m_chartView->setMouseTracking(true);
    }

    QPushButton *mathButton = new QPushButton("Math Operations");
    connect(mathButton, &QPushButton::clicked, this, &ScopeWindow::performMathOperation);
    QPushButton *autoZoomButton = new QPushButton("Auto Zoom");
    connect(autoZoomButton, &QPushButton::clicked, this, &ScopeWindow::onAutoZoom);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(mathButton);
    buttonLayout->addWidget(autoZoomButton);

    layout->addLayout(buttonLayout);

    setLayout(layout);
}

ScopeWindow::~ScopeWindow() {}

void ScopeWindow::setupChart(const std::map<std::string, std::vector<double>>& results, const QString& xAxisTitle)
{
    QChart *chart = new QChart();
    chart->setTitle("Simulation Results");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    if (results.count("Time")) m_xData = results.at("Time");
    else if (results.count("Frequency")) m_xData = results.at("Frequency");
    else if (results.count("Phase")) m_xData = results.at("Phase");

    if (m_xData.empty()) return;

    for (const auto& pair : results) {
        QString name = QString::fromStdString(pair.first);
        if (name != "Time" && name != "Frequency" && name != "Phase") {
            m_yData[name] = pair.second; // کپی کردن داده‌های محور Y
            QLineSeries *series = new QLineSeries();
            series->setName(name);
            for (size_t i = 0; i < std::min(m_xData.size(), pair.second.size()); ++i) {
                series->append(m_xData[i], pair.second[i]);
            }
            chart->addSeries(series);
        }
    }

    if (xAxisTitle.toLower().contains("freq")) {
        QLogValueAxis *axisX = new QLogValueAxis();
        axisX->setTitleText(xAxisTitle);
        axisX->setBase(10.0);
        chart->addAxis(axisX, Qt::AlignBottom);
        QValueAxis *axisY = new QValueAxis();
        axisY->setTitleText("Magnitude");
        chart->addAxis(axisY, Qt::AlignLeft);
        for(auto series : chart->series()) {
            series->attachAxis(axisX);
            series->attachAxis(axisY);
        }
    } else {
        chart->createDefaultAxes();
        if (!chart->axes(Qt::Horizontal).isEmpty()) {
            chart->axes(Qt::Horizontal).first()->setTitleText(xAxisTitle);
        }
    }
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    for (QLegendMarker* marker : chart->legend()->markers()) {
        connect(marker, &QLegendMarker::clicked, this, [=](){
            handleLegendClicked(marker);
        });
    }

    m_chartView = new QChartView(chart);
    m_chartView->setRenderHint(QPainter::Antialiasing);

    m_cursor1Line = new QGraphicsLineItem(chart);
    m_cursor1Line->setPen(QPen(Qt::cyan, 1, Qt::DashLine));
    m_cursor1Line->setVisible(false);
    m_cursor1Text = new QGraphicsTextItem(chart);
    m_cursor1Text->setDefaultTextColor(Qt::cyan);
    m_cursor1Text->setZValue(10);
    m_cursor1Text->setVisible(false);

    m_cursor2Line = new QGraphicsLineItem(chart);
    m_cursor2Line->setPen(QPen(Qt::magenta, 1, Qt::DashLine));
    m_cursor2Line->setVisible(false);
    m_cursor2Text = new QGraphicsTextItem(chart);
    m_cursor2Text->setDefaultTextColor(Qt::magenta);
    m_cursor2Text->setZValue(10);
    m_cursor2Text->setVisible(false);

    m_diffText = new QGraphicsTextItem(chart);
    m_diffText->setDefaultTextColor(Qt::black);
    m_diffText->setZValue(10);
    m_diffText->setVisible(false);
}

void ScopeWindow::addSeries(const QString& name, const std::vector<double>& yData)
{
    if (!m_chartView || m_yData.count(name)) return;

    m_yData[name] = yData;
    QChart* chart = m_chartView->chart();
    QLineSeries *series = new QLineSeries();
    series->setName(name);

    for (size_t i = 0; i < std::min(m_xData.size(), yData.size()); ++i) {
        series->append(m_xData[i], yData[i]);
    }
    chart->addSeries(series);

    for (QLegendMarker* marker : chart->legend()->markers(series)) {
        disconnect(marker, &QLegendMarker::clicked, nullptr, nullptr);
        connect(marker, &QLegendMarker::clicked, this, [=](){
            handleLegendClicked(marker);
        });
    }

    series->attachAxis(chart->axes(Qt::Horizontal).first());
    series->attachAxis(chart->axes(Qt::Vertical).first());
}

void ScopeWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_chartView->chart()->plotArea().contains(event->pos())) {
        m_cursorState = (m_cursorState + 1) % 3;

        m_cursor1Active = (m_cursorState == 1 || m_cursorState == 2);
        m_cursor2Active = (m_cursorState == 2);

        m_cursor1Line->setVisible(m_cursor1Active);
        m_cursor1Text->setVisible(m_cursor1Active);

        m_cursor2Line->setVisible(m_cursor2Active);
        m_cursor2Text->setVisible(m_cursor2Active);
        m_diffText->setVisible(m_cursor2Active);

        updateCursorPosition(event->pos());
    }
}

void ScopeWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (m_cursorState > 0) {
        updateCursorPosition(event->pos());
    }
}

void ScopeWindow::updateCursorPosition(const QPoint& viewPos)
{
    if (!m_chartView || m_chartView->chart()->series().isEmpty() || m_xData.empty()) return;

    QChart *chart = m_chartView->chart();
    QPointF chartValue = chart->mapToValue(viewPos);
    auto series = static_cast<QLineSeries*>(chart->series().first());

    int closestIndex = 0;
    double minDistance = std::abs(m_xData[0] - chartValue.x());
    for (size_t i = 1; i < m_xData.size(); ++i) {
        double dist = std::abs(m_xData[i] - chartValue.x());
        if (dist < minDistance) {
            minDistance = dist;
            closestIndex = i;
        }
    }

    double xVal = m_xData[closestIndex];
    double yVal = series->at(closestIndex).y();
    QPointF dataPointOnScene = chart->mapToPosition({xVal, yVal}, series);
    QRectF plotArea = chart->plotArea();

    if (m_cursorState == 1) {
        m_cursor1Line->setLine(dataPointOnScene.x(), plotArea.top(), dataPointOnScene.x(), plotArea.bottom());
        QString text = QString("C1:\nX: %1\nY: %2").arg(xVal, 0, 'g', 4).arg(yVal, 0, 'g', 4);
        m_cursor1Text->setPlainText(text);
        m_cursor1Text->setPos(dataPointOnScene.x() + 5, plotArea.top());
    } else if (m_cursorState == 2) {
        m_cursor2Line->setLine(dataPointOnScene.x(), plotArea.top(), dataPointOnScene.x(), plotArea.bottom());
        QString text = QString("C2:\nX: %1\nY: %2").arg(xVal, 0, 'g', 4).arg(yVal, 0, 'g', 4);
        m_cursor2Text->setPlainText(text);
        m_cursor2Text->setPos(dataPointOnScene.x() + 5, plotArea.top() + 40);
        updateDiffText();
    }
}

QStringList ScopeWindow::getCurrentSignalNames() const
{
    QStringList names;
    if (m_chartView && m_chartView->chart()) {
        for (auto series : m_chartView->chart()->series()) {
            names.append(series->name());
        }
    }
    return names;
}

void ScopeWindow::performMathOperation()
{
    if (m_yData.size() < 2) {
        QMessageBox::warning(this, "Error", "You need at least two signals to perform an operation.");
        return;
    }

    MathOperationsDialog dialog(getCurrentSignalNames(), this);
    if (dialog.exec() == QDialog::Accepted) {
        QString sig1_name = dialog.getFirstSignal();
        QString sig2_name = dialog.getSecondSignal();
        QString op = dialog.getOperation();
        QString result_name = dialog.getResultSignalName();

        if (m_yData.find(result_name) != m_yData.end()) {
            QMessageBox::warning(this, "Error", "A signal with this name already exists.");
            return;
        }

        const auto& v1 = m_yData.at(sig1_name);
        const auto& v2 = m_yData.at(sig2_name);
        std::vector<double> result_vec;
        result_vec.reserve(v1.size());

        for (size_t i = 0; i < std::min(v1.size(), v2.size()); ++i) {
            if (op == "+") {
                result_vec.push_back(v1[i] + v2[i]);
            } else if (op == "-") {
                result_vec.push_back(v1[i] - v2[i]);
            }
        }
        addSeries(result_name, result_vec);
    }
}
void ScopeWindow::updateDiffText()
{
    if (!m_cursor1Active || !m_cursor2Active) return;

    double x1 = m_cursor1Text->toPlainText().split("\n")[1].split(" ")[1].toDouble();
    double y1 = m_cursor1Text->toPlainText().split("\n")[2].split(" ")[1].toDouble();
    double x2 = m_cursor2Text->toPlainText().split("\n")[1].split(" ")[1].toDouble();
    double y2 = m_cursor2Text->toPlainText().split("\n")[2].split(" ")[1].toDouble();

    double dx = x2 - x1;
    double dy = y2 - y1;
    double slope = (dx == 0) ? std::numeric_limits<double>::infinity() : dy / dx;

    QString text = QString("Difference:\ndX: %1\ndY: %2\nSlope: %3")
            .arg(dx, 0, 'g', 4)
            .arg(dy, 0, 'g', 4)
            .arg(slope, 0, 'g', 4);

    m_diffText->setPlainText(text);
    m_diffText->setPos(m_chartView->chart()->plotArea().topLeft() + QPointF(5, 80));
}

void ScopeWindow::onAutoZoom()
{
    if (!m_chartView || m_chartView->chart()->series().isEmpty()) {
        return;
    }

    QChart *chart = m_chartView->chart();
    double globalMinY = std::numeric_limits<double>::max();
    double globalMaxY = std::numeric_limits<double>::lowest();

    for (QAbstractSeries *abstractSeries : chart->series()) {
        auto series = static_cast<QLineSeries*>(abstractSeries);
        const auto points = series->pointsVector();
        for (const QPointF &p : points) {
            if (p.y() < globalMinY) globalMinY = p.y();
            if (p.y() > globalMaxY) globalMaxY = p.y();
        }
    }

    if (globalMinY == std::numeric_limits<double>::max()) {
        return;
    }
    auto axisY = qobject_cast<QValueAxis*>(chart->axes(Qt::Vertical).first());
    if (axisY) {
        double margin = (globalMaxY - globalMinY) * 0.05;
        if (margin == 0) margin = 1.0;

        axisY->setMin(globalMinY - margin);
        axisY->setMax(globalMaxY + margin);
    }
}

void ScopeWindow::handleLegendClicked(QLegendMarker* marker)
{
    if (!marker) return;

    QLineSeries *series = qobject_cast<QLineSeries *>(marker->series());
    if (!series) return;

    QColor newColor = QColorDialog::getColor(series->color(), this, "Select Signal Color");

    if (newColor.isValid()) {
        series->setColor(newColor);
        marker->setBrush(newColor);
    }
}
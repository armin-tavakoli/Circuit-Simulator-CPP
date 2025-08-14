#ifndef SCOPEWINDOW_H
#define SCOPEWINDOW_H

#include <QDialog>
#include <map>
#include <vector>
#include <string>

QT_BEGIN_NAMESPACE
class QChartView;
class QChart;
class QGraphicsLineItem;
class QGraphicsTextItem;
class QMouseEvent;
class QLegendMarker;
QT_END_NAMESPACE

class ScopeWindow : public QDialog
{
Q_OBJECT

public:
    explicit ScopeWindow(const std::map<std::string, std::vector<double>>& results, const QString& xAxisTitle, QWidget *parent = nullptr);
    ~ScopeWindow();

    void addSeries(const QString& name, const std::vector<double>& yData);
    void performMathOperation();

protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void onAutoZoom();
    void handleLegendClicked(QLegendMarker* marker);

private:
    void setupChart(const std::map<std::string, std::vector<double>>& results, const QString& xAxisTitle);
    QStringList getCurrentSignalNames() const;


    void updateCursorPosition(const QPoint& viewPos);
    void updateDiffText();

    QChartView *m_chartView;
    std::vector<double> m_xData;
    std::map<QString, std::vector<double>> m_yData;


    QGraphicsLineItem *m_cursor1Line, *m_cursor2Line;
    QGraphicsTextItem *m_cursor1Text, *m_cursor2Text, *m_diffText;
    bool m_cursor1Active, m_cursor2Active;
    int m_cursorState;
};

#endif //SCOPEWINDOW_H
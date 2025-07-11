#ifndef SCOPEWINDOW_H
#define SCOPEWINDOW_H

#include <QDialog>
#include <map>
#include <vector>
#include <string>

// Forward declarations for Qt Charts classes
QT_BEGIN_NAMESPACE
class QChartView;
class QChart;
QT_END_NAMESPACE

class ScopeWindow : public QDialog
{
Q_OBJECT

public:
    // Constructor updated to accept an X-axis title
    explicit ScopeWindow(const std::map<std::string, std::vector<double>>& results, const QString& xAxisTitle, QWidget *parent = nullptr);
    ~ScopeWindow();

private:
    void setupChart(const std::map<std::string, std::vector<double>>& results, const QString& xAxisTitle);

    QChartView *m_chartView;
};

#endif
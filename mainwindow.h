#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <map>
#include <string>
#include "Circuit.h"
#include "schematiceditor.h"

class ScopeWindow;

class MainWindow : public QMainWindow
{
Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onFileNew();
    void onFileOpen();
    void onFileSave();
    void onAddResistor();
    void onAddCapacitor();
    void onAddInductor();
    void onAddVoltageSource();
    void onAddCurrentSource();
    void onAddGround();
    void onAddVCVS();
    void onAddVCCS();
    void onAddCCVS();
    void onAddCCCS();
    void onRunSimulation();

    void onAddSinusoidalSource();
    void onAddPulseSource();
    void onAddACVoltageSource();

private:
    void setupMenus();
    std::string getNextComponentName(const std::string& prefix);

    SchematicEditor *editor;
    Circuit circuit;
    ScopeWindow *m_scopeWindow = nullptr;

    std::map<std::string, int> componentCounters;
};

#endif // MAINWINDOW_H

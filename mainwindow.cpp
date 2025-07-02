#include "mainwindow.h"
#include "resistoritem.h"
#include "capacitoritem.h"
#include "inductoritem.h"
#include "voltagesourceitem.h"
#include "currentsourceitem.h"
#include "grounditem.h"
#include "dependentsourceitems.h"
#include "scopewindow.h" // <<<
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QGraphicsScene>
#include <QToolBar>
#include <QPushButton>

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent)
{
    setWindowTitle(tr("ShariF Spice Simulator"));
    resize(1280, 720);

    editor = new SchematicEditor(&circuit, this);
    setCentralWidget(editor);

    setupMenus();

    componentCounters["R"] = 1;
    componentCounters["C"] = 1;
    componentCounters["L"] = 1;
    componentCounters["V"] = 1;
    componentCounters["I"] = 1;
    componentCounters["E"] = 1;
    componentCounters["G"] = 1;
    componentCounters["H"] = 1;
    componentCounters["F"] = 1;
}

MainWindow::~MainWindow()
{
    // The scope window will be deleted automatically as it's a child of MainWindow
}

void MainWindow::setupMenus()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(tr("&New"), this, &MainWindow::onFileNew);
    fileMenu->addAction(tr("&Open..."), this, &MainWindow::onFileOpen);
    fileMenu->addAction(tr("&Save"), this, &MainWindow::onFileSave);
    fileMenu->addSeparator();
    fileMenu->addAction(tr("E&xit"), this, &QWidget::close);

    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(tr("Add &Resistor"), this, &MainWindow::onAddResistor);
    editMenu->addAction(tr("Add &Capacitor"), this, &MainWindow::onAddCapacitor);
    editMenu->addAction(tr("Add &Inductor"), this, &MainWindow::onAddInductor);
    editMenu->addSeparator();
    editMenu->addAction(tr("Add DC &Voltage Source"), this, &MainWindow::onAddVoltageSource);
    editMenu->addAction(tr("Add DC &Current Source"), this, &MainWindow::onAddCurrentSource);
    editMenu->addSeparator();
    editMenu->addAction(tr("Add VCVS (E)"), this, &MainWindow::onAddVCVS);
    editMenu->addAction(tr("Add VCCS (G)"), this, &MainWindow::onAddVCCS);
    editMenu->addAction(tr("Add CCVS (H)"), this, &MainWindow::onAddCCVS);
    editMenu->addAction(tr("Add CCCS (F)"), this, &MainWindow::onAddCCCS);
    editMenu->addSeparator();
    editMenu->addAction(tr("Add &Ground"), this, &MainWindow::onAddGround);
    editMenu->addSeparator();
    QAction* wireAction = editMenu->addAction(tr("Add &Wire"));
    wireAction->setCheckable(true);
    connect(wireAction, &QAction::toggled, editor, &SchematicEditor::toggleWiringMode);

    QMenu *simulateMenu = menuBar()->addMenu(tr("&Simulate"));
    simulateMenu->addAction(tr("&Run Transient Analysis"), this, &MainWindow::onRunSimulation);

    QToolBar *toolBar = addToolBar(tr("Main Toolbar"));
    QPushButton *runButton = new QPushButton(tr("Run Simulation"), this);
    connect(runButton, &QPushButton::clicked, this, &MainWindow::onRunSimulation);
    toolBar->addWidget(runButton);

    menuBar()->addMenu(tr("&View"));
    menuBar()->addMenu(tr("Sco&pe"));
    menuBar()->addMenu(tr("&Help"));
}

void MainWindow::onRunSimulation()
{
    editor->updateBackendNodes();

    try {
        circuit.runTransientAnalysis(1e-3, 1e-6);

        // <<< نمایش پنجره Scope >>>
        if (m_scopeWindow) {
            m_scopeWindow->close();
            delete m_scopeWindow;
        }
        m_scopeWindow = new ScopeWindow(circuit.getSimulationResults(), this);
        m_scopeWindow->show();

    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Simulation Error", e.what());
    }
}

// ... بقیه توابع بدون تغییر ...
std::string MainWindow::getNextComponentName(const std::string& prefix)
{
    int count = componentCounters[prefix];
    componentCounters[prefix]++;
    return prefix + std::to_string(count);
}

void MainWindow::onFileNew() { QMessageBox::information(this, "Action", "New File action triggered!"); }
void MainWindow::onFileOpen() { QMessageBox::information(this, "Action", "Open File action triggered!"); }
void MainWindow::onFileSave() { QMessageBox::information(this, "Action", "Save File action triggered!"); }

void MainWindow::onAddResistor()
{
    std::string name = getNextComponentName("R");
    auto logic_comp = std::make_unique<Resistor>(name, -1, -1, 1000.0);
    Component* ptr = logic_comp.get();
    circuit.addComponent(std::move(logic_comp));
    editor->scene()->addItem(new ResistorItem(ptr));
}

void MainWindow::onAddCapacitor()
{
    std::string name = getNextComponentName("C");
    auto logic_comp = std::make_unique<Capacitor>(name, -1, -1, 1e-6);
    Component* ptr = logic_comp.get();
    circuit.addComponent(std::move(logic_comp));
    editor->scene()->addItem(new CapacitorItem(ptr));
}

void MainWindow::onAddInductor()
{
    std::string name = getNextComponentName("L");
    auto logic_comp = std::make_unique<Inductor>(name, -1, -1, 1e-3);
    Component* ptr = logic_comp.get();
    circuit.addComponent(std::move(logic_comp));
    editor->scene()->addItem(new InductorItem(ptr));
}

void MainWindow::onAddVoltageSource()
{
    std::string name = getNextComponentName("V");
    auto logic_comp = std::make_unique<VoltageSource>(name, -1, -1, 5.0);
    Component* ptr = logic_comp.get();
    circuit.addComponent(std::move(logic_comp));
    editor->scene()->addItem(new VoltageSourceItem(ptr));
}

void MainWindow::onAddCurrentSource()
{
    std::string name = getNextComponentName("I");
    auto logic_comp = std::make_unique<CurrentSource>(name, -1, -1, 1.0);
    Component* ptr = logic_comp.get();
    circuit.addComponent(std::move(logic_comp));
    editor->scene()->addItem(new CurrentSourceItem(ptr));
}

void MainWindow::onAddGround()
{
    auto ground_item = new GroundItem();
    editor->scene()->addItem(ground_item);
}

void MainWindow::onAddVCVS()
{
    std::string name = getNextComponentName("E");
    auto logic_comp = std::make_unique<VCVS>(name, -1, -1, -1, -1, 2.0);
    Component* ptr = logic_comp.get();
    circuit.addComponent(std::move(logic_comp));
    editor->scene()->addItem(new VCVSItem(ptr));
}

void MainWindow::onAddVCCS()
{
    std::string name = getNextComponentName("G");
    auto logic_comp = std::make_unique<VCCS>(name, -1, -1, -1, -1, 0.1);
    Component* ptr = logic_comp.get();
    circuit.addComponent(std::move(logic_comp));
    editor->scene()->addItem(new VCCSItem(ptr));
}

void MainWindow::onAddCCVS()
{
    std::string name = getNextComponentName("H");
    auto logic_comp = std::make_unique<CCVS>(name, -1, -1, "Vdummy", 10.0);
    Component* ptr = logic_comp.get();
    circuit.addComponent(std::move(logic_comp));
    editor->scene()->addItem(new CCVSItem(ptr));
}

void MainWindow::onAddCCCS()
{
    std::string name = getNextComponentName("F");
    auto logic_comp = std::make_unique<CCCS>(name, -1, -1, "Vdummy", 5.0);
    Component* ptr = logic_comp.get();
    circuit.addComponent(std::move(logic_comp));
    editor->scene()->addItem(new CCCSItem(ptr));
}

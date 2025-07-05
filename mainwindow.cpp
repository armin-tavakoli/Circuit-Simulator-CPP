#include "mainwindow.h"
#include "resistoritem.h"
#include "capacitoritem.h"
#include "inductoritem.h"
#include "voltagesourceitem.h"
#include "currentsourceitem.h"
#include "grounditem.h"
#include "dependentsourceitems.h"
#include "scopewindow.h"
#include "propertiesdialog.h"
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QGraphicsScene>
#include <QToolBar>
#include <QPushButton>
#include <utility>
#include <QFileDialog>

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
}

void MainWindow::setupMenus()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(tr("&New"), this, &MainWindow::onFileNew);
    fileMenu->addAction(tr("&Open..."), this, &MainWindow::onFileOpen);
    fileMenu->addAction(tr("&Save..."), this, &MainWindow::onFileSave);
    fileMenu->addSeparator();
    fileMenu->addAction(tr("E&xit"), this, &QWidget::close);

    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(tr("Add &Resistor"), this, &MainWindow::onAddResistor);
    editMenu->addAction(tr("Add &Capacitor"), this, &MainWindow::onAddCapacitor);
    editMenu->addAction(tr("Add &Inductor"), this, &MainWindow::onAddInductor);
    editMenu->addSeparator();
    editMenu->addAction(tr("Add DC &Voltage Source"), this, &MainWindow::onAddVoltageSource);
    editMenu->addAction(tr("Add &Sinusoidal Source"), this, &MainWindow::onAddSinusoidalSource);
    editMenu->addAction(tr("Add &Pulse Source"), this, &MainWindow::onAddPulseSource);
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

void MainWindow::onFileNew()
{
    circuit.clear();
    editor->populateSceneFromCircuit();
    QMessageBox::information(this, "Action", "New circuit created.");
}

void MainWindow::onFileSave()
{
    for (QGraphicsItem* item : editor->scene()->items()) {
        if (auto compItem = dynamic_cast<ComponentItem*>(item)) {
            if (auto logicComp = compItem->getComponent()) {
                logicComp->setPosition(compItem->pos().x(), compItem->pos().y());
            }
        }
    }
    editor->updateCircuitWires();
    QString filePath = QFileDialog::getSaveFileName(this, "Save Circuit", "", "Circuit Files (*.cir)");
    if (!filePath.isEmpty()) {
        try {
            circuit.saveToFile(filePath.toStdString());
            QMessageBox::information(this, "Success", "Circuit saved successfully!");
        } catch (const std::exception& e) {
            QMessageBox::critical(this, "Save Error", e.what());
        }
    }
}

void MainWindow::onFileOpen()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Open Circuit", "", "Circuit Files (*.cir)");
    if (!filePath.isEmpty()) {
        try {
            circuit.loadFromFile(filePath.toStdString());

            editor->populateSceneFromCircuit();

            QMessageBox::information(this, "Success", "Circuit loaded successfully!");
        } catch (const std::exception& e) {
            QMessageBox::critical(this, "Load Error", e.what());
        }
    }
}

void MainWindow::onRunSimulation()
{
    editor->updateBackendNodes();

    try {
        circuit.runTransientAnalysis(1e-3, 1e-6);

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

std::string MainWindow::getNextComponentName(const std::string& prefix)
{
    int count = componentCounters[prefix];
    componentCounters[prefix]++;
    return prefix + std::to_string(count);
}

void MainWindow::onAddResistor()
{
    string name = getNextComponentName("R");
    auto logic_comp = make_unique<Resistor>(name, -1, -1, 1000.0);
    Component* ptr = logic_comp.get();
    circuit.addComponent(std::move(logic_comp));
    editor->scene()->addItem(new ResistorItem(ptr));
}

void MainWindow::onAddCapacitor()
{
    string name = getNextComponentName("C");
    auto logic_comp = make_unique<Capacitor>(name, -1, -1, 1e-6);
    Component* ptr = logic_comp.get();
    circuit.addComponent(std::move(logic_comp));
    editor->scene()->addItem(new CapacitorItem(ptr));
}

void MainWindow::onAddInductor()
{
    string name = getNextComponentName("L");
    auto logic_comp = make_unique<Inductor>(name, -1, -1, 1e-3);
    Component* ptr = logic_comp.get();
    circuit.addComponent(std::move(logic_comp));
    editor->scene()->addItem(new InductorItem(ptr));
}

void MainWindow::onAddVoltageSource()
{
    string name = getNextComponentName("V");
    auto logic_comp = make_unique<VoltageSource>(name, -1, -1, 5.0);
    Component* ptr = logic_comp.get();
    circuit.addComponent(std::move(logic_comp));
    editor->scene()->addItem(new VoltageSourceItem(ptr));
}

void MainWindow::onAddCurrentSource()
{
    string name = getNextComponentName("I");
    auto logic_comp = make_unique<CurrentSource>(name, -1, -1, 1.0);
    Component* ptr = logic_comp.get();
    circuit.addComponent(std::move(logic_comp));
    editor->scene()->addItem(new CurrentSourceItem(ptr));
}

void MainWindow::onAddGround()
{
    string name = getNextComponentName("GND");
    auto logic_comp = make_unique<Ground>(name, 0, 0);
    Component* ptr = logic_comp.get();
    circuit.addComponent(std::move(logic_comp));
    editor->scene()->addItem(new GroundItem(ptr));
}

void MainWindow::onAddVCVS()
{
    string name = getNextComponentName("E");
    auto logic_comp = make_unique<VCVS>(name, -1, -1, -1, -1, 2.0);
    Component* ptr = logic_comp.get();
    circuit.addComponent(std::move(logic_comp));
    editor->scene()->addItem(new VCVSItem(ptr));
}

void MainWindow::onAddVCCS()
{
    string name = getNextComponentName("G");
    auto logic_comp = make_unique<VCCS>(name, -1, -1, -1, -1, 0.1);
    Component* ptr = logic_comp.get();
    circuit.addComponent(std::move(logic_comp));
    editor->scene()->addItem(new VCCSItem(ptr));
}

void MainWindow::onAddCCVS()
{
    string name = getNextComponentName("H");
    auto logic_comp = make_unique<CCVS>(name, -1, -1, "Vdummy", 10.0);
    Component* ptr = logic_comp.get();
    circuit.addComponent(std::move(logic_comp));
    editor->scene()->addItem(new CCVSItem(ptr));
}

void MainWindow::onAddCCCS()
{
    string name = getNextComponentName("F");
    auto logic_comp = make_unique<CCCS>(name, -1, -1, "Vdummy", 5.0);
    Component* ptr = logic_comp.get();
    circuit.addComponent(std::move(logic_comp));
    editor->scene()->addItem(new CCCSItem(ptr));
}

void MainWindow::onAddSinusoidalSource()
{
    string name = getNextComponentName("V");
    auto tempSource = SinusoidalVoltageSource(name, -1, -1, 0, 5, 1000);
    PropertiesDialog dialog(tempSource.getProperties(), this);
    if (dialog.exec() == QDialog::Accepted) {
        try {
            auto props = dialog.getProperties();
            auto logic_comp = make_unique<SinusoidalVoltageSource>(name, -1, -1, props["Offset"], props["Amplitude"], props["Frequency"]);
            Component* ptr = logic_comp.get();
            circuit.addComponent(std::move(logic_comp));
            editor->scene()->addItem(new VoltageSourceItem(ptr));
        } catch (const std::exception& e) {
            QMessageBox::warning(this, "Invalid Value", e.what());
        }
    }
}

void MainWindow::onAddPulseSource()
{
    string name = getNextComponentName("V");
    auto tempSource = PulseVoltageSource(name, -1, -1, 0, 5, 0, 1e-9, 1e-9, 5e-7, 1e-6);
    PropertiesDialog dialog(tempSource.getProperties(), this);
    if (dialog.exec() == QDialog::Accepted) {
        try {
            auto props = dialog.getProperties();
            auto logic_comp = make_unique<PulseVoltageSource>(name, -1, -1,
                                                              props["Initial Value"], props["Pulsed Value"], props["Delay"],
                                                              props["Rise Time"], props["Fall Time"], props["Pulse Width"], props["Period"]);
            Component* ptr = logic_comp.get();
            circuit.addComponent(std::move(logic_comp));
            editor->scene()->addItem(new VoltageSourceItem(ptr));
        } catch (const std::exception& e) {
            QMessageBox::warning(this, "Invalid Value", e.what());
        }
    }
}

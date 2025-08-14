#include "mainwindow.h"
#include "schematiceditor.h"
#include "Circuit.h"
#include "resistoritem.h"
#include "capacitoritem.h"
#include "inductoritem.h"
#include "voltagesourceitem.h"
#include "currentsourceitem.h"
#include "grounditem.h"
#include "dependentsourceitems.h"
#include "scopewindow.h"
#include "propertiesdialog.h"
#include "simulationdialog.h"
#include "plotselectiondialog.h"
#include "SaveSubcircuitDialog.h"
#include "SubCircuit.h"
#include "SubCircuitItem.h"
#include <QTabWidget>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QFileDialog>
#include <QDir>
#include <QCoreApplication>
#include <QFileInfo>
#include <QPushButton>
#include <QToolBar>
#include <QDebug>
#include <utility>
#include "nodelabelitem.h"
#include <QInputDialog>
#include <QStatusBar>
#include <QNetworkInterface>
#include <QStatusBar>

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent)
{
    setWindowTitle(tr("ShariF Spice Simulator"));
    resize(1280, 720);

    tabWidget = new QTabWidget(this);
    tabWidget->setTabsClosable(true);
    tabWidget->setMovable(true);
    setCentralWidget(tabWidget);

    connect(tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::onTabClose);

    setupMenus();

    m_server = new Server(this);
    connect(m_server, &Server::logMessage, this, &MainWindow::logNetworkMessage);

    m_client = new Client(this);
    connect(m_client, &Client::logMessage, this, &MainWindow::logNetworkMessage);
    connect(m_client, &Client::voltageReceived, this, &MainWindow::onVoltageReceived);

    m_broadcastTimer = new QTimer(this);
    connect(m_broadcastTimer, &QTimer::timeout, this, &MainWindow::onBroadcastSignal);

    componentCounters["R"] = 1;
    componentCounters["C"] = 1;
    componentCounters["L"] = 1;
    componentCounters["V"] = 1;
    componentCounters["I"] = 1;
    componentCounters["E"] = 1;
    componentCounters["G"] = 1;
    componentCounters["H"] = 1;
    componentCounters["F"] = 1;
    componentCounters["GND"] = 1;
    componentCounters["X"] = 1;

    setStatusBar(new QStatusBar(this));
    onFileNew();
    populateLibraryMenu();
}

MainWindow::~MainWindow()
{
    for(const auto& doc : openDocuments) {
        delete doc.circuit;
    }
}

void MainWindow::setupMenus()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(tr("&New"), this, &MainWindow::onFileNew);
    fileMenu->addAction(tr("&Open..."), this, &MainWindow::onFileOpen);
    fileMenu->addAction(tr("&Save"), this, &MainWindow::onFileSave);
    fileMenu->addAction(tr("Save &As..."), this, &MainWindow::onFileSaveAs);
    fileMenu->addAction(tr("Save As Subcircuit..."), this, &MainWindow::onSaveAsSubcircuit);
    fileMenu->addSeparator();
    fileMenu->addAction(tr("E&xit"), this, &QWidget::close);

    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(tr("Add &Resistor"), this, &MainWindow::onAddResistor);
    editMenu->addAction(tr("Add &Capacitor"), this, &MainWindow::onAddCapacitor);
    editMenu->addAction(tr("Add &Inductor"), this, &MainWindow::onAddInductor);
    editMenu->addSeparator();
    editMenu->addAction(tr("Add DC &Voltage Source"), this, &MainWindow::onAddVoltageSource);
    editMenu->addAction(tr("Add AC Voltage Source"), this, &MainWindow::onAddACVoltageSource);
    editMenu->addAction(tr("Add &Sinusoidal Source"), this, &MainWindow::onAddSinusoidalSource);
    editMenu->addAction(tr("Add &Pulse Source"), this, &MainWindow::onAddPulseSource);
    editMenu->addAction(tr("Add &Waveform Source..."), this, &MainWindow::onAddWaveformSource);
    editMenu->addAction(tr("Add &Wireless Source"), this, &MainWindow::onAddWirelessSource);
    editMenu->addAction(tr("Add DC &Current Source"), this, &MainWindow::onAddCurrentSource);
    editMenu->addSeparator();
    editMenu->addAction(tr("Add VCVS (E)"), this, &MainWindow::onAddVCVS);
    editMenu->addAction(tr("Add VCCS (G)"), this, &MainWindow::onAddVCCS);
    editMenu->addAction(tr("Add CCVS (H)"), this, &MainWindow::onAddCCVS);
    editMenu->addAction(tr("Add CCCS (F)"), this, &MainWindow::onAddCCCS);
    editMenu->addSeparator();
    editMenu->addAction(tr("Add &Ground"), this, &MainWindow::onAddGround);
    editMenu->addAction(tr("Add Node &Label"), this, &MainWindow::onAddNodeLabel);
    editMenu->addSeparator();
    QAction* wireAction = editMenu->addAction(tr("Add &Wire"));
    wireAction->setCheckable(true);
    connect(wireAction, &QAction::toggled, [this](bool checked){
        if(getCurrentEditor()) getCurrentEditor()->toggleWiringMode(checked);
    });

    QAction* probeAction = editMenu->addAction(tr("Voltage Probe"));
    probeAction->setCheckable(true);
    connect(probeAction, &QAction::toggled, [this](bool checked){
        if(getCurrentEditor()) {
            getCurrentEditor()->setEditorMode(checked ? SchematicEditor::EditorState::Probing : SchematicEditor::EditorState::Normal);
        }
    });

    libraryMenu = menuBar()->addMenu(tr("&Library"));
    libraryMenu->addAction(tr("Refresh Library"), this, &MainWindow::populateLibraryMenu);
    libraryMenu->addSeparator();

    QMenu *simulateMenu = menuBar()->addMenu(tr("&Simulate"));
    simulateMenu->addAction(tr("&Run Simulation..."), this, &MainWindow::onRunSimulation);

    menuBar()->addMenu(tr("&View"));
    m_networkMenu = menuBar()->addMenu(tr("&Network"));
    m_networkMenu->addAction(tr("Start Server..."), this, &MainWindow::onStartServer);
    m_networkMenu->addAction(tr("Connect to Server..."), this, &MainWindow::onConnectToServer);
    m_networkMenu->addAction(tr("Broadcast Signal..."), this, &MainWindow::onSelectSignalToBroadcast);
    menuBar()->addMenu(tr("Sco&pe"));
    menuBar()->addMenu(tr("&Help"));
}

SchematicEditor* MainWindow::getCurrentEditor() {
    if (tabWidget->count() == 0) return nullptr;
    return qobject_cast<SchematicEditor*>(tabWidget->currentWidget());
}

Circuit* MainWindow::getCurrentCircuit() {
    int index = tabWidget->currentIndex();
    if (index < 0 || index >= openDocuments.size()) return nullptr;
    return openDocuments[index].circuit;
}

QString MainWindow::getCurrentFilePath() {
    int index = tabWidget->currentIndex();
    if (index < 0 || index >= openDocuments.size()) return "";
    return openDocuments[index].filePath;
}

void MainWindow::onFileNew() {
    Circuit* newCircuit = new Circuit();
    SchematicEditor* newEditor = new SchematicEditor(newCircuit, this);
    newEditor->setMainWindow(this);

    Document newDoc = {newEditor, newCircuit, ""};
    openDocuments.push_back(newDoc);

    int newIndex = tabWidget->addTab(newEditor, "Untitled");
    tabWidget->setCurrentIndex(newIndex);
}

void MainWindow::openFileInNewTab(const QString& filePath) {
    for (int i = 0; i < openDocuments.size(); ++i) {
        if (openDocuments[i].filePath == filePath) {
            tabWidget->setCurrentIndex(i);
            return;
        }
    }

    Circuit* newCircuit = new Circuit();
    try {
        newCircuit->loadFromFile(filePath.toStdString());
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Load Error", e.what());
        delete newCircuit;
        return;
    }

    SchematicEditor* newEditor = new SchematicEditor(newCircuit, this);
    newEditor->setMainWindow(this);
    newEditor->populateSceneFromCircuit();

    Document newDoc = {newEditor, newCircuit, filePath};
    openDocuments.push_back(newDoc);

    QFileInfo fileInfo(filePath);
    int newIndex = tabWidget->addTab(newEditor, fileInfo.fileName());
    tabWidget->setCurrentIndex(newIndex);
}

void MainWindow::onFileOpen() {
    QString filePath = QFileDialog::getOpenFileName(this, "Open Circuit", "", "Circuit Files (*.cir *.sub);;All Files (*)");
    if (!filePath.isEmpty()) {
        openFileInNewTab(filePath);
    }
}

void MainWindow::onFileSave() {
    QString currentPath = getCurrentFilePath();
    if (currentPath.isEmpty()) {
        onFileSaveAs();
    } else {
        SchematicEditor* editor = getCurrentEditor();
        Circuit* circuit = getCurrentCircuit();
        if (!editor || !circuit) return;

        editor->updateCircuitWires();
        circuit->saveToFile(currentPath.toStdString());
    }
}

void MainWindow::onFileSaveAs() {
    SchematicEditor* editor = getCurrentEditor();
    Circuit* circuit = getCurrentCircuit();
    int index = tabWidget->currentIndex();
    if (!editor || !circuit || index < 0) return;

    QString filePath = QFileDialog::getSaveFileName(this, "Save As", "", "Circuit Files (*.cir);;Subcircuit Files (*.sub)");
    if (!filePath.isEmpty()) {
        editor->updateCircuitWires();
        circuit->saveToFile(filePath.toStdString());

        openDocuments[index].filePath = filePath;
        QFileInfo fileInfo(filePath);
        tabWidget->setTabText(index, fileInfo.fileName());
    }
}

void MainWindow::onSaveAsSubcircuit() {
    Circuit* circuit = getCurrentCircuit();
    if (!circuit) return;

    bool isResistive = true;
    for (const auto& comp : circuit->getComponents()) {
        if (dynamic_cast<Capacitor*>(comp.get()) || dynamic_cast<Inductor*>(comp.get()) ||
            dynamic_cast<ACVoltageSource*>(comp.get())) {
            isResistive = false;
            break;
        }
    }

    if (!isResistive) {
        QMessageBox::warning(this, "Thevenin Conversion Skipped",
                             "The circuit contains reactive components (L, C) or AC sources.\n"
                             "It will be saved as a standard subcircuit.");
        return;
    }
    SaveSubcircuitDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        try {
            QString fileName = dialog.getFileName();
            std::vector<int> ports = dialog.getExternalPorts();
            if (ports.size() != 2) {
                throw std::invalid_argument("Thevenin equivalent requires exactly two external ports.");
            }

            TheveninEquivalent th_eq = circuit->calculateTheveninEquivalent(ports[0], ports[1]);

            auto theveninCircuit = make_unique<Circuit>();
            theveninCircuit->addComponent(make_unique<VoltageSource>("Vth", 1, 2, th_eq.Vth));
            theveninCircuit->addComponent(make_unique<Resistor>("Rth", 2, 0, th_eq.Rth));
            theveninCircuit->setExternalPorts({1, 0}); // پورت‌های خروجی مدار جدید

            QDir dir(QCoreApplication::applicationDirPath());
            if (!dir.exists("library")) dir.mkdir("library");
            QString filePath = dir.filePath("library/" + fileName + ".sub");

            theveninCircuit->saveToFile(filePath.toStdString());
            QMessageBox::information(this, "Success",
                                     QString("Thevenin equivalent circuit saved successfully.\n"
                                             "Vth = %1 V, Rth = %2 Ohm")
                                             .arg(th_eq.Vth).arg(th_eq.Rth));
            populateLibraryMenu();

        } catch (const std::exception& e) {
            QMessageBox::critical(this, "Error", e.what());
        }
    }
}

void MainWindow::onTabClose(int index) {
    if (index < 0 || index >= openDocuments.size()) return;

    Document docToClose = openDocuments[index];
    delete docToClose.circuit;

    openDocuments.erase(openDocuments.begin() + index);
    tabWidget->removeTab(index);

    if (openDocuments.empty()) {
        onFileNew();
    }
}

void MainWindow::populateLibraryMenu() {
    if (!libraryMenu) return;

    QList<QAction*> actions = libraryMenu->actions();
    for (int i = 2; i < actions.size(); ++i) {
        libraryMenu->removeAction(actions[i]);
        delete actions[i];
    }

    QDir libraryDir(QCoreApplication::applicationDirPath() + "/library");
    if (!libraryDir.exists()) return;

    QStringList filters;
    filters << "*.sub";
    QFileInfoList subcircuitFiles = libraryDir.entryInfoList(filters, QDir::Files);

    for (const QFileInfo& fileInfo : subcircuitFiles) {
        QAction* addSubAction = new QAction(fileInfo.baseName(), this);
        QString fullPath = fileInfo.absoluteFilePath();

        connect(addSubAction, &QAction::triggered, this, [this, fullPath]() {
            Circuit* currentCircuit = getCurrentCircuit();
            SchematicEditor* currentEditor = getCurrentEditor();
            if (!currentCircuit || !currentEditor) return;

            try {
                string name = getNextComponentName("X");

                auto tempCircuit = make_unique<Circuit>();
                tempCircuit->loadFromFile(fullPath.toStdString());
                int portCount = tempCircuit->getExternalPorts().size();
                if (portCount == 0) portCount = 2;

                std::vector<int> initialNodes(portCount, -1);

                auto logic_comp = make_unique<SubCircuit>(name, initialNodes, fullPath.toStdString());
                Component* ptr = logic_comp.get();
                currentCircuit->addComponent(std::move(logic_comp));
                currentEditor->scene()->addItem(new SubCircuitItem(ptr));
            } catch (const std::exception& e) {
                QMessageBox::critical(this, "Error Loading Subcircuit", e.what());
            }
        });

        libraryMenu->addAction(addSubAction);
    }
}

std::string MainWindow::getNextComponentName(const std::string& prefix) {
    int count = componentCounters[prefix];
    componentCounters[prefix]++;
    return prefix + std::to_string(count);
}

void MainWindow::onAddResistor() {
    Circuit* circuit = getCurrentCircuit();
    SchematicEditor* editor = getCurrentEditor();
    if (!circuit || !editor) return;

    string name = getNextComponentName("R");
    auto logic_comp = make_unique<Resistor>(name, -1, -1, 1000.0);
    Component* ptr = logic_comp.get();
    circuit->addComponent(std::move(logic_comp));
    editor->scene()->addItem(new ResistorItem(ptr));
}

void MainWindow::onAddCapacitor() {
    Circuit* circuit = getCurrentCircuit();
    SchematicEditor* editor = getCurrentEditor();
    if (!circuit || !editor) return;

    string name = getNextComponentName("C");
    auto logic_comp = make_unique<Capacitor>(name, -1, -1, 1e-6);
    Component* ptr = logic_comp.get();
    circuit->addComponent(std::move(logic_comp));
    editor->scene()->addItem(new CapacitorItem(ptr));
}

void MainWindow::onAddInductor() {
    Circuit* circuit = getCurrentCircuit();
    SchematicEditor* editor = getCurrentEditor();
    if (!circuit || !editor) return;

    string name = getNextComponentName("L");
    auto logic_comp = make_unique<Inductor>(name, -1, -1, 1e-3);
    Component* ptr = logic_comp.get();
    circuit->addComponent(std::move(logic_comp));
    editor->scene()->addItem(new InductorItem(ptr));
}

void MainWindow::onAddVoltageSource() {
    Circuit* circuit = getCurrentCircuit();
    SchematicEditor* editor = getCurrentEditor();
    if (!circuit || !editor) return;

    string name = getNextComponentName("V");
    auto logic_comp = make_unique<VoltageSource>(name, -1, -1, 5.0);
    Component* ptr = logic_comp.get();
    circuit->addComponent(std::move(logic_comp));
    editor->scene()->addItem(new VoltageSourceItem(ptr));
}

void MainWindow::onAddACVoltageSource() {
    Circuit* circuit = getCurrentCircuit();
    SchematicEditor* editor = getCurrentEditor();
    if (!circuit || !editor) return;

    string name = getNextComponentName("V");
    auto logic_comp = make_unique<ACVoltageSource>(name, -1, -1, 1.0, 0.0);
    Component* ptr = logic_comp.get();
    circuit->addComponent(std::move(logic_comp));
    editor->scene()->addItem(new VoltageSourceItem(ptr));
}

void MainWindow::onAddCurrentSource() {
    Circuit* circuit = getCurrentCircuit();
    SchematicEditor* editor = getCurrentEditor();
    if (!circuit || !editor) return;

    string name = getNextComponentName("I");
    auto logic_comp = make_unique<CurrentSource>(name, -1, -1, 1.0);
    Component* ptr = logic_comp.get();
    circuit->addComponent(std::move(logic_comp));
    editor->scene()->addItem(new CurrentSourceItem(ptr));
}

void MainWindow::onAddWaveformSource() {
    Circuit* circuit = getCurrentCircuit();
    SchematicEditor* editor = getCurrentEditor();
    if (!circuit || !editor) return;

    QString filePath = QFileDialog::getOpenFileName(this,
                                                    tr("Open Waveform File"), "", tr("Text Files (*.txt);;All Files (*)"));

    if (!filePath.isEmpty()) {
        try {
            string name = getNextComponentName("V");

            auto logic_comp = make_unique<WaveformVoltageSource>(name, -1, -1, filePath.toStdString());
            Component* ptr = logic_comp.get();
            circuit->addComponent(std::move(logic_comp));

            editor->scene()->addItem(new VoltageSourceItem(ptr)); // از همان آیتم گرافیکی منبع ولتاژ استفاده می‌کنیم

        } catch (const std::exception& e) {
            QMessageBox::critical(this, "Error", e.what());
        }
    }
}

void MainWindow::onAddGround() {
    Circuit* circuit = getCurrentCircuit();
    SchematicEditor* editor = getCurrentEditor();
    if (!circuit || !editor) return;

    string name = getNextComponentName("GND");
    auto logic_comp = make_unique<Ground>(name, 0);
    Component* ptr = logic_comp.get();
    circuit->addComponent(std::move(logic_comp));
    editor->scene()->addItem(new GroundItem(ptr));
}

void MainWindow::onAddVCVS() {
    Circuit* circuit = getCurrentCircuit();
    SchematicEditor* editor = getCurrentEditor();
    if (!circuit || !editor) return;

    string name = getNextComponentName("E");
    auto logic_comp = make_unique<VCVS>(name, -1, -1, -1, -1, 2.0);
    Component* ptr = logic_comp.get();
    circuit->addComponent(std::move(logic_comp));
    editor->scene()->addItem(new VCVSItem(ptr));
}

void MainWindow::onAddVCCS() {
    Circuit* circuit = getCurrentCircuit();
    SchematicEditor* editor = getCurrentEditor();
    if (!circuit || !editor) return;

    string name = getNextComponentName("G");
    auto logic_comp = make_unique<VCCS>(name, -1, -1, -1, -1, 0.1);
    Component* ptr = logic_comp.get();
    circuit->addComponent(std::move(logic_comp));
    editor->scene()->addItem(new VCCSItem(ptr));
}

void MainWindow::onAddCCVS() {
    Circuit* circuit = getCurrentCircuit();
    SchematicEditor* editor = getCurrentEditor();
    if (!circuit || !editor) return;

    string name = getNextComponentName("H");
    auto logic_comp = make_unique<CCVS>(name, -1, -1, "Vdummy", 10.0);
    Component* ptr = logic_comp.get();
    circuit->addComponent(std::move(logic_comp));
    editor->scene()->addItem(new CCVSItem(ptr));
}

void MainWindow::onAddCCCS() {
    Circuit* circuit = getCurrentCircuit();
    SchematicEditor* editor = getCurrentEditor();
    if (!circuit || !editor) return;

    string name = getNextComponentName("F");
    auto logic_comp = make_unique<CCCS>(name, -1, -1, "Vdummy", 5.0);
    Component* ptr = logic_comp.get();
    circuit->addComponent(std::move(logic_comp));
    editor->scene()->addItem(new CCCSItem(ptr));
}

void MainWindow::onAddSinusoidalSource() {
    Circuit* circuit = getCurrentCircuit();
    SchematicEditor* editor = getCurrentEditor();
    if (!circuit || !editor) return;

    string name = getNextComponentName("V");
    auto tempSource = SinusoidalVoltageSource(name, -1, -1, 0, 5, 1000);
    PropertiesDialog dialog(tempSource.getProperties(), this);
    if (dialog.exec() == QDialog::Accepted) {
        try {
            auto props = dialog.getProperties();
            auto logic_comp = make_unique<SinusoidalVoltageSource>(name, -1, -1, props["Offset"], props["Amplitude"], props["Frequency"]);
            Component* ptr = logic_comp.get();
            circuit->addComponent(std::move(logic_comp));
            editor->scene()->addItem(new VoltageSourceItem(ptr));
        } catch (const std::exception& e) {
            QMessageBox::warning(this, "Invalid Value", e.what());
        }
    }
}

void MainWindow::onAddPulseSource() {
    Circuit* circuit = getCurrentCircuit();
    SchematicEditor* editor = getCurrentEditor();
    if (!circuit || !editor) return;

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
            circuit->addComponent(std::move(logic_comp));
            editor->scene()->addItem(new VoltageSourceItem(ptr));
        } catch (const std::exception& e) {
            QMessageBox::warning(this, "Invalid Value", e.what());
        }
    }
}

void MainWindow::onRunSimulation() {
    Circuit* circuit = getCurrentCircuit();
    SchematicEditor* editor = getCurrentEditor();
    if (!circuit || !editor) return;

    SimulationDialog simDialog(this);
    if (simDialog.exec() == QDialog::Accepted) {
        editor->updateBackendNodes();
        try {
            if (m_scopeWindow) {
                m_scopeWindow->close();
                delete m_scopeWindow;
                m_scopeWindow = nullptr;
            }

            int tabIndex = simDialog.getCurrentTabIndex();
            QString xAxisTitle;

            qDebug() << "[DEBUG] Starting analysis...";

            if (tabIndex == 0) {
                circuit->runTransientAnalysis(simDialog.getStopTime(), simDialog.getTimeStep(), {}, simDialog.getStartTime());
                xAxisTitle = "Time (s)";
            } else if (tabIndex == 1) {
                circuit->runACAnalysis(simDialog.getStartFreq(), simDialog.getStopFreq(), simDialog.getNumPoints(), simDialog.getSweepType());
                xAxisTitle = "Frequency (Hz)";
            } else if (tabIndex == 2) {
                circuit->runPhaseAnalysis(simDialog.getBaseFreq(), simDialog.getStartPhase(), simDialog.getStopPhase(), simDialog.getNumPointsPhase());
                xAxisTitle = "Phase (deg)";
            }

            // ------------ ایستگاه بازرسی ۲ ------------
            qDebug() << "[DEBUG] Analysis finished. Getting results...";

            const auto& allResults = circuit->getSimulationResults();
            if (allResults.empty() || allResults.begin()->second.empty()) {
                QMessageBox::information(this, "Simulation Info", "Simulation ran, but no data was produced.");
                return;
            }

            // ------------ ایستگاه بازرسی ۳ ------------
            qDebug() << "[DEBUG] Results are valid. Creating PlotSelectionDialog...";

            QStringList availablePlots;
            for(const auto& pair : allResults) {
                if (pair.first != "Time" && pair.first != "Frequency" && pair.first != "Phase") {
                    availablePlots.append(QString::fromStdString(pair.first));
                }
            }

            PlotSelectionDialog plotDialog(availablePlots, this);

            // ------------ ایستگاه بازرسی ۴ ------------
            qDebug() << "[DEBUG] PlotSelectionDialog created. Executing...";

            if (plotDialog.exec() == QDialog::Accepted) {
                // ------------ ایستگاه بازرسی ۵ ------------
                qDebug() << "[DEBUG] PlotSelectionDialog accepted. Processing selected plots...";

                QStringList selectedPlotNames = plotDialog.getSelectedPlots();
                if (selectedPlotNames.isEmpty()) {
                    return;
                }

                std::map<std::string, std::vector<double>> selectedResults;
                if (allResults.count("Time")) selectedResults["Time"] = allResults.at("Time");
                if (allResults.count("Frequency")) selectedResults["Frequency"] = allResults.at("Frequency");
                if (allResults.count("Phase")) selectedResults["Phase"] = allResults.at("Phase");

                for (const QString& name : selectedPlotNames) {
                    selectedResults[name.toStdString()] = allResults.at(name.toStdString());
                }

                // ------------ ایستگاه بازرسی ۶ ------------
                qDebug() << "[DEBUG] Creating ScopeWindow...";

                m_scopeWindow = new ScopeWindow(selectedResults, xAxisTitle, this);

                // ------------ ایستگاه بازرسی ۷ ------------
                qDebug() << "[DEBUG] Showing ScopeWindow...";

                m_scopeWindow->show();

                // ------------ ایستگاه بازرسی ۸ ------------
                qDebug() << "[DEBUG] ScopeWindow is shown.";
            } else {
                qDebug() << "[DEBUG] PlotSelectionDialog was cancelled or closed.";
            }

        } catch (const std::exception& e) {
            QMessageBox::critical(this, "Simulation Error", e.what());
        }
    }
}

void MainWindow::plotVariable(const QString& varName)
{
    if (!m_scopeWindow || !m_scopeWindow->isVisible()) {
        QMessageBox::information(this, "Probe Info",
                                 "Please run a simulation to open the scope window before using the probe.");
        return;
    }

    Circuit* circuit = getCurrentCircuit();
    if (!circuit) return;

    const auto& allResults = circuit->getSimulationResults();
    if (allResults.find(varName.toStdString()) == allResults.end()) {
        QMessageBox::warning(this, "Probe Error", "Variable '" + varName + "' not found in the simulation results.");
        return;
    }

    m_scopeWindow->addSeries(varName, allResults.at(varName.toStdString()));
    m_scopeWindow->activateWindow();
}

void MainWindow::onAddNodeLabel()
{
    SchematicEditor* editor = getCurrentEditor();
    if (!editor) return;
    string name = getNextComponentName("LBL");
    NodeLabelItem *label = new NodeLabelItem(QString::fromStdString(name));

    label->setPos(editor->mapToScene(editor->viewport()->rect().center()));
    editor->scene()->addItem(label);
}

void MainWindow::onStartServer()
{
    bool ok;
    quint16 port = QInputDialog::getInt(this, "Start Server", "Enter port number:", 8080, 1024, 65535, 1, &ok);
    if (ok) {
        m_server->start(port);
        const QHostAddress &localhost = QHostAddress(QHostAddress::LocalHost);
        for (const QHostAddress &address: QNetworkInterface::allAddresses()) {
            if (address.protocol() == QAbstractSocket::IPv4Protocol && address != localhost)
                logNetworkMessage("Your local IP is: " + address.toString());
        }
    }
}

void MainWindow::logNetworkMessage(const QString& msg)
{
    statusBar()->showMessage(msg, 5000);
}

void MainWindow::onConnectToServer()
{
    bool ok;
    QString ip = QInputDialog::getText(this, "Connect to Server", "Enter server IP:", QLineEdit::Normal, "127.0.0.1", &ok);
    if (ok && !ip.isEmpty()) {
        quint16 port = QInputDialog::getInt(this, "Connect to Server", "Enter port:", 8080, 1024, 65535, 1, &ok);
        if (ok) {
            m_client->connectToServer(ip, port);
        }
    }
}

void MainWindow::onAddWirelessSource()
{
    if (m_wirelessSource) {
        QMessageBox::warning(this, "Error", "Only one wireless source can be added to the circuit.");
        return;
    }

    Circuit* circuit = getCurrentCircuit();
    SchematicEditor* editor = getCurrentEditor();
    if (!circuit || !editor) return;

    string name = getNextComponentName("V");
    auto logic_comp = make_unique<WirelessVoltageSource>(name, -1, -1);
    m_wirelessSource = logic_comp.get();
    Component* ptr = logic_comp.get();
    circuit->addComponent(std::move(logic_comp));
    editor->scene()->addItem(new VoltageSourceItem(ptr));
}

void MainWindow::onVoltageReceived(double voltage)
{
    if (m_wirelessSource) {
        m_wirelessSource->setWirelessVoltage(voltage);
        if (auto editor = getCurrentEditor()) {
            editor->scene()->update();
        }
    }
}

void MainWindow::onSelectSignalToBroadcast()
{
    Circuit* circuit = getCurrentCircuit();
    if (!circuit) return;

    const auto& allResults = circuit->getSimulationResults();
    if (allResults.empty()) {
        QMessageBox::warning(this, "Error", "No simulation data available to broadcast.");
        return;
    }

    QStringList availablePlots;
    for(const auto& pair : allResults) {
        if (pair.first != "Time" && pair.first != "Frequency" && pair.first != "Phase") {
            availablePlots.append(QString::fromStdString(pair.first));
        }
    }

    bool ok;
    QString selectedSignal = QInputDialog::getItem(this, "Select Signal",
                                                   "Choose a signal to broadcast:", availablePlots, 0, false, &ok);

    if (ok && !selectedSignal.isEmpty()) {
        m_signalToBroadcast = allResults.at(selectedSignal.toStdString());
        m_broadcastIndex = 0;
        m_broadcastTimer->start(30);
        logNetworkMessage("Broadcasting signal: " + selectedSignal);
    }
}

void MainWindow::onBroadcastSignal()
{
    if (m_signalToBroadcast.empty() || !m_server) {
        m_broadcastTimer->stop();
        return;
    }
    double value = m_signalToBroadcast[m_broadcastIndex];
    m_server->sendToClient(QString::number(value));
    m_broadcastIndex++;
    if (m_broadcastIndex >= m_signalToBroadcast.size()) {
        m_broadcastIndex = 0;
    }
}
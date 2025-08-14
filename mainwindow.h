#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <map>
#include <string>
#include <vector>
#include "server.h"
#include "client.h"
#include <QTimer>

class WirelessVoltageSource;
class SchematicEditor;
class Circuit;
class ScopeWindow;
class QMenu;
class QTabWidget;

class MainWindow : public QMainWindow
{
Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void openFileInNewTab(const QString& filePath);
    void plotVariable(const QString& varName);

public slots:
    void onFileNew();
    void onFileOpen();
    void onFileSave();
    void onFileSaveAs();
    void onSaveAsSubcircuit();
    void onTabClose(int index);
    void onRunSimulation();

    void onAddResistor();
    void onAddCapacitor();
    void onAddInductor();
    void onAddVoltageSource();
    void onAddACVoltageSource();
    void onAddSinusoidalSource();
    void onAddPulseSource();
    void onAddCurrentSource();
    void onAddGround();
    void onAddVCVS();
    void onAddVCCS();
    void onAddCCVS();
    void onAddCCCS();
    void onAddWaveformSource();
    void onAddNodeLabel();
    void onStartServer();
    void onConnectToServer();
    void onAddWirelessSource();
    void onVoltageReceived(double voltage);
    void onBroadcastSignal();
    void onSelectSignalToBroadcast();
    void logNetworkMessage(const QString& msg);

private:
    void setupMenus();
    void populateLibraryMenu();
    Server* m_server = nullptr;
    QMenu* m_networkMenu = nullptr;
    QTimer* m_broadcastTimer = nullptr;
    std::vector<double> m_signalToBroadcast;
    int m_broadcastIndex = 0;
    std::string getNextComponentName(const std::string& prefix);

    Client* m_client = nullptr;
    WirelessVoltageSource* m_wirelessSource = nullptr;

    SchematicEditor* getCurrentEditor();
    Circuit* getCurrentCircuit();
    QString getCurrentFilePath();

    struct Document {
        SchematicEditor* editor;
        Circuit* circuit;
        QString filePath;
    };

    QTabWidget *tabWidget;
    std::vector<Document> openDocuments;

    ScopeWindow *m_scopeWindow = nullptr;
    QMenu* libraryMenu = nullptr;
    std::map<std::string, int> componentCounters;
};

#endif // MAINWINDOW_H

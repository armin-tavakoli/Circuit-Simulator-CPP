#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <map>
#include <string>
#include <vector>

// Forward declarations
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

private slots:
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

private:
    void setupMenus();
    void populateLibraryMenu();
    std::string getNextComponentName(const std::string& prefix);

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

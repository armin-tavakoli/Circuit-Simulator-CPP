// MainWindow.h

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "CircuitView.h"

// Forward declaration
class QAction;
class QMenu;

class MainWindow : public QMainWindow {
Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void newFile();
    void runTransientAnalysis();

private:
    void createActions();
    void createMenus();
    void createToolbars();

    // Actions
    QAction *newAction;
    QAction *exitAction;
    QAction *transientAction;

    // Menus
    QMenu *fileMenu;
    CircuitView *view;
    QMenu *simulateMenu;
};

#endif // MAINWINDOW_H
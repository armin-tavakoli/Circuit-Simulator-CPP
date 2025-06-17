// MainWindow.cpp

#include "MainWindow.h"
#include "CircuitView.h"
#include "TransientAnalysisDialog.h"
#include "PlotWindow.h"
#include <QApplication>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QIcon>
#include <QKeySequence>
#include <QMessageBox>
#include <QDebug>

// Constructor
MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent) {
    // 1. تنظیمات اولیه پنجره
    setWindowTitle("Circuit Simulator");
    resize(1280, 720);

    // 2. تنظیم ویجت مرکزی برای رسم شماتیک
    view = new CircuitView(this);
    setCentralWidget(view);

    // 3. ایجاد اکشن‌ها، منوها و نوار ابزار
    createActions();
    createMenus();
    createToolbars();

    // 4. تنظیم پیام اولیه نوار وضعیت
    statusBar()->showMessage(tr("Ready"));
}

// Destructor
MainWindow::~MainWindow() {
}

// Private slots
void MainWindow::newFile() {
    // در آینده این تابع مدار جدید را ایجاد خواهد کرد.
    QMessageBox::information(this, tr("New File"), tr("Functionality to create a new file will be implemented here."));
}

void MainWindow::runTransientAnalysis() {
    // یک نمونه از دیالوگ خود را می‌سازیم
    TransientAnalysisDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        double stopTime = dialog.stopTime();
        double startTime = dialog.startTime();
        double maxTimestep = dialog.maxTimestep();
        qDebug() << "Starting Transient Analysis with:";
        qDebug() << "Stop Time:" << stopTime;
        qDebug() << "Start Time:" << startTime;
        qDebug() << "Max Timestep:" << maxTimestep;
        auto *plotWindow = new PlotWindow(this);
        plotWindow->setAttribute(Qt::WA_DeleteOnClose);
        plotWindow->show();
    }
}

// UI creation methods
void MainWindow::createActions() {
    // Action for New File
    newAction = new QAction(QIcon(":/icons/new.png"), tr("&New"), this);
    newAction->setShortcuts(QKeySequence::New);
    newAction->setStatusTip(tr("Create a new circuit file"));
    connect(newAction, &QAction::triggered, this, &MainWindow::newFile);

    // Action for Exit
    exitAction = new QAction(tr("E&xit"), this);
    exitAction->setShortcuts(QKeySequence::Quit);
    exitAction->setStatusTip(tr("Exit the application"));
    connect(exitAction, &QAction::triggered, qApp, &QApplication::quit);

    // Action for Transient Analysis
    transientAction = new QAction(tr("Run &Transient Analysis..."), this);
    transientAction->setStatusTip(tr("Perform a transient analysis"));
    connect(transientAction, &QAction::triggered, this, &MainWindow::runTransientAnalysis);
}

void MainWindow::createMenus() {
    // File Menu
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newAction);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);

    // Simulate Menu
    simulateMenu = menuBar()->addMenu(tr("&Simulate"));
    simulateMenu->addAction(transientAction);
}

void MainWindow::createToolbars() {
    // File Toolbar
    QToolBar *fileToolBar = addToolBar(tr("File"));
    fileToolBar->addAction(newAction);
}
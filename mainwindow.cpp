#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    mainwindow_ui(new Ui::MainWindow)
{
    mainwindow_ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete mainwindow_ui;
}

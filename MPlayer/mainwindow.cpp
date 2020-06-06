#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "v4l2capture.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_2_clicked()
{
    v4l2Capture cap;
    QString info = cap.enumDevices("/dev/video0");

    ui->textEdit->setPlainText(info);
}

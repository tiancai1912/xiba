#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QCamera>
#include <QCameraInfo>

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

void MainWindow::on_mBtnEnumDevice_clicked()
{
    QList<QCameraInfo> infos = QCameraInfo::availableCameras();
    for(QCameraInfo info : infos) {
        ui->mEnumDevice->addItem(info.deviceName());
    }
}

void MainWindow::on_mEnumDevice_currentIndexChanged(const QString &arg1)
{
    QList<QCameraInfo> infos = QCameraInfo::availableCameras();
    for(QCameraInfo info : infos) {
        if (info.deviceName() == arg1) {
            QCamera *camera = new QCamera(info);
        }
    }
}

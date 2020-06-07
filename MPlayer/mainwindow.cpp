#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "v4l2capture.h"

#include <QFileDialog>
#include <QMessageBox>

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

//enum device cap
void MainWindow::on_pushButton_2_clicked()
{
//    v4l2Capture cap;
    QString info = m_cap.enumDevices("/dev/video0");

    ui->textEdit->setPlainText(info);
}

//capture a frame
void MainWindow::on_pushButton_clicked()
{
    m_cap.init("/dev/video0");
    m_cap.setCallback(&m_handle);
    m_cap.startCapture();
//    m_cap.getOneFrame();
}

//open image
void MainWindow::on_pushButton_3_clicked()
{
    QString filename=QFileDialog::getOpenFileName(this,tr("选择图像"),"",tr("Images (*.png *.bmp *.jpg *.yuv)"));
        if(filename.isEmpty())
            return;
        else
        {
            QImage img;
            if(!(img.load(filename))) //加载图像
            {
                QMessageBox::information(this, tr("打开图像失败"),tr("打开图像失败!"));
                return;
            }
            ui->label->setPixmap(QPixmap::fromImage(img.scaled(ui->label->size())));
        }
}

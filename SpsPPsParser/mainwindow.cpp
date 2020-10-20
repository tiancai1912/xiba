#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <string>

#define COLUMENUM 2

#define COLUME_INDEX 0
#define COLUME_TYPE 1

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    mParser = new VideoParser();
    initTable();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionOpen_triggered()
{
    mParser->openFile("test.mp4");

    int count = 0;
    while(count < MAX_NALU_READ_SIZE) {
        VideoParser::PacketItem *item = mParser->readNaluItems();
        if (item != NULL) {
           int type = mParser->parseNaluItem(item);
           switch (type) {
           case NALU_SLICE:
    //            qDebug() << "Slice Nalu" << endl;
               setTableItem(count, "slice");
               break;
           case NALU_IDR:
               qDebug() << "IDR Nalu" << endl;
               setTableItem(count, "idr");
               break;
           case NALU_SEI:
               qDebug() << "SEI Nalu" << endl;
               setTableItem(count, "sei");
               break;
           case NALU_SPS:
               qDebug() << "SPS Nalu" << endl;
               setTableItem(count, "sps");
               break;
           case NALU_PPS:
               qDebug() << "PPS Nalu" << endl;
               setTableItem(count, "pps");
               break;
           default:
               qDebug() << "Unknown Nalu" << endl;
               setTableItem(count, "unknown");
               break;
           }

           count ++;
        }
    }
}



void MainWindow::initTable()
{
    QStringList headers;
    headers << "Index" << "Nalu Type";

    ui->mTable->setColumnCount(headers.count());
    ui->mTable->setRowCount(MAX_NALU_READ_SIZE);
    ui->mTable->setHorizontalHeaderLabels(headers);
}

void MainWindow::setTableItem(int index, char *type)
{
    qDebug() << index << type << endl;
    QTableWidgetItem *item;

    for (int j = 0; j < COLUMENUM; j++) {
        if (j == COLUME_INDEX) {
            item = new QTableWidgetItem(std::to_string(index).c_str());
            item->setBackgroundColor(Qt::red);
            ui->mTable->setItem(index, j, item);
        } else if (j == COLUME_TYPE) {
            item = new QTableWidgetItem(type);
            item->setBackgroundColor(Qt::green);
            ui->mTable->setItem(index, j, item);
        }
    }
}

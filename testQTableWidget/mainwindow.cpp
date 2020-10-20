#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>

#define COLUMENUM 3
#define ROWNUM 10

#define COLUME_NAME 0
#define COLUME_AGE 1
#define COLUME_SEX 2

static const char * g_name[ROWNUM] =
{
    "zhangsan1",
    "zhangsan2",
    "zhangsan3",
    "zhangsan4",
    "zhangsan5",
    "zhangsan6",
    "zhangsan7",
    "zhangsan8",
    "zhangsan9",
    "zhangsan10"
};

static const char * g_age[ROWNUM] =
{
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "10"
};

static const char * g_sex[ROWNUM] =
{
    "boy",
    "girl",
    "boy",
    "girl",
    "boy",
    "girl",
    "boy",
    "girl",
    "boy",
    "girl"
};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    mParser = new VideoParser();
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_mBtnSetHeader_clicked()
{
    QStringList headers;
    headers << "name" << "age" << "sex";

    ui->mTable->setColumnCount(headers.count());

    QTableWidgetItem *headerItem;
    //func1
    for (int i = 0; i < headers.count(); i++) {
        headerItem = new QTableWidgetItem(headers.at(i));
        QFont font = headerItem->font();
        font.setBold(true);
        font.setPointSize(12);
        headerItem->setFont(font);
        headerItem->setTextColor(Qt::blue);
        ui->mTable->setHorizontalHeaderItem(i, headerItem);
    }

    //func2
//    ui->mTable->setHorizontalHeaderLabels(headers);
}

void MainWindow::on_mBtnSetRows_clicked()
{
    ui->mTable->setRowCount(ROWNUM);
}

void MainWindow::on_mBtnInsertRow_clicked()
{
    ui->mTable->insertRow(2);
}

void MainWindow::on_mBtnAddRow_clicked()
{
    ui->mTable->insertRow(ui->mTable->rowCount());
}

void MainWindow::on_mBtnDeleteRow_clicked()
{
    qDebug() << "current row count: " << ui->mTable->rowCount();
    qDebug() << "current row: " << ui->mTable->currentRow();
    ui->mTable->removeRow(ui->mTable->currentRow());
//    ui->mTable->removeRow(ui->mTable->rowCount() - 1);
}

void MainWindow::on_mBtnSetItem_clicked()
{
    QModelIndex index = ui->mTable->currentIndex();
    qDebug() << index.column() << index.row();

    QTableWidgetItem *item;

    for (int j = 0; j < COLUMENUM; j++) {
        for (int i = 0; i < ROWNUM; i++) {
            if (j == COLUME_NAME) {
                item = new QTableWidgetItem(g_name[i]);
                item->setBackgroundColor(Qt::red);
                ui->mTable->setItem(i, j, item);
            } else if (j == COLUME_AGE) {
                item = new QTableWidgetItem(g_age[i]);
                item->setBackgroundColor(Qt::green);
                ui->mTable->setItem(i, j, item);
            } else if (j == COLUME_SEX) {
                item = new QTableWidgetItem(g_sex[i]);
                item->setBackgroundColor(Qt::blue);
                ui->mTable->setItem(i, j, item);
            }
        }
    }

}

void MainWindow::on_pushButton_clicked()
{
    mParser->openFile("test.mp4");
    int num = 0;
    while(num < 3000) {
        VideoParser::PacketItem *item = mParser->readNaluItems();
        mParser->parseNaluItem(item);
        delete item;
        item = NULL;
        num++;
    }

    mParser->closeFile();
}

void MainWindow::on_mBtnHidNum_clicked()
{
    ui->mTable->verticalHeader()->setHidden(true);
}

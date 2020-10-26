#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidgetItem>

#include "videoparser.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void initTable();
    void setTableItem(int index, char *type);

    void setTreeItem();

    void setSpsTreeItem(sps_t *sps);
    QTreeWidgetItem * addItem(QTreeWidgetItem *src, QString dst);

private slots:
    void on_actionOpen_triggered();

    virtual void dragEnterEvent(QDragEnterEvent* event);
    virtual void dropEvent(QDropEvent *event);

private:
    Ui::MainWindow *ui;

    VideoParser *mParser;
};

#endif // MAINWINDOW_H

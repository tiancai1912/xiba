#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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

private slots:
    void on_actionOpen_triggered();

private:
    Ui::MainWindow *ui;

    VideoParser *mParser;
};

#endif // MAINWINDOW_H

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_mBtnSetHeader_clicked();

    void on_mBtnSetRows_clicked();

    void on_mBtnInsertRow_clicked();

    void on_mBtnAddRow_clicked();

    void on_mBtnDeleteRow_clicked();

    void on_mBtnSetItem_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H

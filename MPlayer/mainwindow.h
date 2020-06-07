#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "v4l2capture.h"

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
    void on_pushButton_2_clicked();

    void on_pushButton_clicked();

    void on_pushButton_3_clicked();

private:
    Ui::MainWindow *ui;

    v4l2Capture m_cap;

    HandleFrame m_handle;
};

#endif // MAINWINDOW_H

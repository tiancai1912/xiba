#include "mainwindow.h"
#include <QApplication>

#include "utils.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

//    MyThread thread;
//    thread.init();
//    thread.start();

    return a.exec();
}

#include <QCoreApplication>

#include "mytimer.h"

#include <QDebug>

static int g_count = 0;

static void handle() {
    g_count ++;
    qDebug() << "timer out : " <<g_count << endl;;
}

int main(int argc, char *argv[])
{
    MyTimer timer("mytimer");
//    timer.Start(1000, () -> {

//                }, true, true);

    timer.Start(1000, handle, true, true);

    while (g_count >= 10) {
        timer.Cancel();
    }


    getchar();
    return 0;
}

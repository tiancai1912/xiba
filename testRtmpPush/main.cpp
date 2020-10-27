#include <QCoreApplication>
#include <QDebug>
#include "rtmppush.h"
#include <unistd.h>


int main(int argc, char *argv[])
{
    qDebug() << "hello world" << endl;

    RtmpPush *push = new RtmpPush();
    push->openFile("test.mp4");
    push->start();

    sleep(10);
    push->stop();
    push->closeFile();

    return 0;
}

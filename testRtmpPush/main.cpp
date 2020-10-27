#include <QCoreApplication>
#include <QDebug>
#include "rtmppush.h"
#include <unistd.h>


int main(int argc, char *argv[])
{
    qDebug() << "hello world" << endl;

    RtmpPush *push = new RtmpPush();
    push->start();

    push->openFile("test.mp4");
    push->closeFile();

    sleep(5);
    push->stop();

    return 0;
}

#include <QCoreApplication>

#include <QDebug>
#include <sys/types.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
//#include <fcntl.h>

int main(int argc, char *argv[])
{
    char buf[] = "a write to stdout\n";
    if (write(STDOUT_FILENO, buf, sizeof(buf) - 1) != (sizeof(buf) - 1)) {
        qDebug() << "errors in write\n";
    }

    printf("before fork\n");

    int num = 1;

    pid_t pid = fork();
    if (pid == 0) {
        printf("this is child\n");
        num += 1;
        printf("child num is : %d\n", num);
    } else {
        printf("this is parent\n");
        num += 2;
        printf("parent num is : %d\n", num);
    }

    printf("this is main process\n");
    printf("main process num is : %d\n", num);

    return 0;
}

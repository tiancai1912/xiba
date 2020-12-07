#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <thread>

class MyThread
{
public:

    class ThreadRun
    {
    public:
        virtual void run() = 0;
    };

    MyThread(ThreadRun *run);

    int start();
    void stop();

private:
    ThreadRun *mRun;
    std::thread mThread;
};

#endif // MYTHREAD_H

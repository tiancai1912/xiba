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
    MyThread();

    int start();
    void stop();

private:
    void run();

private:
    std::thread *m_thread;
    ThreadRun *m_run;
};

#endif // MYTHREAD_H

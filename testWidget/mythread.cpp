#include "mythread.h"

MyThread::MyThread(ThreadRun *run) :
    m_run(run)
{

}

MyThread::MyThread()
{

}

int MyThread::start()
{
    m_thread = new std::thread(&MyThread::run, this);
    return 0;
}

void MyThread::stop()
{
    if (m_thread->joinable()) {
        m_thread->join();
        delete m_thread;
        m_thread = nullptr;
    }
}

void MyThread::run()
{
    m_run->run();
}

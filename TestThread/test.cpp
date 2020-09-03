#include "test.h"
#include <QDebug>

ThreadImpl::ThreadImpl()
{

}

void ThreadImpl::start()
{
    m_thread = new std::thread(std::bind(&ThreadImpl::run_inner, this));
//    m_thread->join();
}

void ThreadImpl::wait()
{
    m_thread->join();
}

void ThreadImpl::run_inner()
{
    run();
}


Test::Test()
{

}

void Test::run()
{
    qDebug() << "hello thread\n";
}



#ifndef TEST_H
#define TEST_H

#include <thread>

class ThreadImpl
{
public:
    ThreadImpl();

    void start();
    void wait();

    void run_inner();

protected:
    virtual void run() = 0;

private:

    std::thread *m_thread;
};


class Test : public ThreadImpl
{
public:
    Test();


protected:
    virtual void run();


};

#endif // TEST_H

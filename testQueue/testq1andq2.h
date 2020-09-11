#ifndef TESTQ1ANDQ2_H
#define TESTQ1ANDQ2_H


#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>

class testQ1AndQ2
{
public:
    testQ1AndQ2();

    int startRawThread();
    void stopRawThread();

    int startHandleThread();
    void stopHandleThread();

    void pushRawItem(int item);
    int popRawItem();

    void pushHandleItem(int item);
    int popHandleItem();

private:
    void handleRawData();
    void handleHandleData();

private:

    std::thread *mRawThread;
    std::thread *mHandleThread;

    std::mutex m_mutex;
    std::condition_variable m_con_handle;
    std::condition_variable m_con_raw;

    std::queue<int> m_queue_raw;
    std::queue<int> m_queue_handle;
};

#endif // TESTQ1ANDQ2_H

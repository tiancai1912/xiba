#include "testq1andq2.h"
#include <QDebug>
#include <unistd.h>

testQ1AndQ2::testQ1AndQ2()
{
    for(int i = 0; i < 10; i++) {
        m_queue_handle.push(i);

        mVideoHandle.push(new Mp4File::PacketItem());
        mAudioHandle.push(new Mp4File::PacketItem());
    }
}

int testQ1AndQ2::startRawThread()
{
    mRawThread = new std::thread(std::bind(&testQ1AndQ2::handleRawData, this));
}

void testQ1AndQ2::stopRawThread()
{
    mRawThread->join();
}

int testQ1AndQ2::startHandleThread()
{
    mHandleThread = new std::thread(std::bind(&testQ1AndQ2::handleHandleData, this));
}

void testQ1AndQ2::stopHandleThread()
{
    mHandleThread->join();
}

void testQ1AndQ2::pushRawItem(int item)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_queue_raw.push(item);

    lock.unlock();
    m_con_handle.notify_one();
}

int testQ1AndQ2::popRawItem()
{
    std::unique_lock<std::mutex> lock(m_mutex);

    while (m_queue_raw.empty()) {
        m_con_handle.wait(lock);
    }

    int item = m_queue_raw.front();
    m_queue_raw.pop();
    lock.unlock();

    return item;
}

void testQ1AndQ2::pushHandleItem(int item)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_queue_handle.push(item);

    lock.unlock();
    m_con_raw.notify_one();
}

int testQ1AndQ2::popHandleItem()
{
    std::unique_lock<std::mutex> lock(m_mutex);

    while (m_queue_handle.empty()) {
        m_con_raw.wait(lock);
    }

    int item = m_queue_handle.front();
    m_queue_handle.pop();
    lock.unlock();

    return item;
}

void testQ1AndQ2::handleRawData()
{
    while(1) {
        int item = popHandleItem();
        qDebug() << "get handle item: " << item << endl;

        pushRawItem(item);

        sleep(1);
    }
}

void testQ1AndQ2::handleHandleData()
{
    while(1) {
        int item1 = popRawItem();
        qDebug() << "get raw item: " << item1 << endl;

        pushHandleItem(item1);

        sleep(1);
    }
}

///////
void testQ1AndQ2::pushVideoRawItem(Mp4File::PacketItem *item)
{
    std::unique_lock<std::mutex> lock(mVideoMutex);
    mVideoRaw.push(item);

    lock.unlock();
    mConVideoHandle.notify_one();
}

Mp4File::PacketItem * testQ1AndQ2::popVideoRawItem()
{
    std::unique_lock<std::mutex> lock(mVideoMutex);

    while (mVideoRaw.empty()) {
        mConVideoHandle.wait(lock);
    }

    Mp4File::PacketItem *item = mVideoRaw.front();
    mVideoRaw.pop();
    lock.unlock();

    return item;
}

void testQ1AndQ2::pushVideoHandleItem(Mp4File::PacketItem *item)
{
    std::unique_lock<std::mutex> lock(mVideoMutex);
    mVideoHandle.push(item);

    lock.unlock();
    mConVideoRaw.notify_one();
}

Mp4File::PacketItem * testQ1AndQ2::popVideoHandleItem()
{
    std::unique_lock<std::mutex> lock(mVideoMutex);

    while (mVideoHandle.empty()) {
        mConVideoRaw.wait(lock);
    }

    Mp4File::PacketItem *item = mVideoHandle.front();
    mVideoHandle.pop();
    lock.unlock();

    return item;
}

void testQ1AndQ2::pushAudioRawItem(Mp4File::PacketItem *item)
{
    std::unique_lock<std::mutex> lock(mAudioMutex);
    mAudioRaw.push(item);

    lock.unlock();
    mConAudioHandle.notify_one();
}

Mp4File::PacketItem * testQ1AndQ2::popAudioRawItem()
{
    std::unique_lock<std::mutex> lock(mAudioMutex);

    while (mAudioRaw.empty()) {
        mConAudioHandle.wait(lock);
    }

    Mp4File::PacketItem *item = mAudioRaw.front();
    mAudioRaw.pop();
    lock.unlock();

    return item;
}

void testQ1AndQ2::pushAudioHandleItem(Mp4File::PacketItem *item)
{
    std::unique_lock<std::mutex> lock(mAudioMutex);
    mAudioHandle.push(item);

    lock.unlock();
    mConAudioRaw.notify_one();
}

Mp4File::PacketItem * testQ1AndQ2::popAudioHandleItem()
{
    std::unique_lock<std::mutex> lock(mAudioMutex);

    while (mAudioHandle.empty()) {
        mConAudioRaw.wait(lock);
    }

    Mp4File::PacketItem *item = mAudioHandle.front();
    mAudioHandle.pop();
    lock.unlock();

    return item;
}

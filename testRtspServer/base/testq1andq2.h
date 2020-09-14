#ifndef TESTQ1ANDQ2_H
#define TESTQ1ANDQ2_H


#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>

#include "mp4file.h"

class testQ1AndQ2
{
public:
    testQ1AndQ2();

    int startRawThread();
    void stopRawThread();

    int startHandleThread();
    void stopHandleThread();

//
    void pushVideoRawItem(Mp4File::PacketItem *item);
    Mp4File::PacketItem * popVideoRawItem();

    void pushVideoHandleItem(Mp4File::PacketItem *item);
    Mp4File::PacketItem * popVideoHandleItem();

    void pushAudioRawItem(Mp4File::PacketItem *item);
    Mp4File::PacketItem * popAudioRawItem();

    void pushAudioHandleItem(Mp4File::PacketItem *item);
    Mp4File::PacketItem * popAudioHandleItem();

//
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

    //
    std::mutex mVideoMutex;
    std::condition_variable mConVideoHandle;
    std::condition_variable mConVideoRaw;

    std::mutex mAudioMutex;
    std::condition_variable mConAudioHandle;
    std::condition_variable mConAudioRaw;

    std::queue<Mp4File::PacketItem *> mVideoRaw;
    std::queue<Mp4File::PacketItem *> mVideoHandle;

    std::queue<Mp4File::PacketItem *> mAudioRaw;
    std::queue<Mp4File::PacketItem *> mAudioHandle;
};

#endif // TESTQ1ANDQ2_H

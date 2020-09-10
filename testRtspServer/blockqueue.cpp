#include "blockqueue.h"

BlockQueue::BlockQueue()
{
    m_max_items = MAX_ITEMS;
}

BlockQueue::~BlockQueue()
{

}

int BlockQueue::pushVideoFrame(Mp4File::PacketItem *item)
{
    std::unique_lock<std::mutex> lock(m_video_mutex);

//    printf("jinru push video frame\n");

    //判断队列有没有满
    while(isVideoQueueFull()) {
//        printf("push jinru dengdai\n");
        m_video_productor_con.wait(lock);
    }

    m_video_block_queue.push((item));

//    printf("finish push video frame\n");

    lock.unlock();

//    printf("finish push unlock \n");

    m_video_consumer_con.notify_all();

//    printf("finish push video con notify\n");

    return 0;
}

Mp4File::PacketItem * BlockQueue::popVideoFrame()
{
    std::unique_lock<std::mutex> lock(m_video_mutex);

//    printf("jinru pop video frame\n");

    //判断队列有没有空
    while(m_video_block_queue.empty()) {
//        printf("pop jinru dengdai\n");
        m_video_consumer_con.wait(lock);
    }

    Mp4File::PacketItem *item = m_video_block_queue.front();
    m_video_block_queue.pop();

//    printf("finish pop video frame\n");

    lock.unlock();

//    printf("finish pop video unlock\n");

    m_video_productor_con.notify_all();

//    printf("finish pop video notify\n");

    return item;
}

int BlockQueue::pushAudioFrame(Mp4File::PacketItem *item)
{
    std::unique_lock<std::mutex> lock(m_audio_mutex);

    //判断队列有没有满
    while(isAudioQueueFull()) {
        m_audio_productor_con.wait(lock);
    }

    m_audio_block_queue.push((item));

    lock.unlock();
    m_audio_consumer_con.notify_one();

    return 0;
}

Mp4File::PacketItem * BlockQueue::popAudioFrame()
{
    std::unique_lock<std::mutex> lock(m_audio_mutex);

    //判断队列有没有空
    while(m_audio_block_queue.empty()) {
        m_audio_consumer_con.wait(lock);
    }

    Mp4File::PacketItem *item = m_audio_block_queue.front();
    m_audio_block_queue.pop();

    lock.unlock();
    m_audio_productor_con.notify_one();

    return item;
}

bool BlockQueue::isVideoQueueFull()
{
    return (m_video_block_queue.size() == MAX_ITEMS ? true : false);
}

bool BlockQueue::isAudioQueueFull()
{
    return (m_audio_block_queue.size() >= MAX_ITEMS ? true : false);
}

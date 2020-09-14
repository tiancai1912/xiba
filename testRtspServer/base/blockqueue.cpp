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
    //判断队列有没有满
    while(isVideoQueueFull()) {
        m_video_productor_con.wait(lock);
    }

    m_video_block_queue.push((item));
    lock.unlock();

    m_video_consumer_con.notify_all();
    return 0;
}

Mp4File::PacketItem * BlockQueue::popVideoFrame()
{
    std::unique_lock<std::mutex> lock(m_video_mutex);
    //判断队列有没有空
    while(m_video_block_queue.empty()) {
        m_video_consumer_con.wait(lock);
    }

    Mp4File::PacketItem *item = m_video_block_queue.front();
    m_video_block_queue.pop();

    lock.unlock();
    m_video_productor_con.notify_all();

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

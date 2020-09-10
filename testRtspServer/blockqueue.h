#ifndef BLOCKQUEUE_H
#define BLOCKQUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>

#include "mp4file.h"

#define MAX_ITEMS 10


class BlockQueue
{
public:
    BlockQueue();
    ~BlockQueue();

    int pushVideoFrame(Mp4File::PacketItem *item);
    Mp4File::PacketItem * popVideoFrame();

    int pushAudioFrame(Mp4File::PacketItem *item);
    Mp4File::PacketItem * popAudioFrame();

private:
    bool isVideoQueueFull();
    bool isAudioQueueFull();


    int m_max_items;

    std::queue<Mp4File::PacketItem *> m_video_block_queue;
    std::queue<Mp4File::PacketItem *> m_audio_block_queue;

    std::mutex m_video_mutex;
    std::mutex m_audio_mutex;

    std::condition_variable m_video_consumer_con;
    std::condition_variable m_video_productor_con;
    std::condition_variable m_audio_consumer_con;
    std::condition_variable m_audio_productor_con;
};

#endif // BLOCKQUEUE_H

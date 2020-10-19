#ifndef MEDIASOURCE_H
#define MEDIASOURCE_H

#include <queue>
#include "base/mutex.h"
#include "base/threadpool.h"

//ffmpeg
#include <stdlib.h>
#include <stdint.h>
#include <string>
#include <string.h>
#include <fcntl.h>

extern "C" {

#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>

}

using namespace std;

#define MAX_FRAME_SIZE 1920 * 1080 * 3
#define ADTS_HEADER_SIZE 7
static const char *TAG = "MP4Parser";

class Frame
{
public:
    Frame() :
        mBuffer(new char[MAX_FRAME_SIZE]),
        mFrameSize(0)
    {
        mFrame = nullptr;
    }

    ~Frame()
    {
        mFrameSize = 0;
        delete mBuffer;
        mBuffer = nullptr;
        mFrame = nullptr;
    }

    char *mBuffer;
    char *mFrame;
    int mFrameSize;
};


class MediaSource
{
public:
    MediaSource(ThreadPool *pool);
    ~MediaSource();

    bool open(std::string url);
    void close();

    Frame * getFrame();
    bool putFrame(Frame *frame);

protected:
    typedef void (*callback) (void *arg);

    static void taskCallback(void * arg);

private:
    void readFrame();

private:
    std::string mUrl;

    std::queue<Frame *> mInputQueue;
    std::queue<Frame *> mOutputQueue;

    Mutex *mMutex;

    ThreadPool *mPool;
    ThreadPool::Task mTask;

    //ffmpeg
    AVPacket pkt;
    AVFormatContext *mFmtCtx;

    int m_video_stream_idx;
    int m_audio_stream_idx;

//    ADTSContext  m_adts_ctx;
    AVBitStreamFilterContext *m_video_bsf;
    AVBitStreamFilterContext *m_audio_bsf;

};

#endif // MEDIASOURCE_H

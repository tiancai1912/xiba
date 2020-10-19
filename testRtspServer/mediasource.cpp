#include "mediasource.h"

MediaSource::MediaSource(ThreadPool *pool)
{
    mMutex = Mutex::createNew();
    mPool = pool;
    mTask.setTaskCallback(taskCallback, this);
}

MediaSource::~MediaSource()
{

}

bool MediaSource::open(std::string url)
{
    if (avformat_open_input(&mFmtCtx, url.c_str(), NULL, NULL) < 0) {
        fprintf(stderr, "Could not open source file %s\n", url);
        return -1;
    }

    mUrl = url;

    if (avformat_find_stream_info(mFmtCtx, NULL) < 0) {
        fprintf(stderr, "Could not find stream information\n");
        return -1;
    }

    av_dump_format(mFmtCtx, 0, mUrl.c_str(), 0);

//    const char *formatName = mFmtCtx->iformat->long_name;
//    printf("the input file format name: %s\n", formatName);

//    int64_t duration = mFmtCtx->duration;
//    printf("the total time: %ld\n", duration / 1000000);

//    int streams = mFmtCtx->nb_streams;
//    printf("the stream is: %d\n", streams);

    int ret = av_find_best_stream(mFmtCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (ret < 0) {
        fprintf(stderr, "Could not find %s stream in input file '%s'\n",
                av_get_media_type_string(AVMEDIA_TYPE_AUDIO), mUrl);
        return ret;
    } else {
        m_video_stream_idx = ret;
        int frametrate = mFmtCtx->streams[m_video_stream_idx]->avg_frame_rate.num / mFmtCtx->streams[m_video_stream_idx]->avg_frame_rate.den;
        int width = mFmtCtx->streams[m_video_stream_idx]->codec->width;
        int height = mFmtCtx->streams[m_video_stream_idx]->codec->height;
        int frames = mFmtCtx->streams[m_video_stream_idx]->nb_frames; //nb_frames may be is 0

        printf("the video stream width: %d, the height: %d, the frame rate: %d, total frames: %d\n", width, height, frametrate, frames);
    }

    ret = av_find_best_stream(mFmtCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    if (ret < 0) {
        fprintf(stderr, "Could not find %s stream in input file '%s'\n",
                av_get_media_type_string(AVMEDIA_TYPE_AUDIO), mUrl);
        return ret;
    } else {
        m_audio_stream_idx = ret;
//        /**
//         * sometimes, we will not get valid value with samplerate, channels etc.
//         * we need to read some frames firstly, than we can get valud value
//         */
//        int sampletrate = mFmtCtx->streams[m_audio_stream_idx]->codec->sample_rate;
//        int channel = mFmtCtx->streams[m_audio_stream_idx]->codec->channels;
//        int profile = mFmtCtx->streams[m_audio_stream_idx]->codec->profile;
//        printf("the audio stream channel: %d, the sample rate: %d, the profile: %d\n", channel, sampletrate, profile);
    }

    m_video_bsf = av_bitstream_filter_init("h264_mp4toannexb");
}

void MediaSource::close()
{
    av_bitstream_filter_close(m_video_bsf);
    avformat_close_input(&mFmtCtx);
}

Frame * MediaSource::getFrame()
{
    MutexLockGuard lock(mMutex);

    if (mOutputQueue.empty()) {
        return nullptr;
    }

    Frame *frame = mOutputQueue.front();
    mOutputQueue.pop();
    return frame;
}

bool MediaSource::putFrame(Frame *frame)
{
    MutexLockGuard lock(mMutex);

    if (frame == nullptr) {
        return false;
    }

    mInputQueue.push(frame);

    mPool->addTask(mTask);
}

void MediaSource::taskCallback(void *arg)
{
    MediaSource *source = (MediaSource *)(arg);
    source->readFrame();
}

void MediaSource::readFrame()
{
    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;

    int ret = 0;
    ret = av_read_frame(mFmtCtx, &pkt);
    if (ret >= 0) {
        if (pkt.stream_index == m_video_stream_idx) {
            uint8_t *out_data = NULL;
            int out_size = 0;

            int err = av_bitstream_filter_filter(m_video_bsf, mFmtCtx->streams[m_video_stream_idx]->codec, NULL, &out_data, &out_size, pkt.data, pkt.size, pkt.flags & AV_PKT_FLAG_KEY);
            if (err <=0 ) {
                printf("%s, run read, av_bitstream_filter_filter failed\n", TAG);
            }

            Frame *frame = mInputQueue.front();
            frame->mFrameSize = out_size;
            memcpy(frame->mBuffer, (char *)out_data, out_size);
            mInputQueue.pop();

            mOutputQueue.push(frame);
            return;
        } else if (pkt.stream_index == m_audio_stream_idx) {
            Frame *frame = mInputQueue.front();
            frame->mFrameSize = pkt.size;
            memcpy(frame->mBuffer, (const char *)(pkt.data), pkt.size);
            mInputQueue.pop();

            mOutputQueue.push(frame);
            return;
        }

        av_packet_unref(&pkt);
    } else {
        printf("av read frame failed!\n");
        return;
    }

    return;
}

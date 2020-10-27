#ifndef RTMPPUSH_H
#define RTMPPUSH_H

#include <librtmp/rtmp.h>

extern "C" {

#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>

}

static const char *SERVER_URL = "rtmp://192.168.0.105:1935/live/test";

class RtmpPush
{
public:
    RtmpPush();
    ~RtmpPush();

    int openFile(char *url);
    int closeFile();

    int start();
    int stop();

private:
    bool connect();
    void disconnect();
    int sendPacket(char *buf, int len);

    void readFrame();

private:

    //ffmpeg
    AVPacket pkt;
    AVFormatContext *m_fmt_ctx;
    int m_video_stream_idx;
    AVBitStreamFilterContext *m_video_bsf;

    //rtmp
    RTMP* mRtmp;
};

#endif // RTMPPUSH_H

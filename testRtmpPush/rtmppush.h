#ifndef RTMPPUSH_H
#define RTMPPUSH_H

#include <librtmp/rtmp.h>

extern "C" {

#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>

}

#define SERVER_URL "rtmp://10.1.198.100:1935/live/test";

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

    AVPacket pkt;
    AVFormatContext *m_fmt_ctx;
    int m_video_stream_idx;
    AVBitStreamFilterContext *m_video_bsf;

};

#endif // RTMPPUSH_H

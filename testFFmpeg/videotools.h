#ifndef VIDEOTOOLS_H
#define VIDEOTOOLS_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>

extern "C" {

#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>

}

class VideoDecoder
{
public:
    VideoDecoder();
    ~VideoDecoder();

    int init();
    void unInit();

    int openInput(char *url);
    void closeInput();

    AVPacket * getOnePacketFromFile();
    AVFrame * decodePacket(AVPacket *pkt);

    int decode(AVPacket *pkt, AVFrame *frame, int *got_frame);

    int open_codec_context(AVCodecContext **dec_ctx, AVFormatContext *fmt_ctx, enum AVMediaType type);

    AVFormatContext *m_fmt_ctx;
    AVCodecContext *m_video_dec_ctx;
    AVCodecContext *m_audio_dec_ctx;

    int m_video_stream_idx;
    int m_audio_stream_idx;

    AVBitStreamFilterContext *m_video_bsf;

};

class VideoEncoder
{
public:
    VideoEncoder();
    ~VideoEncoder();

    int init();
    void unInit();

    int openInput(unsigned char *url);
    void closeInput();

    AVPacket * encodeFrame(unsigned char *buf, int size);
    AVPacket * encodeFrame(AVFrame *frame);
};

class VideoTools
{
public:
    VideoTools();
    ~VideoTools();
};

#endif // VIDEOTOOLS_H

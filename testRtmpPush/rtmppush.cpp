#include "rtmppush.h"

RtmpPush::RtmpPush()
{
    m_video_bsf = NULL;
    m_fmt_ctx = NULL;
    m_video_stream_idx = -1;
}

RtmpPush::~RtmpPush()
{

}

int RtmpPush::openFile(char *url)
{
    /* open input file, and allocate format context */
    if (avformat_open_input(&m_fmt_ctx, (char *)url, NULL, NULL) < 0) {
        fprintf(stderr, "Could not open source file %s\n", url);
        return false;
    }

    /* retrieve stream information */
    if (avformat_find_stream_info(m_fmt_ctx, NULL) < 0) {
        fprintf(stderr, "Could not find stream information\n");
        return false;
    }

    /* dump input information to stderr */
    av_dump_format(m_fmt_ctx, 0, url, 0);

    //find video stream index
    int ret = av_find_best_stream(m_fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (ret < 0) {
        fprintf(stderr, "Could not find %s stream in input file '%s'\n",
                av_get_media_type_string(AVMEDIA_TYPE_AUDIO), url);
        return false;
    }

    m_video_stream_idx = ret;
    m_video_bsf = av_bitstream_filter_init("h264_mp4toannexb");
}

int RtmpPush::closeFile()
{
    av_bitstream_filter_close(m_video_bsf);
    avformat_close_input(&m_fmt_ctx);
}

int RtmpPush::start()
{

}

int RtmpPush::stop()
{

}

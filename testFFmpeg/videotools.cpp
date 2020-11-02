#include "videotools.h"

VideoTools::VideoTools()
{
    //todo
}

VideoTools::~VideoTools()
{
    //todo
}

VideoDecoder::VideoDecoder() :
    m_fmt_ctx(NULL),
    m_video_dec_ctx(NULL),
    m_audio_dec_ctx(NULL),
    m_video_bsf(NULL),
    m_audio_stream_idx(-1),
    m_video_stream_idx(-1)
{

}

VideoDecoder::~VideoDecoder()
{

}

int VideoDecoder::init()
{

}

void VideoDecoder::unInit()
{

}

int VideoDecoder::open_codec_context(AVCodecContext **dec_ctx, AVFormatContext *fmt_ctx, enum AVMediaType type)
{
    int ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0);
    if (ret < 0) {
        printf("not find stream\n");
        return -1;
    }

    printf("fin best stream : %d\n", ret);

    int stream_idx = ret;
    AVStream *st = fmt_ctx->streams[stream_idx];

    AVCodec *codec = avcodec_find_decoder(st->codecpar->codec_id);
    if (!codec) {
        printf("failed to find decoder\n");
        return -1;
    }

    *dec_ctx = avcodec_alloc_context3(codec);
    if (!*dec_ctx) {
        printf("failed to alloc context\n");
        return -1;
    }

    if ((ret = avcodec_parameters_to_context(*dec_ctx, st->codecpar)) < 0) {
        printf("failed to copy codec parameters to decoder context\n");
        return -1;
    }

    AVDictionary *opts = NULL;
    int refcount = 0;
    av_dict_set(&opts, "refcounted_frames", refcount ? "1" : "0", 0);
    if (ret = avcodec_open2(*dec_ctx, codec, &opts) < 0) {
        printf("failed to open codec\n");
        return -1;
    }

    return stream_idx;
}


int VideoDecoder::openInput(char *url)
{
    if (url == NULL) {
        printf("invalid url\n");
        return -1;
    }

    if (avformat_open_input(&m_fmt_ctx, (char *)url, NULL, NULL) < 0) {
        printf("open input failed\n");
        return -1;
    }

    if (avformat_find_stream_info(m_fmt_ctx, NULL) < 0) {
        printf("failed to find stream info\n");
        return -1;
    }

    AVStream *video_stream = NULL;
    if ((m_video_stream_idx = open_codec_context(&m_video_dec_ctx, m_fmt_ctx, AVMEDIA_TYPE_VIDEO)) >= 0) {
        video_stream = m_fmt_ctx->streams[m_video_stream_idx];
        printf("the video stream index: %d\n", m_video_stream_idx);
    }

    AVStream *audio_stream = NULL;
    if ((m_audio_stream_idx = open_codec_context(&m_audio_dec_ctx, m_fmt_ctx, AVMEDIA_TYPE_AUDIO)) >= 0) {
        audio_stream = m_fmt_ctx->streams[m_audio_stream_idx];
        printf("the audio stream index: %d\n", m_audio_stream_idx);
    }

    if (!video_stream || !audio_stream) {
        printf("failed to get video / audio stream\n");
    }

//    av_dump_format(m_fmt_ctx, 0, (char *)url, 0);

    m_video_bsf = av_bitstream_filter_init("h264_mp4toannexb");

    return 0;
}

void VideoDecoder::closeInput()
{
    avcodec_free_context(&m_video_dec_ctx);
    avcodec_free_context(&m_audio_dec_ctx);
    avformat_close_input(&m_fmt_ctx);
    av_bitstream_filter_close(m_video_bsf);
}

AVPacket * VideoDecoder::getOnePacketFromFile()
{
    AVPacket *packet = av_packet_alloc();
    if (!packet) {
        printf("failed to alloc packet\n");
        return NULL;
    }

    while(1) {
        if (av_read_frame(m_fmt_ctx, packet) < 0) {
            return NULL;
        }

        if (packet->stream_index == m_video_stream_idx)
            break;
    }


//    uint8_t *out_data = NULL;
//    int out_size = 0;

    if (packet->stream_index == m_video_stream_idx) {
//        printf("the video stream index: %d\n", m_video_stream_idx);
        int err = av_bitstream_filter_filter(m_video_bsf, m_fmt_ctx->streams[m_video_stream_idx]->codec, NULL, &packet->data, &packet->size, packet->data, packet->size, packet->flags & AV_PKT_FLAG_KEY);
        if (err <= 0 ) {
            printf("run read, av_bitstream_filter_filter failed\n");
        }
    }

//    packet->data = out_data;
//    packet->size = out_size;

    if (packet->stream_index == m_audio_stream_idx) {
        return NULL;
    }

    return packet;
}

int VideoDecoder::decode(AVPacket *pkt, AVFrame *frame, int *got_frame)
{
    int decoded = pkt->size;
    int ret = 0;

//    printf("packet: %d\n", pkt->size);

    if (pkt->stream_index == m_video_stream_idx) {
        ret = avcodec_decode_video2(m_video_dec_ctx, frame, got_frame, pkt);
        if (ret < 0) {
            printf("failed to decode video packet\n");
        }
    } else if (pkt->stream_index == m_audio_stream_idx, got_frame, pkt) {
//        ret = avcodec_decode_audio4(m_audio_dec_ctx, frame, got_frame, pkt);
//        if (ret < 0) {
//            printf("failed to decode audio packet\n");
//        }
    }

    decoded = FFMIN(ret, pkt->size);

    return decoded;
}

AVFrame * VideoDecoder::decodePacket(AVPacket *pkt)
{
    int ret = 0;
    int got_frame = 0;
    AVFrame *frame = av_frame_alloc();

    do {
        ret = decode(pkt, frame, &got_frame);
        if (ret < 0)
            break;
        pkt->data += ret;
        pkt->size -= ret;
    } while (pkt->size > 0);

    printf("the got frame : %d\n", got_frame);

    return frame;
}

#include "videoparser.h"
#include <QDebug>

#define NALU_SLICE  1
#define NALU_IDR  5
#define NALU_SEI  6
#define NALU_SPS  7
#define NALU_PPS  8


VideoParser::VideoParser() :
    mFd(NULL),
    mFrameCount(0),
    m_video_bsf(NULL),
    m_fmt_ctx(NULL),
    m_video_stream_idx(-1)
{
}

VideoParser::~VideoParser()
{

}

VideoParser::PacketItem * VideoParser::readNaluItems()
{
    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;

    int ret = 0;
    ret = av_read_frame(m_fmt_ctx, &pkt);
    if (ret >= 0) {
        if (pkt.stream_index == m_video_stream_idx) {
            uint8_t *out_data = NULL;
            int out_size = 0;

            int err = av_bitstream_filter_filter(m_video_bsf, m_fmt_ctx->streams[m_video_stream_idx]->codec, NULL, &out_data, &out_size, pkt.data, pkt.size, pkt.flags & AV_PKT_FLAG_KEY);
//            int err = av_bitstream_filter_filter(m_video_bsf, m_fmt_ctx->streams[m_video_stream_idx]->codec, NULL, &out_data, &out_size, pkt.data, pkt.size, 0);
            if (err <= 0 ) {
                qDebug() << "run read, av_bitstream_filter_filter failed\n";
            }


            PacketItem *item = new PacketItem();

            item->pts = pkt.pts;
            item->duration = pkt.duration;
            item->length = out_size;
            memcpy(item->data, (char *)out_data, out_size);

//            qDebug() << item->data[0]  << " - " << item->data[1] << " - " << item->data[2] << " - " << item->data[3] << " - " << item->data[4] << " - " << endl;
//            printf("%02x - %02x - %02x - %02x - %02x \n", item->data[0], item->data[1], item->data[2], item->data[3], item->data[4]);

            mFrameCount++;

            return item;
        }

        av_packet_unref(&pkt);
    } else {
        printf("av read frame failed!\n");
        return NULL;
    }

    return NULL;
}

void VideoParser::parseNaluItem(PacketItem *item)
{
    if (item != NULL) {
        int type = item->data[4] & 0x1F;

        switch (type) {
        case NALU_SLICE:
//            qDebug() << "Slice Nalu" << endl;
            break;
        case NALU_IDR:
            qDebug() << "IDR Nalu" << endl;
            break;
        case NALU_SEI:
            qDebug() << "SEI Nalu" << endl;
            break;
        case NALU_SPS:
            qDebug() << "SPS Nalu" << endl;
            break;
        case NALU_PPS:
            qDebug() << "PPS Nalu" << endl;
            break;
        default:
            qDebug() << "Unknown Nalu" << endl;
            break;
        }
    }
}

bool VideoParser::openFile(char *url)
{
    /* open input file, and allocate format context */
    if (avformat_open_input(&m_fmt_ctx, (char *)url, NULL, NULL) < 0) {
        fprintf(stderr, "Could not open source file %s\n", url);
        return false;
    }

    m_src_filename = url;

    /* retrieve stream information */
    if (avformat_find_stream_info(m_fmt_ctx, NULL) < 0) {
        fprintf(stderr, "Could not find stream information\n");
        return false;
    }

    /* dump input information to stderr */
    av_dump_format(m_fmt_ctx, 0, (char *)m_src_filename, 0);

    //find video stream index
    int ret = av_find_best_stream(m_fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (ret < 0) {
        fprintf(stderr, "Could not find %s stream in input file '%s'\n",
                av_get_media_type_string(AVMEDIA_TYPE_AUDIO), m_src_filename);
        return false;
    }

    m_video_stream_idx = ret;

    m_video_bsf = av_bitstream_filter_init("h264_mp4toannexb");

    return true;
}

void VideoParser::closeFile()
{
    av_bitstream_filter_close(m_video_bsf);
    avformat_close_input(&m_fmt_ctx);
}

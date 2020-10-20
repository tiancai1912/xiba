#include "videoparser.h"
#include <QDebug>
#include <thread>

VideoParser::VideoParser() :
    mFrameCount(0),
    m_video_bsf(NULL),
    m_fmt_ctx(NULL),
    m_video_stream_idx(-1)
{
}

VideoParser::~VideoParser()
{
    for(int i = 0; i < mPackets.size(); i++) {
        delete mPackets.at(i);
    }

    mPackets.clear();
}

VideoParser::PacketItem * VideoParser::readNaluItems()
{
//    std::thread *t = new std::thread(std::bind(parseData, this));

    int ret = 0;
//    while(mFrameCount < MAX_NALU_READ_SIZE) {
        AVPacket pkt;
        av_init_packet(&pkt);
        pkt.data = NULL;
        pkt.size = 0;
        ret = av_read_frame(m_fmt_ctx, &pkt);
        if (ret >= 0) {
            if (pkt.stream_index == m_video_stream_idx) {
                uint8_t *out_data = NULL;
                int out_size = 0;

                int err = av_bitstream_filter_filter(m_video_bsf, m_fmt_ctx->streams[m_video_stream_idx]->codec, NULL, &out_data, &out_size, pkt.data, pkt.size, pkt.flags & AV_PKT_FLAG_KEY);
                if (err <= 0 ) {
                    qDebug() << "run read, av_bitstream_filter_filter failed\n";
                }


                PacketItem *item = new PacketItem();

                item->pts = pkt.pts;
                item->duration = pkt.duration;
                item->length = out_size;
                memcpy(item->data, (char *)out_data, MAX_DATA_SIZE);
//                parseNaluItem(item);

                mPackets.push_back(item);
                return item;
            }

            av_packet_unref(&pkt);
        } else {
            printf("av read frame failed!\n");
            return NULL;
        }

        mFrameCount++;
//    }

    return NULL;
}

int VideoParser::parseNaluItem(PacketItem *item)
{
//    qDebug() << item->length << endl;
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

        return type;
    }
}

bool VideoParser::openFile(char *url)
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

    return true;
}

void VideoParser::closeFile()
{
    av_bitstream_filter_close(m_video_bsf);
    avformat_close_input(&m_fmt_ctx);
}

void VideoParser::parseData(void *arg)
{
    VideoParser *pThis = (VideoParser *)arg;
    pThis->handleParseData();
}

void VideoParser::handleParseData()
{
    int ret = 0;
    while(mFrameCount < MAX_NALU_READ_SIZE) {
        AVPacket pkt;
        av_init_packet(&pkt);
        pkt.data = NULL;
        pkt.size = 0;
        ret = av_read_frame(m_fmt_ctx, &pkt);
        if (ret >= 0) {
            if (pkt.stream_index == m_video_stream_idx) {
                uint8_t *out_data = NULL;
                int out_size = 0;

                int err = av_bitstream_filter_filter(m_video_bsf, m_fmt_ctx->streams[m_video_stream_idx]->codec, NULL, &out_data, &out_size, pkt.data, pkt.size, pkt.flags & AV_PKT_FLAG_KEY);
                if (err <= 0 ) {
                    qDebug() << "run read, av_bitstream_filter_filter failed\n";
                }


                PacketItem *item = new PacketItem();

                item->pts = pkt.pts;
                item->duration = pkt.duration;
                item->length = out_size;
                memcpy(item->data, (char *)out_data, MAX_DATA_SIZE);
                parseNaluItem(item);

                mPackets.push_back(item);
//                return item;
            }

            av_packet_unref(&pkt);
        } else {
            printf("av read frame failed!\n");
            return;
        }

        mFrameCount++;
    }

    return;
}

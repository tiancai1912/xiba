#include "rtmppush.h"
#include <QtDebug>
#include <unistd.h>

#define RTMP_HEAD_SIZE   (sizeof(RTMPPacket)+RTMP_MAX_HEADER_SIZE)

#define NALU_SLICE  1
#define NALU_SLICE_DPA  2
#define NALU_SLICE_DPB  3
#define NALU_SLICE_DPC  4
#define NALU_IDR  5
#define NALU_SEI  6
#define NALU_SPS  7
#define NALU_PPS  8

uint8_t sps[15] = {0x67, 0x42, 0x00, 0x1f, 0x95, 0xa8, 0x14, 0x01, 0x6e, 0x9b, 0x80, 0x80, 0x80, 0x81, 0x01};
uint8_t pps[4] = {0x68, 0xce, 0x3c, 0x80};

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

    for (int i = 0; i < m_fmt_ctx->streams[m_video_stream_idx]->codec->extradata_size; i++) {
        printf("the hex data: %02x\n", m_fmt_ctx->streams[m_video_stream_idx]->codec->extradata[i]);
    }

    readFrame();
}

int RtmpPush::closeFile()
{
    av_bitstream_filter_close(m_video_bsf);
    avformat_close_input(&m_fmt_ctx);
}

int RtmpPush::start()
{
    connect();
    return 0;
}

int RtmpPush::stop()
{
    disconnect();
    return 0;
}

bool RtmpPush::connect()
{
    mRtmp = RTMP_Alloc();
    RTMP_Init(mRtmp);

    if (RTMP_SetupURL(mRtmp, (char *)SERVER_URL) == FALSE) {
        RTMP_Free(mRtmp);
        printf("set url failed!\n");
        return false;
    }

    printf("set url[%s] success!\n", SERVER_URL);

    RTMP_EnableWrite(mRtmp);
    if (RTMP_Connect(mRtmp, NULL) == FALSE) {
        RTMP_Free(mRtmp);
        printf("connect server failed!\n");
        return false;
    }

    printf("connect success!\n");

    if (RTMP_ConnectStream(mRtmp, 0) == FALSE) {
        RTMP_Close(mRtmp);
        RTMP_Free(mRtmp);
        printf("connect stream failed!\n");
        return false;
    }

    printf("connect stream success!\n");

    return true;
}

void RtmpPush::disconnect()
{
    if (mRtmp) {
        RTMP_Close(mRtmp);
        RTMP_Free(mRtmp);
        mRtmp = NULL;
        printf("disconnect success!\n");
    }
}

int RtmpPush::sendPacket(char *buf, int size, int nTimeStamp)
{
    RTMPPacket *packet;
    packet = (RTMPPacket *)malloc(RTMP_HEAD_SIZE + size);
    memset(packet, 0, RTMP_HEAD_SIZE);

    packet->m_body = (char *)packet + RTMP_HEAD_SIZE;
    packet->m_nBodySize = size;
    memcpy(packet->m_body, buf,size);
    packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
    packet->m_nInfoField2 = mRtmp->m_stream_id;
    packet->m_nChannel = 0x04;

    packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
    packet->m_nTimeStamp = nTimeStamp;
    int nRet = 0;
    if (RTMP_IsConnected(mRtmp))
    {
        nRet = RTMP_SendPacket(mRtmp, packet, TRUE); /*TRUE为放进发送队列,FALSE是不放进发送队列,直接发送*/
//        qDebug() << "send packet : " << nRet;
    }

//    qDebug() << "send packet success!\n";
    /*释放内存*/
    free(packet);
    return nRet;
}

void RtmpPush::readFrame()
{
    int ret = 0;
    int nTimeStamp = 0;
    bool ishasSps = false;
    while(1) {
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

//                qDebug() << "read one frame: " << out_size << endl;
//                sendPacket((char *)out_data, out_size);

//                int type = pkt.data[0] & 0x1F;
                int type = out_data[4] & 0x1F;
                qDebug() << "the type is: " << type << endl;
//                printf("%02x %02x %02x %02x %02x\n", pkt.data[0], pkt.data[1], pkt.data[2], pkt.data[3], pkt.data[4]);
//                printf("%02x %02x %02x %02x %02x\n", out_data[0], out_data[1], out_data[2], out_data[3], out_data[4]);

                qDebug() << "this pkt key flag: " << pkt.flags << endl;
                if (type == NALU_SLICE ||
                        type == NALU_SLICE_DPA ||
                        type == NALU_SLICE_DPB ||
                        type == NALU_SLICE_DPC ||
                        type == NALU_IDR || pkt.flags & AV_PKT_FLAG_KEY) {
                    if (type == NALU_IDR || pkt.flags & AV_PKT_FLAG_KEY) {
                        sendSPSPPSPacket((char *)sps, 15, (char *)pps, 4);
                        qDebug() << "key frame size: " << out_size << endl;

                        for (int i = 0; i < 60; i++) {
                            printf("the hex data: %02x\n", out_data[i]);
                        }


                        sendH264Packet((char *)(out_data + 4), (out_size - 4), true, nTimeStamp);
                    } else {
                        sendH264Packet((char *)(out_data + 4), (out_size - 4), false, nTimeStamp);
                    }

                    nTimeStamp ++;
                } else if (type == NALU_SPS && ishasSps == false) {
                    qDebug() << out_data[0] << out_data[1] << out_data[2] << out_data[3] << out_data[4] << endl;
                    qDebug() << "send sps" << endl;
//                    sendSPSPPSPacket((char *)(out_data + 4), (out_size - 4));
//                    sendSPSPPSPacket((char *)sps, 15);
                    ishasSps = true;
                }


//                sendPacket((char *)pkt.data, pkt.size);
            }

            av_packet_unref(&pkt);
        } else {
            printf("av read frame failed!\n");
            return;
        }

        usleep(80);
    }

    return;
}

int RtmpPush::sendH264Packet(char *buf, int size, int isKeyFrame, int nTimeStamp)
{
    if (buf == NULL || size < 11) {
        return false;
    }

    unsigned char *body = (unsigned char *)malloc(size + 9);
    memset(body, 0, size + 9);

    int i = 0;
    if (isKeyFrame) {
        body[i++] = 0x17;
        body[i++] = 0x01;
        body[i++] = 0x00;
        body[i++] = 0x00;
        body[i++] = 0x00;

        body[i++] = size >> 24 & 0xff;
        body[i++] = size >> 16 & 0xff;
        body[i++] = size >> 8 & 0xff;
        body[i++] = size & 0xff;

        memcpy(&body[i], buf, size);
    } else {
        body[i++] = 0x27;
        body[i++] = 0x01;
        body[i++] = 0x00;
        body[i++] = 0x00;
        body[i++] = 0x00;

        body[i++] = size >> 24 & 0xff;
        body[i++] = size >> 16 & 0xff;
        body[i++] = size >> 8 & 0xff;
        body[i++] = size & 0xff;

        memcpy(&body[i], buf, size);
    }

    int nRet = sendPacket((char *)body, i + size, nTimeStamp);
    free(body);
    return nRet;

}

int RtmpPush::sendSPSPPSPacket(char *buf, int size, char *pps, int ppsSize)
{
    if (buf == NULL || size < 11) {
        qDebug() << "sps buf is null" << endl;
        return 0;
    }
        RTMPPacket * packet=NULL;//rtmp包结构
        unsigned char * body=NULL;
        int i;
        packet = (RTMPPacket *)malloc(RTMP_HEAD_SIZE+50000);
        RTMPPacket_Reset(packet);//重置packet状态
//        memset(packet,0,RTMP_HEAD_SIZE+1024);
        packet->m_body = (char *)(packet + RTMP_HEAD_SIZE);
        body = (unsigned char *)packet->m_body;
        i = 0;
        body[i++] = 0x17;
        body[i++] = 0x00;

        body[i++] = 0x00;
        body[i++] = 0x00;
        body[i++] = 0x00;

        /*AVCDecoderConfigurationRecord*/
        body[i++] = 0x01;
        body[i++] = buf[1];
        body[i++] = buf[2];
        body[i++] = buf[3];
        body[i++] = 0xff;

        /*sps*/
        body[i++]   = 0xe1;
        body[i++] = (size >> 8) & 0xff;
        body[i++] = size & 0xff;
        qDebug() << "size: " << size << endl;
        if (body[i] == NULL || buf == NULL || size <= 0 || size >= 50000) {
            qDebug() << "body is null" << endl;
            free(packet);
            return 0;
        } else {
            memcpy(&body[i],buf, size);
        }

        i +=  size;

        /*pps*/
        body[i++]   = 0x01;
        body[i++] = (ppsSize >> 8) & 0xff;
        body[i++] = (ppsSize) & 0xff;
        memcpy(&body[i],pps,ppsSize);
        i +=  ppsSize;

        packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
        packet->m_nBodySize = i;
        packet->m_nChannel = 0x04;
        packet->m_nTimeStamp = 0;
        packet->m_hasAbsTimestamp = 0;
        packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
        packet->m_nInfoField2 = mRtmp->m_stream_id;

        /*调用发送接口*/
        int nRet = RTMP_SendPacket(mRtmp,packet,TRUE);
        free(packet);    //释放内存
        return nRet;
}

#include "rtmppush.h"

#define RTMP_HEAD_SIZE   (sizeof(RTMPPacket)+RTMP_MAX_HEADER_SIZE)

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

int RtmpPush::sendPacket(char *buf, int size)
{
    RTMPPacket *packet;
    packet = (RTMPPacket *)malloc(RTMP_HEAD_SIZE + size);

//    RTMPPacket* packet;
//        /*分配包内存和初始化,len为包体长度*/
//        packet = (RTMPPacket *)malloc(RTMP_HEAD_SIZE+size);
//        memset(packet,0,RTMP_HEAD_SIZE);
//        /*包体内存*/
//        packet->m_body = (char *)packet + RTMP_HEAD_SIZE;
//        packet->m_nBodySize = size;
//        memcpy(packet->m_body,data,size);
//        packet->m_hasAbsTimestamp = 0;
//        packet->m_packetType = nPacketType; /*此处为类型有两种一种是音频,一种是视频*/
//        packet->m_nInfoField2 = m_pRtmp->m_stream_id;
//        packet->m_nChannel = 0x04;

//        packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
//        if (RTMP_PACKET_TYPE_AUDIO ==nPacketType && size !=4)
//        {
//            packet->m_headerType = RTMP_PAsCKET_SIZE_MEDIUM;
//        }
//        packet->m_nTimeStamp = nTimestamp;
//        /*发送*/
//        int nRet =0;
//        if (RTMP_IsConnected(m_pRtmp))
//        {
//            nRet = RTMP_SendPacket(m_pRtmp,packet,TRUE); /*TRUE为放进发送队列,FALSE是不放进发送队列,直接发送*/
//        }
//        /*释放内存*/
//        free(packet);
//        return nRet;
}

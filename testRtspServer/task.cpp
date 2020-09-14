#include "task.h"

Task::Task()
{
    mServerRtpSocketfd = -1;
    mClientIp = (char *)malloc(40);
    mVideoClientPort = -1;
    mAudioClientPort = -1;
}

Task::~Task()
{
}


int Task::start(int serverSocketfd, char *ip, int videoClientPort, int audioClientPort)
{
    mServerRtpSocketfd = serverSocketfd;
    mVideoClientPort = videoClientPort;
    mAudioClientPort = audioClientPort;
    strcpy(mClientIp, ip);

    mVideoRtpPacket = (struct RtpPacket*)malloc(500000);
    mRtp.rtpHeaderInit(mVideoRtpPacket, 0, 0, 0, RTP_VESION, RTP_PAYLOAD_TYPE_H264, 0,
                        0, 0, 0x88923423);

    mAudioRtpPacket = (struct RtpPacket *)malloc(5000);
    mRtp.rtpHeaderInit(mAudioRtpPacket, 0, 0, 0, RTP_VESION, RTP_PAYLOAD_TYPE_AAC, 1, 0, 0, 0x32411);

    mFile = new Mp4File();
    mFile->openFile("22.mp4");

    mGetRawData = new std::thread(&Task::getRawData, this);

    mH264Timer = new MyTimer("h264");
    mH264Timer->Start(40, std::bind(&Task::handleH264Data, this), true, true);

    mAACTimer = new MyTimer("aac");
    mAACTimer->Start(23, std::bind(&Task::handleAACData, this), true, true);

    return 0;
}

void Task::stop()
{
    mH264Timer->Cancel();
    mAACTimer->Cancel();
    mGetRawData->join();
    mFile->closeFile();
}

void Task::getRawData()
{
    while(1) {
        Mp4File::PacketItem *item = mFile->getOneFrame();
        if (item != NULL) {
            if (item->type == Mp4File::PacketType::PACKET_VIDEO) {
                if(!mRtp.startCode3(item->data) && !mRtp.startCode4(item->data)) {
                    free(item);
                    item = NULL;
                    continue;
                }

                Mp4File::PacketItem *tmp = mBlockQueue.popVideoHandleItem();
                tmp->copy(item);
                mBlockQueue.pushVideoRawItem(tmp);
            } else if (item->type == Mp4File::PacketType::PACKET_AUDIO) {
                Mp4File::PacketItem *tmp = mBlockQueue.popAudioHandleItem();
                tmp->copy(item);
                mBlockQueue.pushAudioRawItem(tmp);
            } else {
                printf("get frame invalid type!\n");
            }

            free(item->data);
            item->data = NULL;
        }
    }
}

void Task::handleH264Data()
{
    Mp4File::PacketItem *item = mBlockQueue.popVideoRawItem();
    //handle
    int startCode;

//    printf("start play\n");
//    printf("client ip:%s\n", mClientIp);
    printf("client port:%d\n", mVideoClientPort);
//    printf("audio client port:%d\n", mAudioClientPort);

    if(!mRtp.startCode3(item->data) && !mRtp.startCode4(item->data)) {
        printf("start code invalid\n");
        mBlockQueue.pushVideoHandleItem(item);
        return;
    }

    if(mRtp.startCode3(item->data))
        startCode = 3;
    else
        startCode = 4;

    item->length -= startCode;
    mRtp.rtpSendH264Frame(mServerRtpSocketfd, (char *)mClientIp, mVideoClientPort,
                          mVideoRtpPacket, (unsigned char *)(item->data + startCode), item->length);

    mVideoRtpPacket->rtpHeader.timestamp += 90000/25;

//    free(videoRtpPacket);

    //
    mBlockQueue.pushVideoHandleItem(item);
}

void Task::handleAACData()
{
    Mp4File::PacketItem *item = mBlockQueue.popAudioRawItem();
    //handle
//    struct RtpPacket* audioRtpPacket = (struct RtpPacket *)malloc(5000);
//    mRtp.rtpHeaderInit(audioRtpPacket, 0, 0, 0, RTP_VESION, RTP_PAYLOAD_TYPE_AAC, 1, 0, 0, 0x32411);


//    printf("start play\n");
//    printf("client ip:%s\n", mClientIp);
//    printf("client port:%d\n", mVideoClientPort);
    printf("audio client port:%d\n", mAudioClientPort);

    mRtp.rtpSendAACFrame(mServerRtpSocketfd, mClientIp, mAudioClientPort,
                         mAudioRtpPacket, (unsigned char *)(item->data), item->length);
    //
    mBlockQueue.pushAudioHandleItem(item);
}

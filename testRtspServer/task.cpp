#include "task.h"

Task::Task()
{
    mServerRtpSocketfd = -1;
    mClientIp = (char *)malloc(40);
    mVideoClientPort = -1;
    mAudioClientPort = -1;
    m_diff_time = 0;
    mFps = 0;
    mFirstTime = 0;
    mCurTime = 0;
    mFrames = 0;
    mQuit = false;
    mVideoMutex = Mutex::createNew();
    mAudioMutex = Mutex::createNew();
    mCanSendVideo = Condition::createNew();
    mCanSendAudio = Condition::createNew();
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
    mFile->openFile("11.mp4");

    mGetRawData = new std::thread(&Task::getRawData, this);

    createVideoTimer(40);
    createAudioTimer(23);
//    createCheckFpsTimer(10);

    mHandleVideoData = new std::thread(&Task::handleH264Data, this);
    mHandleAudioData = new std::thread(&Task::handleAACData, this);

//    mH264Timer = new MyTimer("h264");
//    mH264Timer->Start(40, std::bind(&Task::handleH264Data, this), true, true);

//    mAACTimer = new MyTimer("aac");
//    mAACTimer->Start(23, std::bind(&Task::handleAACData, this), true, true);

    return 0;
}

void Task::stop()
{
//    mH264Timer->Cancel();
//    mAACTimer->Cancel();
    mQuit = true;
    cancelVideoTimer();
    cancelAudioTimer();
    mHandleVideoData->join();
    mHandleAudioData->join();
    mGetRawData->join();
    mFile->closeFile();
}

void Task::getRawData()
{
//    MyTimer *timer = new MyTimer("tmp");
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

//void Task::getRawData()
//{
////    MyTimer *timer = new MyTimer("tmp");
//    while(1) {
//        Mp4File::PacketItem *item = mFile->getOneFrame();
//        if (item != NULL) {
//            if (item->type == Mp4File::PacketType::PACKET_VIDEO) {
//                if(!mRtp.startCode3(item->data) && !mRtp.startCode4(item->data)) {
//                    free(item);
//                    item = NULL;
//                    continue;
//                }

//                Mp4File::PacketItem *tmp = mBlockQueue.popVideoHandleItem();
//                tmp->copy(item);
//                mBlockQueue.pushVideoRawItem(tmp);
//            } else if (item->type == Mp4File::PacketType::PACKET_AUDIO) {
////                Mp4File::PacketItem *tmp = mBlockQueue.popAudioHandleItem();
////                tmp->copy(item);
////                mBlockQueue.pushAudioRawItem(tmp);
//            } else {
//                printf("get frame invalid type!\n");
//            }

//            free(item->data);
//            item->data = NULL;
//        }
//    }
//}

void Task::handleH264Data()
{
    while(!mQuit) {
        waitVideoEvent();

        Mp4File::PacketItem *item = mBlockQueue.popVideoRawItem();
        //handle
        int startCode;
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

//        if (mFps == 0) {
//            mVideoRtpPacket->rtpHeader.timestamp += 90000 / 25;
//        } else {
//            mVideoRtpPacket->rtpHeader.timestamp += 90000 / mFps;
//        }

        mBlockQueue.pushVideoHandleItem(item);

        if (mFirstTime == 0) {
            mFirstTime = getTime();
            mCurTime = getTime();
        } else {
            mCurTime = getTime();
        }

        if (mCurTime != mFirstTime) {
            int fps = (mFrames * 1000) / (mCurTime - mFirstTime);
            mFps = fps;
            printf("the cur - first value: %lld\n", (mCurTime - mFirstTime));
            printf("cur fps is: %d\n", fps);
//            if (fps <= 23 && fps > 0) {
//                cancelVideoTimer();

//                int timeout = 1000 / ((25 - fps) + 25);
//                createVideoTimer(timeout);

//                mFrames = 0;
//                mFirstTime = 0;
//                mCurTime = 0;
//                continue;
//            } else if (fps >= 27) {
//                cancelVideoTimer();

//                int timeout = 1000 / (25 - (fps - 25));
//                createVideoTimer(timeout);

//                mFrames = 0;
//                mFirstTime = 0;
//                mCurTime = 0;
//                continue;
//            }
        }

        mFrames++;
    }
}

void Task::handleAACData()
{
    while(!mQuit) {
        waitAudioEvent();

        Mp4File::PacketItem *item = mBlockQueue.popAudioRawItem();
        mRtp.rtpSendAACFrame(mServerRtpSocketfd, mClientIp, mAudioClientPort,
                             mAudioRtpPacket, (unsigned char *)(item->data), item->length);

        mBlockQueue.pushAudioHandleItem(item);
    }
}

void Task::handleVideoTimer()
{
    setVideoEvent();
}

void Task::checkFpsTimer()
{
    if (mFps <= 23 && mFps > 0) {
        cancelVideoTimer();

        int timeout = 1000 / ((25 - mFps) + 25);
        createVideoTimer(timeout);

        mFrames = 0;
        mFirstTime = 0;
        mCurTime = 0;
    } else if (mFps >= 27) {
        cancelVideoTimer();

        int timeout = 1000 / (25 - (mFps - 25));
        createVideoTimer(timeout);

        mFrames = 0;
        mFirstTime = 0;
        mCurTime = 0;
    }
}

void Task::handleAudioTimer()
{
    setAudioEvent();
}

ino64_t Task::getTime()
{
    struct timeval tv;
    gettimeofday(&tv , NULL);
    int64_t msec =  tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
    printf("the msec is : %lld\n", msec);
    return msec;
//    timestamp_increse = (uint)(90000.0 / (1000.0 / ((tv.tv_sec - tv_pre.tv_sec) * 1000.0 + (tv.tv_usec - tv_pre.tv_usec) / 1000.0)));

}

void Task::setVideoEvent()
{
    mCanSendVideo->signal();
}

void Task::setAudioEvent()
{
    mCanSendAudio->signal();
}

bool Task::waitVideoEvent()
{
    mCanSendVideo->wait(mVideoMutex);
}

bool Task::waitAudioEvent()
{
    mCanSendAudio->wait(mAudioMutex);
}

void Task::createVideoTimer(int timeout)
{
    mH264Timer = new MyTimer("h264");
    mH264Timer->Start(timeout, std::bind(&Task::handleVideoTimer, this), true, true);
}

void Task::createAudioTimer(int timeout)
{
    mAACTimer = new MyTimer("aac");
    mAACTimer->Start(timeout, std::bind(&Task::handleAudioTimer, this), true, true);
}

void Task::createCheckFpsTimer(int timeout)
{
    mCheckFps = new MyTimer("h264");
    mCheckFps->Start(timeout, std::bind(&Task::checkFpsTimer, this), true, true);
}

void Task::cancelVideoTimer()
{
    mH264Timer->Cancel();
}

void Task::cancelAudioTimer()
{
    mAACTimer->Cancel();
}

void Task::cancelCheckFpsTimer()
{
    mCheckFps->Cancel();
}


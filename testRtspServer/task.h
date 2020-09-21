#ifndef TASK_H
#define TASK_H

#include "mp4file.h"
#include "base/testq1andq2.h"
#include "base/mytimer.h"
#include <thread>
#include <unistd.h>
#include <sys/time.h>
#include "net/rtp.h"
#include "base/condition.h"
#include "base/mutex.h"

#define RTP_VESION              2

#define RTP_PAYLOAD_TYPE_H264   96
#define RTP_PAYLOAD_TYPE_AAC    97

#define RTP_HEADER_SIZE         12
#define RTP_MAX_PKT_SIZE        1400

class Task
{
public:
    Task();
    ~Task();

    int start(int serverSocketfd, char *ip, int videoClientPort, int audioClientPort);
    void stop();

protected:
    void getRawData();
    void handleH264Data();
    void handleAACData();

    void handleVideoTimer();
    void handleAudioTimer();

    void checkFpsTimer();

    ino64_t getTime();

private:
    void setVideoEvent();
    void setAudioEvent();
    bool waitVideoEvent();
    bool waitAudioEvent();
    void createVideoTimer(int timeout);
    void createAudioTimer(int timeout);
    void createCheckFpsTimer(int timeout);
    void cancelVideoTimer();
    void cancelAudioTimer();
    void cancelCheckFpsTimer();

private:
    Mp4File *mFile;
    std::thread *mGetRawData;
    MyTimer *mH264Timer;
    MyTimer *mAACTimer;
    testQ1AndQ2 mBlockQueue;

    char *mClientIp;
    int mServerRtpSocketfd;
    int mVideoClientPort;
    int mAudioClientPort;

    rtp mRtp;
    struct RtpPacket* mVideoRtpPacket;
    struct RtpPacket* mAudioRtpPacket;

    int64_t m_diff_time;


    ino64_t mFirstTime;
    ino64_t mCurTime;
    int64_t mFrames;

    int mFps;

    MyTimer *mCheckFps;

    bool mQuit;
    Mutex *mVideoMutex;
    Mutex *mAudioMutex;
    Condition *mCanSendVideo;
    Condition *mCanSendAudio;

    std::thread *mHandleVideoData;
    std::thread *mHandleAudioData;
};

#endif // TASK_H

#ifndef TASK_H
#define TASK_H

#include "mp4file.h"
#include "base/testq1andq2.h"
#include "base/mytimer.h"
#include <thread>
#include <unistd.h>
#include "net/rtp.h"

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
};

#endif // TASK_H

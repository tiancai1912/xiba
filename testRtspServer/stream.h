#ifndef STREAM_H
#define STREAM_H

#include "mediasource.h"
#include "rtphandler.h"

class Stream
{
public:
    Stream();
    ~Stream();

    int addMediaSource(MediaSource *source);
    int addRtpHandler(RtpHandler *handler);

    void removeMediaSource();
    void removeRtpHandler();

    int getId() { return mId; }

private:
    int mId;
    MediaSource *mSource;
    RtpHandler *mHandler;

};

#endif // STREAM_H

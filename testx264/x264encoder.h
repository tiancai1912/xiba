#ifndef X264ENCODER_H
#define X264ENCODER_H

#include <stdint.h>
#include <stdio.h>

extern "C" {
#include <x264.h>
}

#include <thread>


class x264Encoder
{
public:
    x264Encoder();

    struct EncodingFormat {
        char    *path;
        int     width;
        int     height;
        int     format;
        int     bitDepth;

    public:
        EncodingFormat() {
            path = nullptr;
            width = -1;
            height = -1;
            format = -1;
            bitDepth = -1;
        }
    };

    struct Buffer {
        char    *data;
        int     len;

    public:
        Buffer() {
            data = nullptr;
            len = 0;
        }
    };

#define Out Buffer

    class Callback
    {
    public:
        virtual void onData(char *data, int len) = 0;
    };

    int init(EncodingFormat format);
    void setCallback(Callback *callback);
    int start();
    void stop();
    void unInit();

    Out * encodeOneFrame(char *in, int len);


private:
    void run();



public:
    EncodingFormat  mFormat;
    x264_t          *mX264;
    x264_param_t    mX264Param;
    x264_picture_t  mPicIn;
    x264_picture_t  mPicOut;
    int             mIndex;

    x264_nal_t      *mNalu;
    int             mNaluIndex;

    Callback        *mCallback;
    std::thread     *mEncodingThread;
    FILE            *mFile;
    bool            mExit;
};

#endif // X264ENCODER_H

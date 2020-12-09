#include "x264encoder.h"
#include <string.h>
#include <QDebug>

x264Encoder::x264Encoder()
{
    mFile = nullptr;
    mExit = false;
    mIndex = 0;
}

/**
 * set zerolatency is very importent
 * else x264_encoder_encode return value is 0
 * */
int x264Encoder::init(EncodingFormat format) {
    if( x264_param_default_preset(&mX264Param, "medium", "zerolatency") < 0 ) {
        qDebug() << "default preset failed!\n";
        return -1;
    }

    mFormat.path = format.path;
    mFormat.width = format.width;
    mFormat.height = format.height;
    mFormat.format = format.format;
    mFormat.bitDepth = format.bitDepth;

    mX264Param.i_bitdepth = mFormat.bitDepth;
    mX264Param.i_csp = mFormat.format;
    mX264Param.i_width  = mFormat.width;
    mX264Param.i_height = mFormat.height;
    mX264Param.b_vfr_input = 0;
    mX264Param.b_repeat_headers = 1;
    mX264Param.b_annexb = 1;

    /* Apply profile restrictions. */
    if( x264_param_apply_profile(&mX264Param, 0) < 0 )
        return -1;

    if( x264_picture_alloc(&mPicIn, mX264Param.i_csp, mX264Param.i_width, mX264Param.i_height ) < 0 )
        return -1;

    mX264 = x264_encoder_open(&mX264Param);
    if( !mX264 )
        x264_picture_clean(&mPicIn);

    return 0;
}

void x264Encoder::setCallback(x264Encoder::Callback *callback) {
    mCallback = callback;
}

int x264Encoder::start() {
    char *path = mFormat.path;
    if (path != nullptr) {
        mFile = fopen(path, "rb");
        if (mFile == nullptr) {
            qDebug() << "open file failed! " << path << endl;
            return -1;
        }
    }

    mEncodingThread = new std::thread(std::bind(&x264Encoder::run, this));
    mEncodingThread->join();

//    run();

    qDebug() << "start encode\n";

    return 0;
}

void x264Encoder::stop() {
    //wait thread stop
    mExit  = true;

    int count = 0;
    //delay encode
    while( x264_encoder_delayed_frames(mX264) ) {
        int frameSize = x264_encoder_encode( mX264, &mNalu, &mNaluIndex, NULL, &mPicOut );
        if( frameSize < 0 ) {
            x264_encoder_close( mX264 );
        }

        count ++;
//        qDebug() << "zhangyu delay encode frame size: " << frameSize << endl;
    }

    qDebug() << "zhangyu the count is: " << count << endl;


    x264_encoder_close(mX264);
    x264_picture_clean(&mPicIn);

    fclose(mFile);
    mFile = nullptr;
}

void x264Encoder::unInit() {
    int count = 0;
    //delay encode
    while( x264_encoder_delayed_frames(mX264) ) {
        int frameSize = x264_encoder_encode( mX264, &mNalu, &mNaluIndex, NULL, &mPicOut );
        if( frameSize < 0 ) {
            x264_encoder_close( mX264 );
        }

        count ++;
    }

    x264_encoder_close(mX264);
    x264_picture_clean(&mPicIn);
}

x264Encoder::Out * x264Encoder::encodeOneFrame(unsigned char *in, int len) {
    mPicIn.img.plane[0] = (uint8_t *)in;
    mPicIn.i_pts = mIndex++;
    int frameSize = x264_encoder_encode(mX264, &mNalu, &mNaluIndex, &mPicIn, &mPicOut);
    if (frameSize < 0) {
        x264_encoder_close(mX264);
        return nullptr;
    }

    Buffer * buffer = new Buffer();
    buffer->len = frameSize;
    buffer->data = (mNalu[0].p_payload);
    return buffer;
//    return nullptr;
}

void x264Encoder::run()
{
//    qDebug() << "jinru run\n";
    printf("zhangyu jinru run\n");
    int size = mFormat.width * mFormat.height;
    char *bufY = (char *)malloc(size);
    char *bufU = (char *)malloc(size / 4);
    char *bufV = (char *)malloc(size / 4);

    printf("zhangyu jinru run11\n");

    Out *buffer = new Out();

    while (mExit == false) {
        printf("zhangyu jinru run22\n");
        memset(bufY, 0, size);
        memset(bufU, 0, size);
        memset(bufV, 0, size);

        //read buf
//        int ret = fread(bufY, 1, size, mFile);
//        if (ret != size) {
//            qDebug() << "read bufY error size: " << ret << endl;
//            break;
//        }

//        ret = fread(bufU, 1, size / 4, mFile);
//        if (ret != size / 4) {
//            qDebug() << "read bufU error size: " << ret << endl;
//            break;
//        }

//        ret = fread(bufV, 1, size / 4, mFile);
//        if (ret != size / 4) {
//            qDebug() << "read bufV error size: " << ret << endl;
//            break;
//        }

        int ret = fread(mPicIn.img.plane[0], 1, size, mFile);
        if (ret != size) {
            qDebug() << "read bufY error size: " << ret << endl;
            break;
        }

        ret = fread(mPicIn.img.plane[1], 1, size / 4, mFile);
        if (ret != size / 4) {
            qDebug() << "read bufU error size: " << ret << endl;
            break;
        }

        ret = fread(mPicIn.img.plane[2], 1, size / 4, mFile);
        if (ret != size / 4) {
            qDebug() << "read bufV error size: " << ret << endl;
            break;
        }

        //encode buf
//        mPicIn.img.plane[0] = (uint8_t *)bufY;
//        mPicIn.img.plane[1] = (uint8_t *)bufU;
//        mPicIn.img.plane[2] = (uint8_t *)bufV;

        mPicIn.i_pts = mIndex;
        int frameSize = x264_encoder_encode(mX264, &mNalu, &mNaluIndex, &mPicIn, &mPicOut);
        if (frameSize < 0) {
            x264_encoder_close(mX264);
            qDebug() << "encode error\n";
            break;
        }

        buffer->data = (mNalu[0].p_payload);
        buffer->len = frameSize;

        qDebug() << "encode one frame : " << frameSize << endl;
        if (mCallback != nullptr) {
            mCallback->onEncodedCallback(buffer->data, buffer->len);
        }

        mIndex++;
    }

    qDebug() << "zhangyu first encode count is: " << mIndex << endl;
}

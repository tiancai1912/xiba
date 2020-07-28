#include <QCoreApplication>

#include <stdint.h>
#include <stdio.h>
#include <x264.h>
#include "x264encoder.h"

#include <QDebug>

#define FAIL_IF_ERROR( cond, ... )\
do\
{\
    if( cond )\
    {\
        fprintf( stderr, __VA_ARGS__ );\
        goto fail;\
    }\
} while( 0 )


class test : public x264Encoder::Callback
{
public:
    test() {
        mFile = fopen("test.h264", "wb");
        if (mFile == nullptr) {
            qDebug() << "file is null\n";
        }
    }

    void onData(char *data, int len);

    void closeFile() {
        fclose(mFile);
        mFile = nullptr;
    }

private:
    FILE *mFile;
};

void test::onData(char *data, int len)
{
//    qDebug() << "pic len: " << len << endl;
    printf("zhangyu onData: %d", len);
    if (fwrite(data, len, 1, mFile) != len) {
        qDebug() << "write failed!" << len << endl;
    }
}


int main(int argc, char *argv[])
{
    //init params
    x264Encoder encoder;
    x264Encoder::EncodingFormat format;
    format.width = 1280;
    format.height = 720;
    format.format = X264_CSP_I420;
    format.bitDepth = 8;
    format.path = "test.i420";

    //open encoder
    encoder.init(format);

    test *tt = new test();
    encoder.setCallback(tt);

    encoder.start();

    //close encoder
    encoder.stop();
    tt->closeFile();

    return 0;
}

#ifndef VIDEOPARSER_H
#define VIDEOPARSER_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <vector>

extern "C" {

#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>

}

#define NALU_SLICE  1
#define NALU_IDR  5
#define NALU_SEI  6
#define NALU_SPS  7
#define NALU_PPS  8

#define MAX_DATA_SIZE 100

#define MAX_NALU_READ_SIZE 500

class VideoParser
{
public:
    VideoParser();
    ~VideoParser();

    struct PacketItem {
        int length;
        int64_t pts;
        int64_t duration;
        char *data;

        PacketItem()
        {
            length = -1;
            pts = 0;
            duration = 0;
//            data = (char *)malloc(50000);
//            memset(data, 0, 50000);
            data = (char *)malloc(100);
            memset(data, 0, 100);
        }

        ~PacketItem() {
            if (data != NULL) {
                free(data);
                data = NULL;
            }
        }

        void copy(PacketItem *other)
        {
            length = other->length;
            pts = other->pts;
            duration = other->duration;
            memcpy(data, other->data, other->length);
        }
    };

    PacketItem * readNaluItems();

    int parseNaluItem(PacketItem *item);


    bool openFile(char *url);
    void closeFile();

    static void parseData(void *arg);
    void handleParseData();

private:
    std::vector<PacketItem *> mPackets;
    int mFrameCount;

    AVPacket pkt;
    AVFormatContext *m_fmt_ctx;
    int m_video_stream_idx;
    AVBitStreamFilterContext *m_video_bsf;

};

#endif // VIDEOPARSER_H

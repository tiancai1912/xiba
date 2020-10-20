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
            data = (char *)malloc(500000);
            memset(data, 0, 500000);
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

    void parseNaluItem(PacketItem *item);


    bool openFile(char *url);
    void closeFile();

private:
    FILE *mFd;
    std::vector<AVPacket *> mPackets;
    int mFrameCount;

    AVPacket pkt;
    AVFormatContext *m_fmt_ctx;
    int m_video_stream_idx;
    const char *m_src_filename;
    AVBitStreamFilterContext *m_video_bsf;

};

#endif // VIDEOPARSER_H

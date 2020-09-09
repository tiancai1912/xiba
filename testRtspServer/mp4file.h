#ifndef MP4FILE_H
#define MP4FILE_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>

extern "C" {

#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>

}

#define ADTS_HEADER_SIZE 7

static const char *TAG = "MP4Parser";


class Mp4File
{
public:
    Mp4File();

    typedef struct {
        int write_adts;
        int object_type;
        int sample_rate_index;
        int channel_conf;
    } ADTSContext;

    enum PacketType {
        PACKET_UNKNOWN,
        PACKET_VIDEO,
        PACKET_AUDIO
    };

    struct PacketItem{
        int length;
        char *data;
        PacketType type;

        PacketItem()
        {
            length = -1;
            type = PACKET_UNKNOWN;
            data = (char *)malloc(500000);
        }
    };

    int openFile(const char *fileName);
    void closeFile();
    Mp4File::PacketItem * getOneFrame();

protected:
    int aac_decode_extradata(ADTSContext *adts, unsigned char *pbuf, int bufsize);
    int aac_set_adts_head(ADTSContext *acfg, unsigned char *buf, int size);


private:
    AVPacket pkt;
    AVFormatContext *m_fmt_ctx;

    int m_video_stream_idx;
    int m_audio_stream_idx;

    const char *m_src_filename;

    ADTSContext  m_adts_ctx;
    AVBitStreamFilterContext *m_video_bsf;
    AVBitStreamFilterContext *m_audio_bsf;
};

#endif // MP4FILE_H

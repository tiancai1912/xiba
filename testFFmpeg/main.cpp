#include <QCoreApplication>

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

typedef struct {
    int write_adts;
    int object_type;
    int sample_rate_index;
    int channel_conf;
} ADTSContext;


static int aac_decode_extradata(ADTSContext *adts, unsigned char *pbuf, int bufsize) {
      int aot, aotext, samfreindex;
      int i, channelconfig;
      unsigned char *p = pbuf;
      if (!adts || !pbuf || bufsize < 2) {
            printf("%s, run aac_decode_extradata failed\n", TAG);
            return -1;
      }
      aot = (p[0]>>3)&0x1f;
      if (aot == 31) {
            aotext = (p[0]<<3 | (p[1]>>5)) & 0x3f;
            aot = 32 + aotext;
            samfreindex = (p[1]>>1) & 0x0f;
            if (samfreindex == 0x0f) {
                  channelconfig = ((p[4]<<3) | (p[5]>>5)) & 0x0f;
            }
            else {
                  channelconfig = ((p[1]<<3)|(p[2]>>5)) & 0x0f;
            }
      }
      else {
            samfreindex = ((p[0]<<1)|p[1]>>7) & 0x0f;
            if (samfreindex == 0x0f) {
                  channelconfig = (p[4]>>3) & 0x0f;
            }
            else {
                  channelconfig = (p[1]>>3) & 0x0f;
            }
      }
#ifdef AOT_PROFILE_CTRL
      if (aot < 2) aot = 2;
#endif
      adts->object_type = aot-1;
      adts->sample_rate_index = samfreindex;
      adts->channel_conf = channelconfig;
      adts->write_adts = 1;
      return 0;
}

static int aac_set_adts_head(ADTSContext *acfg, unsigned char *buf, int size) {
      unsigned char byte;
      if (size < ADTS_HEADER_SIZE) {
          printf("%s, run aac_set_adts_head failed, input size < ADTS_HEADER_SIZE\n", TAG);
          return -1;
      }

      buf[0] = 0xff;
      buf[1] = 0xf1;
      byte = 0;
      byte |= (acfg->object_type & 0x03) << 6;
      byte |= (acfg->sample_rate_index & 0x0f) << 2;
      byte |= (acfg->channel_conf & 0x07) >> 2;
      buf[2] = byte;
      byte = 0;
      byte |= (acfg->channel_conf & 0x07) << 6;
      byte |= (ADTS_HEADER_SIZE + size) >> 11;
      buf[3] = byte;
      byte = 0;
      byte |= (ADTS_HEADER_SIZE + size) >> 3;
      buf[4] = byte;
      byte = 0;
      byte |= ((ADTS_HEADER_SIZE + size) & 0x7) << 5;
      byte |= (0x7ff >> 6) & 0x1f;
      buf[5] = byte;
      byte = 0;
      byte |= (0x7ff & 0x3f) << 2;
      buf[6] = byte;

      return 0;
}

int main(int argc, char *argv[])
{
    AVPacket pkt;
    AVFormatContext *fmt_ctx = NULL;

    int ret = 0;
    int video_stream_idx = -1;
    int audio_stream_idx = -1;

    const char *src_filename = NULL;
    const char *video_dst_filename = NULL;
    const char *audio_dst_filename = NULL;

    FILE *video_dst_file = NULL;
    FILE *audio_dst_file = NULL;


    if (argc != 4) {
        fprintf(stderr, "usage: %s  input_file video_output_file audio_output_file\n"
                        "API example program to show how to read frames from an input file.\n"
                        "This program reads frames from a file, decodes them, and writes decoded\n"
                        "video frames to a rawvideo file named video_output_file, and decoded\n"
                        "audio frames to a rawaudio file named audio_output_file.\n",
                argv[0]);
        exit(1);
    }

    src_filename = argv[1];
    video_dst_filename = argv[2];
    audio_dst_filename = argv[3];

    /* open input file, and allocate format context */
    if (avformat_open_input(&fmt_ctx, src_filename, NULL, NULL) < 0) {
        fprintf(stderr, "Could not open source file %s\n", src_filename);
        exit(1);
    }

    /* retrieve stream information */
    if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
        fprintf(stderr, "Could not find stream information\n");
        exit(1);
    }

    /* dump input information to stderr */
    av_dump_format(fmt_ctx, 0, src_filename, 0);

    //find video stream index
    ret = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (ret < 0) {
        fprintf(stderr, "Could not find %s stream in input file '%s'\n",
                av_get_media_type_string(AVMEDIA_TYPE_AUDIO), src_filename);
        return ret;
    } else {
        video_stream_idx = ret;
    }

    //find audio stream index
    ret = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    if (ret < 0) {
        fprintf(stderr, "Could not find %s stream in input file '%s'\n",
                av_get_media_type_string(AVMEDIA_TYPE_AUDIO), src_filename);
        return ret;
    } else {
        audio_stream_idx = ret;
    }


    /* initialize packet, set data to NULL, let the demuxer fill it */
    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;

    video_dst_file = fopen(video_dst_filename, "wb+");
    if (!video_dst_file) {
        printf("open %s failed!\n", video_dst_filename);
        goto end;
    }

    audio_dst_file = fopen(audio_dst_filename, "wb+");
    if (!audio_dst_file) {
        printf("open %s failed!\n", audio_dst_filename);
        goto end;
    }

    ADTSContext  m_adts_ctx;
    AVBitStreamFilterContext *m_video_bsf;
    AVBitStreamFilterContext *m_audio_bsf;
    m_video_bsf = av_bitstream_filter_init("h264_mp4toannexb");
    m_audio_bsf = av_bitstream_filter_init("aac_adtstoasc");

    /* read frames from the file */
    while (av_read_frame(fmt_ctx, &pkt) >= 0) {
        if (pkt.stream_index == video_stream_idx) {
            uint8_t *out_data = NULL;
            int out_size = 0;

            int err = av_bitstream_filter_filter(m_video_bsf, fmt_ctx->streams[video_stream_idx]->codec, NULL, &out_data, &out_size, pkt.data, pkt.size, pkt.flags & AV_PKT_FLAG_KEY);
            if (err <=0 ) {
                printf("%s, run read, av_bitstream_filter_filter failed\n", TAG);
            }


            int ret = fwrite(out_data, 1, out_size, video_dst_file);
//            int ret = fwrite(pkt.data, 1, pkt.size, video_dst_file);
            if (ret != out_size) {
                printf("fwrite video pkt failed!\n");
            }

        } else if (pkt.stream_index == audio_stream_idx) {
            aac_decode_extradata(&m_adts_ctx, fmt_ctx->streams[audio_stream_idx]->codec->extradata, fmt_ctx->streams[audio_stream_idx]->codec->extradata_size);

            unsigned char adtsHdr[ADTS_HEADER_SIZE] = {0};
            aac_set_adts_head(&m_adts_ctx, adtsHdr, pkt.size);


            int ret = fwrite(adtsHdr, 1, ADTS_HEADER_SIZE, audio_dst_file);
            if (ret != ADTS_HEADER_SIZE) {
                printf("fwrite audio adts header failed!\n");
            }

            ret = fwrite(pkt.data, 1, pkt.size, audio_dst_file);
            if (ret != pkt.size) {
                printf("fwrite audio pkt failed!\n");
            }
        }

        av_packet_unref(&pkt);
        if (ret < 0)
            break;
    }

    printf("Demuxing succeeded.\n");

end:
    fclose(video_dst_file);
    fclose(audio_dst_file);

    av_bitstream_filter_close(m_video_bsf);
    av_bitstream_filter_close(m_audio_bsf);

    avformat_close_input(&fmt_ctx);

    return ret < 0;
}

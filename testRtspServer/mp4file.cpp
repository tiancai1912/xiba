#include "mp4file.h"

#include "net/rtp.h"
Mp4File::Mp4File()
{
    m_fmt_ctx = NULL;
    m_video_stream_idx = -1;
    m_audio_stream_idx = -1;
    m_src_filename = NULL;
    m_video_bsf = NULL;
    m_audio_bsf = NULL;

}

int Mp4File::aac_decode_extradata(ADTSContext *adts, unsigned char *pbuf, int bufsize) {
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

int Mp4File::aac_set_adts_head(ADTSContext *acfg, unsigned char *buf, int size) {
      unsigned char byte;
      if (size < ADTS_HEADER_SIZE) {
          printf("%s, run aac_set_adts_head failed, input size %d < ADTS_HEADER_SIZE\n", TAG, size);
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

int Mp4File::openFile(const char *fileName)
{
    /* open input file, and allocate format context */
    if (avformat_open_input(&m_fmt_ctx, fileName, NULL, NULL) < 0) {
        fprintf(stderr, "Could not open source file %s\n", fileName);
        return -1;
    }

    m_src_filename = fileName;

    /* retrieve stream information */
    if (avformat_find_stream_info(m_fmt_ctx, NULL) < 0) {
        fprintf(stderr, "Could not find stream information\n");
        return -1;
    }

    /* dump input information to stderr */
    av_dump_format(m_fmt_ctx, 0, m_src_filename, 0);

    //find video stream index
    int ret = av_find_best_stream(m_fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (ret < 0) {
        fprintf(stderr, "Could not find %s stream in input file '%s'\n",
                av_get_media_type_string(AVMEDIA_TYPE_AUDIO), m_src_filename);
        return ret;
    } else {
        m_video_stream_idx = ret;
    }

    //find audio stream index
    ret = av_find_best_stream(m_fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    if (ret < 0) {
        fprintf(stderr, "Could not find %s stream in input file '%s'\n",
                av_get_media_type_string(AVMEDIA_TYPE_AUDIO), m_src_filename);
        return ret;
    } else {
        m_audio_stream_idx = ret;
    }

    m_video_bsf = av_bitstream_filter_init("h264_mp4toannexb");
    m_audio_bsf = av_bitstream_filter_init("aac_adtstoasc");

    return 0;

}

void Mp4File::closeFile()
{
    av_bitstream_filter_close(m_video_bsf);
    av_bitstream_filter_close(m_audio_bsf);

    avformat_close_input(&m_fmt_ctx);
}

Mp4File::PacketItem * Mp4File::getOneFrame()
{
    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;

    int ret = 0;
    /* read frames from the file */
    ret = av_read_frame(m_fmt_ctx, &pkt);
    if (ret >= 0) {
//        printf("av read frame success!\n");
        if (pkt.stream_index == m_video_stream_idx) {
//            printf("av read video frame!\n");
            uint8_t *out_data = NULL;
            int out_size = 0;

            int err = av_bitstream_filter_filter(m_video_bsf, m_fmt_ctx->streams[m_video_stream_idx]->codec, NULL, &out_data, &out_size, pkt.data, pkt.size, pkt.flags & AV_PKT_FLAG_KEY);
            if (err <=0 ) {
                printf("%s, run read, av_bitstream_filter_filter failed\n", TAG);
            }

//            printf("pkt size: %d, out data size: %d\n", pkt.size, out_size);
//            printf("out data: %02x %02x %02x %02x\n", out_data[0], out_data[1], out_data[2], out_data[3]);
//            printf("pkt data: %02x %02x %02x %02x\n", pkt.data[0], pkt.data[1], pkt.data[2], pkt.data[3]);

            printf("get video frame pts: %ld, dts: %ld\n", pkt.pts, pkt.dts);

            PacketItem *item = new PacketItem();
            item->type = PacketType::PACKET_VIDEO;
//            item->length = pkt.size;
//            memcpy(item->data, (char *)(pkt.data), pkt.size);

            item->length = out_size;
            memcpy(item->data, (char *)out_data, out_size);
//            printf("video frame %02x %02x %02x %02x \n", item->data[0], item->data[1], item->data[2], item->data[3]);
//            printf("out data: %02x %02x %02x %02x\n", out_data[0], out_data[1], out_data[2], out_data[3]);
//            printf("pkt data: %02x %02x %02x %02x\n", pkt.data[0], pkt.data[1], pkt.data[2], pkt.data[3]);

            return item;

        } else if (pkt.stream_index == m_audio_stream_idx) {
//            aac_decode_extradata(&m_adts_ctx, m_fmt_ctx->streams[m_audio_stream_idx]->codec->extradata, m_fmt_ctx->streams[m_audio_stream_idx]->codec->extradata_size);

//            printf("the extra data size: %d\n", m_fmt_ctx->streams[m_audio_stream_idx]->codec->extradata_size);

//            unsigned char adtsHdr[ADTS_HEADER_SIZE] = {0};
//            aac_set_adts_head(&m_adts_ctx, adtsHdr, pkt.size + 7);

//            rtp g_rtp;
//            struct AdtsHeader adtsHeader;
//            g_rtp.parseAdtsHeader(adtsHdr, &adtsHeader);

//            printf("the pckt size: %d, the length: %d", pkt.size, adtsHeader.aacFrameLength - 7);

            printf("get audio frame pts: %ld, dts: %ld\n", pkt.pts, pkt.dts);

            PacketItem *item = new PacketItem();
//            item->length =  adtsHeader.aacFrameLength - 7;
            item->length = pkt.size;
//            item->length = pkt.size + 7;
            item->type = PacketType::PACKET_AUDIO;
//            item->data = strncpy(item->data, (const char *)adtsHdr, 7);
//            item->data = strncpy(item->data + 7, (const char *)(pkt.data), pkt.size);
//            item->data = strncpy(item->data, (const char *)(pkt.data), pkt.size);
//            item->data = strncpy(item->data, (const char *)(pkt.data), item->length);
            memcpy(item->data, (const char *)(pkt.data), item->length);
            return item;
        }

        av_packet_unref(&pkt);
//        if (ret < 0)
//            return NULL;
    } else {
        printf("av read frame failed!\n");
        return NULL;
    }

    return NULL;
}

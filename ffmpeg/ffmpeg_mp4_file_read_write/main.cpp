#include <QCoreApplication>
#include <QDebug>

#include <unistd.h>

extern "C" {

#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>

}

/**
 * @brief read_file
 * @return
 * 1. mp4文件读取出来的h264数据是没有start code
 * 2. 保存成h264文件时候可以使用bsf过滤器手动添加start code
 * 3. h264文件保存时候虽然保存的是原始的压缩数据，但是需要手动添加start code，方便解码器解码
 */
int read_file()
{
    AVFormatContext *fmt_ctx = nullptr;
    int ret = avformat_open_input(&fmt_ctx, "test.mp4", nullptr, nullptr);
    if (ret != 0) {
        qDebug() << "avformat open input failed!\n";
        return -1;
    }

    ret = avformat_find_stream_info(fmt_ctx, nullptr);
    if (ret < 0) {
        qDebug() << "avformat find stream info failed!\n";
        return -1;
    }

    int video_index = -1;
    int audio_index = -1;
    int streams = fmt_ctx->nb_streams;
    const AVBitStreamFilter * filter = nullptr;
    AVBSFContext *bsf_ctx = nullptr;

    for (int i = 0; i < streams; i++) {
        auto stream = fmt_ctx->streams[i];
        qDebug() << "stream info: " << stream->index << endl;
        if (stream->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_index = stream->index;
            filter = av_bsf_get_by_name("h264_mp4toannexb");
            if (filter == nullptr) {
                qDebug() << "create bsf failed!\n";
                return -1;
            }

            ret = av_bsf_alloc(filter, &bsf_ctx);
            if (ret != 0) {
                qDebug() << "alloc bsf failed!\n";
                return -1;
            }

            ret = avcodec_parameters_copy(bsf_ctx->par_in, stream->codecpar);
            if (ret < 0) {
                qDebug() << "copy params failed!\n";
                return -1;
            }

            av_bsf_init(bsf_ctx);
        } else if (stream->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            audio_index = stream->index;
        }
    }

    AVPacket packet;
    FILE *f = fopen("test.h264", "w+");
    if (f == nullptr) {
        qDebug() << "open test.h264 failed!\n";
        return -1;
    }

    bool find_key = false;

    while(1) {
        if (av_read_frame(fmt_ctx, &packet) != 0) {
            qDebug() << "read frame failed!" << endl;
            break;
        }
        if (packet.flags & AV_PKT_FLAG_KEY) {
            qDebug() << "find key frame\n";
            find_key = true;
        }

        if (!find_key) {
            continue;
        }

        if (packet.stream_index == video_index) {
            ret = av_bsf_send_packet(bsf_ctx, &packet);
            if (ret != 0) {
                qDebug() << "bsf send packet failed!\n";
                break;
            }

            ret = av_bsf_receive_packet(bsf_ctx, &packet);
            if (ret != 0) {
                qDebug() << "bsf receive packet failed!\n";
                continue;
            }

//            qDebug() << "data 0: " <<  packet.data[0] << "data 1:" << packet.data[1] << "data 2: " << packet.data[2] << "data 3: " << packet.data[3] << "data 4: " << packet.data[4] << endl;
            ret = fwrite(packet.data, 1, packet.size, f);
            if (ret != packet.size) {
                qDebug() << "write frame failed!\n";
                return -1;
            }
        }

//        qDebug() << "write frame success!" << "frame size: " << packet.size << endl;
        av_packet_unref(&packet);
    }

    fclose(f);
    f = nullptr;

    av_bsf_free(&bsf_ctx);
    bsf_ctx = nullptr;

    avformat_close_input(&fmt_ctx);
    fmt_ctx = nullptr;

    return 0;
}

/**
 * @brief write_file
 * @return
 * 1. ffmpeg从mp4读取出来的h264数据是没有start code
 * 2. 将h264数据写入mp4文件时候需要使用bsf过滤器，手动添加start code，不然mp4播放的时候识别不到sps，pps
 */
int write_file()
{
    AVOutputFormat *out_fmt = nullptr;
    AVFormatContext *fmt_ctx = nullptr;

    const AVBitStreamFilter * filter = nullptr;
    AVBSFContext *bsf_ctx = nullptr;

    int ret = avformat_alloc_output_context2(&fmt_ctx, nullptr, nullptr, "output.mp4");
    if (ret < 0) {
        qDebug() << "avformat aaloc output context failed!\n";
        return -1;
    }

    out_fmt = fmt_ctx->oformat;
    if (out_fmt->video_codec != AV_CODEC_ID_NONE) {
        AVStream *video_stream = avformat_new_stream(fmt_ctx, nullptr);
        if (video_stream == nullptr) {
            qDebug() << "new video stream failed!\n";
            return -1;
        }

        AVCodecContext *codec_ctx = nullptr;
        codec_ctx = video_stream->codec;

        //find input video stream
        AVFormatContext *input_fmt_ctx = nullptr;
        int ret = avformat_open_input(&input_fmt_ctx, "test.mp4", nullptr, nullptr);
        if (ret != 0) {
            qDebug() << "avformat open input failed!\n";
            return -1;
        }

        ret = avformat_find_stream_info(input_fmt_ctx, nullptr);
        if (ret < 0) {
            qDebug() << "avformat find input stream info failed!\n";
            return -1;
        }

        int video_index = -1;
        int audio_index = -1;
        int streams = input_fmt_ctx->nb_streams;
        AVStream *input_video_stream = nullptr;
        for (int i = 0; i < streams; i++) {
            auto stream = input_fmt_ctx->streams[i];
            qDebug() << "stream info: " << stream->index << endl;
            if (stream->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
                video_index = stream->index;
                input_video_stream = stream;

                filter = av_bsf_get_by_name("h264_mp4toannexb");
                if (filter == nullptr) {
                    qDebug() << "create bsf failed!\n";
                    return -1;
                }

                ret = av_bsf_alloc(filter, &bsf_ctx);
                if (ret != 0) {
                    qDebug() << "alloc bsf failed!\n";
                    return -1;
                }

                ret = avcodec_parameters_copy(bsf_ctx->par_in, stream->codecpar);
                if (ret < 0) {
                    qDebug() << "copy params failed!\n";
                    return -1;
                }

                av_bsf_init(bsf_ctx);

            } else if (stream->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
                audio_index = stream->index;
            }
        }


        codec_ctx->bit_rate = input_video_stream->codec->bit_rate;
        codec_ctx->codec_id = input_video_stream->codec->codec_id;
        codec_ctx->codec_type = input_video_stream->codec->codec_type;
        codec_ctx->time_base.num = input_video_stream->time_base.num;
        codec_ctx->time_base.den = input_video_stream->time_base.den;
        codec_ctx->width = input_video_stream->codec->width;
        codec_ctx->height = input_video_stream->codec->height;
        codec_ctx->pix_fmt = input_video_stream->codec->pix_fmt;
        codec_ctx->flags = input_video_stream->codec->flags;
        codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        codec_ctx->me_range = input_video_stream->codec->me_range;
        codec_ctx->max_qdiff = input_video_stream->codec->max_qdiff;
        codec_ctx->qmin = input_video_stream->codec->qmin;
        codec_ctx->qmax = input_video_stream->codec->qmax;
        codec_ctx->qcompress = input_video_stream->codec->qcompress;

        video_stream->time_base = input_video_stream->time_base;

        avio_open(&fmt_ctx->pb, "output.mp4", AVIO_FLAG_WRITE);
        avformat_write_header(fmt_ctx, nullptr);

        AVPacket packet;
//        bool find_key = false;
        while(1) {
            if (av_read_frame(input_fmt_ctx, &packet) != 0) {
                qDebug() << "read frame failed!" << endl;
                break;
            }

//            if (packet.flags & AV_PKT_FLAG_KEY) {
//                qDebug() << "find key frame\n";
//                find_key = true;
//            }

//            if (!find_key) {
//                continue;
//            }

            if (packet.stream_index == video_index) {
                ret = av_bsf_send_packet(bsf_ctx, &packet);
                if (ret != 0) {
                    qDebug() << "bsf send packet failed!\n";
                    break;
                }

                ret = av_bsf_receive_packet(bsf_ctx, &packet);
                if (ret != 0) {
                    qDebug() << "bsf receive packet failed!\n";
                    continue;
                }

                packet.stream_index = input_video_stream->index;
                int ret = av_write_frame(fmt_ctx, &packet);
                if (ret != 0) {
                    qDebug() << "write frame failed!\n";
                    break;
                }
            }

            av_packet_unref(&packet);
//            count++;
        }

        avformat_close_input(&input_fmt_ctx);
        av_write_trailer(fmt_ctx);
        avcodec_close(fmt_ctx->streams[0]->codec);
        av_freep(&fmt_ctx->streams[0]->codec);
        av_freep(&fmt_ctx->streams[0]);
        avio_close(fmt_ctx->pb);
        av_free(fmt_ctx);
    }

    return 0;
}

/**
 * @brief read_rtsp
 * @return
 * 1. rtsp读取到的video data为h264时候不一定有start code，所以需要服务器端那边使用bsf进行转换下
 * 2. 即使h264数据有start code之后，可能会有sps nalu,但是不一定有pps nalu，所以目前处理方式是在关键帧前面加上extra data,保证一定有sps,pps
 */
int read_rtsp()
{
    AVFormatContext *fmt_ctx = nullptr;
    int ret = avformat_open_input(&fmt_ctx, "rtsp://192.168.0.105/live", nullptr, nullptr);
    if (ret != 0) {
        qDebug() << "avformat open input failed!\n";
        return -1;
    }

    ret = avformat_find_stream_info(fmt_ctx, nullptr);
    if (ret < 0) {
        qDebug() << "avformat find stream info failed!\n";
        return -1;
    }

    int video_index = -1;
    int audio_index = -1;
    int streams = fmt_ctx->nb_streams;
    AVStream *video_stream = nullptr;

    for (int i = 0; i < streams; i++) {
        auto stream = fmt_ctx->streams[i];
        qDebug() << "stream info: " << stream->index << endl;
        if (stream->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_index = stream->index;
            video_stream = stream;
        } else if (stream->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            audio_index = stream->index;
        }
    }

    AVPacket packet;
    FILE *f = fopen("test.h264", "w+");
    if (f == nullptr) {
        qDebug() << "open test.h264 failed!\n";
        return -1;
    }

    bool find_key = false;
    int count = 0;
    while(count < 300) {
        if (av_read_frame(fmt_ctx, &packet) != 0) {
            qDebug() << "read frame failed!" << endl;
            break;
        }
        if (packet.flags & AV_PKT_FLAG_KEY) {
            qDebug() << "find key frame\n";
            find_key = true;
        }

        if (!find_key) {
            av_packet_unref(&packet);
            continue;
        }

        if (packet.stream_index == video_index) {
            if (packet.flags & AV_PKT_FLAG_KEY) {
                fwrite(video_stream->codec->extradata, 1, video_stream->codec->extradata_size, f);
            }

            ret = fwrite(packet.data, 1, packet.size, f);
            if (ret != packet.size) {
                qDebug() << "write frame failed!\n";
                return -1;
            }

            count++;
        }

        qDebug() << "write frame success!" << "frame size: " << packet.size << endl;
        av_packet_unref(&packet);
    }

    fclose(f);
    f = nullptr;

    avformat_close_input(&fmt_ctx);
    fmt_ctx = nullptr;

    return 0;
}

int write_rtsp()
{
    AVOutputFormat *out_fmt = nullptr;
    AVFormatContext *fmt_ctx = nullptr;

    const AVBitStreamFilter * filter = nullptr;
    AVBSFContext *bsf_ctx = nullptr;

//    int ret = avformat_alloc_output_context2(&fmt_ctx, nullptr, nullptr, "rtsp://192.168.0.105/live");
    int ret = avformat_alloc_output_context2(&fmt_ctx, nullptr, "rtsp", "rtsp://192.168.0.105/live");
    if (ret < 0) {
        qDebug() << "avformat aaloc output context failed!\n";
        return -1;
    }

    out_fmt = fmt_ctx->oformat;
    if (out_fmt->video_codec != AV_CODEC_ID_NONE) {
        AVStream *video_stream = avformat_new_stream(fmt_ctx, nullptr);
        if (video_stream == nullptr) {
            qDebug() << "new video stream failed!\n";
            return -1;
        }

        AVCodecContext *codec_ctx = nullptr;
        codec_ctx = video_stream->codec;

        //find input video stream
        AVFormatContext *input_fmt_ctx = nullptr;
        int ret = avformat_open_input(&input_fmt_ctx, "test.mp4", nullptr, nullptr);
        if (ret != 0) {
            qDebug() << "avformat open input failed!\n";
            return -1;
        }

        ret = avformat_find_stream_info(input_fmt_ctx, nullptr);
        if (ret < 0) {
            qDebug() << "avformat find input stream info failed!\n";
            return -1;
        }

        int video_index = -1;
        int audio_index = -1;
        int streams = input_fmt_ctx->nb_streams;
        AVStream *input_video_stream = nullptr;
        for (int i = 0; i < streams; i++) {
            auto stream = input_fmt_ctx->streams[i];
            qDebug() << "stream info: " << stream->index << endl;
            if (stream->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
                video_index = stream->index;
                input_video_stream = stream;

                filter = av_bsf_get_by_name("h264_mp4toannexb");
                if (filter == nullptr) {
                    qDebug() << "create bsf failed!\n";
                    return -1;
                }

                ret = av_bsf_alloc(filter, &bsf_ctx);
                if (ret != 0) {
                    qDebug() << "alloc bsf failed!\n";
                    return -1;
                }

                ret = avcodec_parameters_copy(bsf_ctx->par_in, stream->codecpar);
                if (ret < 0) {
                    qDebug() << "copy params failed!\n";
                    return -1;
                }

                av_bsf_init(bsf_ctx);

            } else if (stream->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
                audio_index = stream->index;
            }
        }


        codec_ctx->bit_rate = input_video_stream->codec->bit_rate;
        codec_ctx->codec_id = input_video_stream->codec->codec_id;
        codec_ctx->codec_type = input_video_stream->codec->codec_type;
        codec_ctx->time_base.num = input_video_stream->time_base.num;
        codec_ctx->time_base.den = input_video_stream->time_base.den;
        codec_ctx->width = input_video_stream->codec->width;
        codec_ctx->height = input_video_stream->codec->height;
        codec_ctx->pix_fmt = input_video_stream->codec->pix_fmt;
        codec_ctx->flags = input_video_stream->codec->flags;
        codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        codec_ctx->me_range = input_video_stream->codec->me_range;
        codec_ctx->max_qdiff = input_video_stream->codec->max_qdiff;
        codec_ctx->qmin = input_video_stream->codec->qmin;
        codec_ctx->qmax = input_video_stream->codec->qmax;
        codec_ctx->qcompress = input_video_stream->codec->qcompress;

        video_stream->time_base = input_video_stream->time_base;

        avio_open(&fmt_ctx->pb, "rtsp://192.168.0.105/live", AVIO_FLAG_WRITE);
        avformat_write_header(fmt_ctx, nullptr);

        AVPacket packet;
//        bool find_key = false;
        while(1) {
            if (av_read_frame(input_fmt_ctx, &packet) != 0) {
                qDebug() << "read frame failed!" << endl;
                break;
            }

//            if (packet.flags & AV_PKT_FLAG_KEY) {
//                qDebug() << "find key frame\n";
//                find_key = true;
//            }

//            if (!find_key) {
//                continue;
//            }

            if (packet.stream_index == video_index) {
                ret = av_bsf_send_packet(bsf_ctx, &packet);
                if (ret != 0) {
                    qDebug() << "bsf send packet failed!\n";
                    break;
                }

                ret = av_bsf_receive_packet(bsf_ctx, &packet);
                if (ret != 0) {
                    qDebug() << "bsf receive packet failed!\n";
                    continue;
                }

                packet.stream_index = input_video_stream->index;
                qDebug() << "write one frame: " << packet.size << endl;
                int ret = av_write_frame(fmt_ctx, &packet);
                if (ret != 0) {
                    qDebug() << "write frame failed!\n";
                    break;
                }
            }

            av_packet_unref(&packet);
//            count++;

            usleep(40000);
        }

        avformat_close_input(&input_fmt_ctx);
        av_write_trailer(fmt_ctx);
        avcodec_close(fmt_ctx->streams[0]->codec);
        av_freep(&fmt_ctx->streams[0]->codec);
        av_freep(&fmt_ctx->streams[0]);
        avio_close(fmt_ctx->pb);
        av_free(fmt_ctx);
    }

    return 0;
}

int main(int argc, char *argv[])
{
    qDebug() << "hello world!\n";

//    read_file();
//    write_file();
//    read_rtsp();
    write_rtsp();

    return 0;
}

#include <QCoreApplication>
#include <QDebug>

extern "C" {

#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>

}

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

int main(int argc, char *argv[])
{
    qDebug() << "hello world!\n";

    read_file();

    return 0;
}

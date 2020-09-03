#include "mainwindow.h"
#include <QApplication>

extern "C" {
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

#include "dirent.h"
#include <list>

static bool convertImageFormat(unsigned char *src_data, int src_width, int src_height, AVPixelFormat src_format,
                               unsigned char *dst_data, int dst_width, int dst_height, AVPixelFormat dst_format)
{
    struct SwsContext *sws_ctx;
    sws_ctx = sws_getContext(src_width, src_height, src_format,
                   dst_width, dst_height, dst_format,
                   SWS_BILINEAR, NULL, NULL, NULL);
    if (!sws_ctx) {
        printf("get sws context failed!");
        return false;
    }


    unsigned char *src[4];
    int src_linesize[4];

    unsigned char *dst[4];
    int dst_linesize[4];

    int dst_buffer_size = 0;

    src[0] = src_data;
    int ret = 0;
    ret = av_image_alloc(src, src_linesize, src_width, src_height, src_format, 16);
    if (ret < 0) {
        printf("alloc src image failed!\n");
        return false;
    }

    ret = av_image_alloc(dst, dst_linesize, dst_width, dst_height, dst_format, 1);
    if (ret < 0) {
        printf("alloc dst image failed!\n");
        return false;
    }

    dst_buffer_size = ret;

    sws_scale(sws_ctx, (const unsigned char * const*)src, src_linesize, 0, src_width, dst, dst_linesize);
    dst_data = dst[0];

    av_freep(&src[0]);
    av_freep(&dst[0]);
    sws_freeContext(sws_ctx);
    return true;
}

static bool v4l2_is_v4l_dev(const char *name)
{
    return (!strncmp(name, "video", 5) ||
            !strncmp(name, "radio", 5) ||
            !strncmp(name, "vbi", 3) ||
            !strncmp(name, "v4l-subdev", 10));
}

static std::list<std::string> findCamera()
{
    std::list<std::string> dev_list;
    DIR *dir = opendir("/dev");
    if (!dir) {
        printf("open dev dir failed!\n");
        return dev_list;
    }

    struct dirent *entry;
    while ((entry = readdir(dir))) {
        if (!v4l2_is_v4l_dev(entry->d_name)) {
            continue;
        }

        char device_name[256] = {0};
        sprintf(device_name, "/dev/%s", entry->d_name);
        dev_list.push_back(std::string(device_name));
    }

    return dev_list;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}

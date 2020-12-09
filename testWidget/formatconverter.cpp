#include "formatconverter.h"

FormatConverter::FormatConverter()
{

}

int FormatConverter::convert(unsigned char *src_data[4], int *src_linesize, unsigned char *dst_data[4], int *dst_linesize)
{
    SwsContext *context = sws_getContext(160, 120, AVPixelFormat::AV_PIX_FMT_YUYV422, 160, 120, AVPixelFormat::AV_PIX_FMT_RGB24, SWS_BILINEAR, NULL,NULL, NULL);
    if (context == nullptr) {
        printf("get sws context failed!\n");
    }

    int src_linse[4] = {320, 0, 0, 0};
//    av_image_fill_linesizes(src_linse, AVPixelFormat::AV_PIX_FMT_YUYV422, 160);
//    int ret = av_image_alloc(src_data, src_linse, 160, 120, AVPixelFormat::AV_PIX_FMT_YUYV422, 1);
//    if (ret < 0) {
//        printf("av_image_alloc src failed!\n");
//        return -1;
//    }

    printf("the src linesize: %d, %d, %d, %d\n", src_linse[0], src_linse[1], src_linse[2], src_linse[3]);

//    printf("src image alloc size: %d\n", ret);

    int dst_linse[4] = {480, 0, 0, 0};
//    av_image_fill_linesizes(dst_linse, AVPixelFormat::AV_PIX_FMT_RGB24, 160);
//    ret = av_image_alloc(dst_data, dst_linse, 160, 120, AVPixelFormat::AV_PIX_FMT_RGB24, 1);
//    if (ret < 0) {
//        printf("av_image_alloc dst failed!\n");
//        return -1;
//    }

//    printf("dst image alloc size: %d\n", ret);

    printf("the dst linesize: %d, %d, %d, %d\n", dst_linse[0], dst_linse[1], dst_linse[2], dst_linse[3]);

    int src_stride[4] = {2, 0, 0, 0};
    int dst_stride[4] = {3, 0, 0, 0};
//    ret = sws_scale(context, (const unsigned char * const*)src_data, src_linse, 0, 120, (unsigned char * const *)dst_data, dst_linse);
    int ret = sws_scale(context, src_data, src_linse, 0, 120, dst_data, dst_linse);
    if (ret != 120) {
        printf("sws_scale failed! -- ret is: %d\n", ret);
        return -1;
    }


    printf("sws_scale success!, the ret: %d\n", ret);

    sws_freeContext(context);

    return 0;
}

#ifndef FORMATCONVERTER_H
#define FORMATCONVERTER_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

extern "C" {

#include <libavutil/imgutils.h>
#include <libavutil/parseutils.h>
#include <libswscale/swscale.h>

}

class FormatConverter
{
public:
    FormatConverter();

    int convert(unsigned char *src_data[], int *src_linesize, unsigned char *dst_data[], int *dst_linesize);
};

#endif // FORMATCONVERTER_H

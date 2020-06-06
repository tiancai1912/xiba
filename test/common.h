#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <linux/videodev2.h>

#define CLEAR(x) memset(&(x), 0, sizeof(x))

enum io_method {
    IO_METHOD_READ,
    IO_METHOD_MMAP,
    IO_METHOD_USERPTR,
};

struct buffer {
    void *start;
    size_t length;
};

char            *dev_name;
enum io_method   io = IO_METHOD_MMAP;
int              fd;
buffer          *buffers;
unsigned int     n_buffers;
int              out_buf;
int              force_format;
int              frame_count = 4;


#endif // COMMON_H

#include <QCoreApplication>

#include "common.h"

static void usage(FILE *fp, int argc, char **argv)
{
        fprintf(fp,
                 "Usage: %s [options]\n\n"
                 "Version 1.3\n"
                 "Options:\n"
                 "-d | --device name   Video device name [%s]\n"
                 "-h | --help          Print this message\n"
                 "-m | --mmap          Use memory mapped buffers [default]\n"
                 "-r | --read          Use read() calls\n"
                 "-u | --userp         Use application allocated buffers\n"
                 "-o | --output        Outputs stream to stdout\n"
                 "-f | --format        Force format to 640x480 YUYV\n"
                 "-c | --count         Number of frames to grab [%i]\n"
                 "",
                 argv[0], dev_name, frame_count);
}

static const char short_options[] = "d:hmruofc:";

static const struct option
long_options[] = {
        { "device", required_argument, NULL, 'd' },
        { "help",   no_argument,       NULL, 'h' },
        { "mmap",   no_argument,       NULL, 'm' },
        { "read",   no_argument,       NULL, 'r' },
        { "userp",  no_argument,       NULL, 'u' },
        { "output", no_argument,       NULL, 'o' },
        { "format", no_argument,       NULL, 'f' },
        { "count",  required_argument, NULL, 'c' },
        { 0, 0, 0, 0 }
};

void errno_exit(const char *s)
{
    fprintf(stderr, "$s error %d, %s\n", s, errno, strerror(errno));
    exit(EXIT_FAILURE);
}

int xioctl(int fh, int request, void *arg)
{
    int r;
    do {
        r = ioctl(fh, request, arg);
//        printf("the r : %d", r);
    } while(-1 == r && EINTR == errno);

    return r;
}

void processImage(const void *p, int size)
{
    if (out_buf) {
        fwrite(p, size, 1, stdout);
    }

    fflush(stderr);
    fprintf(stderr, ".");
    fflush(stdout);
}

void storeImage(const char *buf_start, int size, int index)
{
    char path[20];

    snprintf(path, sizeof(path), "./capture%d.yuv", index);
    int fd = open(path, O_WRONLY|O_CREAT, 00700);
    if (-1 == fd) {
        fprintf(stderr, "Cannot open '%s': %d, %s\n",
                path, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    write(fd, buf_start, size);
    close(fd);
}

int readFrame(void)
{
    struct v4l2_buffer buf;
    unsigned int i;

    switch (io) {
    case IO_METHOD_READ:
        if (-1 == read(fd, buffers[0].start, buffers[0].length)) {
            switch (errno) {
            case EAGAIN:
                return 0;
            case EIO:
            default:
                errno_exit("read");
            }
        }

        processImage(buffers[0].start, buffers[0].length);
        break;
    case IO_METHOD_MMAP:
        CLEAR(buf);

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf)) {
            switch (errno) {
            case EAGAIN:
                return 0;
            case EIO:
            default:
                errno_exit("VIDIOC_DQBUF");
            }
        }

        assert(buf.index < n_buffers);

        processImage(buffers[buf.index].start, buf.bytesused);
        storeImage((char *)(buffers[buf.index].start), buf.bytesused, buf.index);

        if (-1 == xioctl(fd, VIDIOC_QBUF, &buf)) {
            errno_exit("VIDIOC_QBUF");
        }

        break;

        break;
    case IO_METHOD_USERPTR:
        CLEAR(buf);

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_USERPTR;

        if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf)) {
            switch (io) {
            case EAGAIN:
                return 0;
            case EIO:
            default:
                errno_exit("VIDIOC_DQBUF");
            }
        }

        for (i = 0; i < n_buffers; ++i) {
            if (buf.m.userptr == (unsigned long)buffers[i].start && buf.length == buffers[i].length)
                break;
        }

        assert(i < n_buffers);

        processImage((void *)buf.m.userptr, buf.bytesused);

        if (-1 == xioctl(fd, VIDIOC_QBUF, &buf)) {
            errno_exit("VIDIOC_QBUF");
        }
        break;
    default:
        break;
    }

    return 1;
}

void mainloop(void)
{
    unsigned int count;
    count = frame_count;

    while(count-- > 0) {
//        printf("the count is: %d\n", count);
        for(int i = 0; i < 1; i++) {
//        while (1){
            fd_set fds;
            struct timeval tv;
            int r;

            FD_ZERO(&fds);
            FD_SET(fd, &fds);

            tv.tv_sec = 20;
            tv.tv_usec = 0;

            r = select(fd + 1, &fds, NULL, NULL, &tv);

            if (-1 == r) {
                if (EINTR == errno)
                    continue;

                errno_exit("select");
            }

            if (0 == r) {
                fprintf(stderr, "select timeout\n");
                exit(EXIT_FAILURE);
            }

            if (readFrame())
                break;
        }
    }
}

void stopCaptuing(void)
{
    enum v4l2_buf_type type;
    switch (io) {
    case IO_METHOD_READ:
        break;
    case IO_METHOD_MMAP:
    case IO_METHOD_USERPTR:
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (-1 == xioctl(fd, VIDIOC_STREAMOFF, &type)) {
            errno_exit("VIDIOC_STREAMOFF");
        }
        break;
    default:
        break;
    }
}

void startCaptuing(void)
{
    unsigned int i;
    enum v4l2_buf_type type;

    switch (io) {
    case IO_METHOD_READ:
        break;
    case IO_METHOD_MMAP:
        for (i = 0; i < n_buffers; ++i) {
            struct v4l2_buffer buf;

            CLEAR(buf);

            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;
            buf.index = i;

            if (-1 == xioctl(fd, VIDIOC_QBUF, &buf)) {
                errno_exit("VIDIOC_QBUF");
            }
        }

        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (-1 == xioctl(fd, VIDIOC_STREAMON, &type)) {
            errno_exit("VIDIOC_STREAMON");
        }
        break;
    case IO_METHOD_USERPTR:
        for (i = 0; i < n_buffers; ++i) {
            struct v4l2_buffer buf;

            CLEAR(buf);
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_USERPTR;
            buf.index = i;
            buf.m.userptr = (unsigned long)buffers[i].start;
            buf.length = buffers[i].length;

            if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
                errno_exit("VIDIOC_QBUF");
        }

        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (-1 == xioctl(fd, VIDIOC_STREAMON, &type)) {
            errno_exit("VIDIOC_STREAMON");
        }
        break;
    default:
        break;
    }
}

void unInitDevice(void)
{
    unsigned int i;

    switch (io) {
    case IO_METHOD_READ:
        free(buffers[0].start);
        break;
    case IO_METHOD_MMAP:
        for (i = 0; i < n_buffers; ++i) {
            if (-1 == munmap(buffers[i].start, buffers[i].length))
                errno_exit("munmap");
        }
        break;
    case IO_METHOD_USERPTR:
        for (i = 0; i < n_buffers; ++i) {
            free(buffers[i].start);
        }
        break;
    default:
        break;
    }

    free(buffers);
}

void initRead(unsigned int buffer_size)
{
    buffers = (buffer *)calloc(1, sizeof(*buffers));
    if (!buffers) {
        fprintf(stderr, "out of memory\n");
        exit(EXIT_FAILURE);
    }

    buffers[0].length = buffer_size;
    buffers[0].start = malloc(buffer_size);

    if (!buffers[0].start) {
        fprintf(stderr, "out of memory\n");
        exit(EXIT_FAILURE);
    }
}

void initMmap(void)
{
    struct v4l2_requestbuffers req;

    CLEAR(req);

    req.count = 4;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) {
        if (EINVAL == errno) {
            fprintf(stderr, "%s does not support"
                            "memory mapping\n", dev_name);
            exit(EXIT_FAILURE);
        } else {
            errno_exit("VIDIOC_REQBUFS");
        }
    }

    if (req.count < 2) {
        fprintf(stderr, "Insufficient buffer memory on %s\n", dev_name);
        exit(EXIT_FAILURE);
    }

    buffers = (buffer *)calloc(req.count, sizeof(*buffers));

    if (!buffers) {
        fprintf(stderr, "out of memory\n");
        exit(EXIT_FAILURE);
    }

    for(n_buffers = 0; n_buffers < req.count; ++n_buffers) {
        struct v4l2_buffer buf;

        CLEAR(buf);

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = n_buffers;

        if (-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf)) {
            errno_exit("VIDIOC_QUERYBUF");
        }

        buffers[n_buffers].length = buf.length;
        printf("buffers[%d].length=%d\n", n_buffers, buffers[n_buffers].length);
        buffers[n_buffers].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);

        if (MAP_FAILED == buffers[n_buffers].start) {
            errno_exit("mmap");
        }
    }
}

void initUserp(unsigned int buffer_size)
{
    struct v4l2_requestbuffers req;

    CLEAR(req);

    req.count = 4;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_USERPTR;

    if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) {
        if (EINVAL == errno) {
            fprintf(stderr, "%s does not support"
                            "user pointer i/o\n", dev_name);
            exit(EXIT_FAILURE);
        } else {
            errno_exit("VIDIOC_REQBUFS");
        }
    }

    buffers = (buffer *)calloc(1, sizeof(*buffers));

    if (!buffers) {
        fprintf(stderr, "out of memory\n");
        exit(EXIT_FAILURE);
    }

    for(n_buffers = 0; n_buffers < 4; ++n_buffers) {
        buffers[n_buffers].length = buffer_size;
        buffers[n_buffers].start = malloc(buffer_size);

        if (!buffers[n_buffers].start) {
            fprintf(stderr, "out of memory\n");
            exit(EXIT_FAILURE);
        }
    }
}

void initDevice(void)
{
    v4l2_capability cap;
    v4l2_cropcap cropcap;
    v4l2_crop crop;
    v4l2_format fmt;
    unsigned int min;

    if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &cap)) {
        if (EINVAL == errno) {
            fprintf(stderr, "%s is no v4l2 device\n", dev_name);
            exit(EXIT_FAILURE);
        } else {
            errno_exit("VIDEO_QUERYCAP");
        }
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        fprintf(stderr, "%s is no video capture device\n", dev_name);
        exit(EXIT_FAILURE);
    }

    switch (io) {
    case IO_METHOD_READ:
        if (!(cap.capabilities & V4L2_CAP_READWRITE)) {
            fprintf(stderr, "%s does not support read i/o\n", dev_name);
            exit(EXIT_FAILURE);
        }
        break;
    case IO_METHOD_MMAP:
    case IO_METHOD_USERPTR:
        if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
            fprintf(stderr, "%s does not support streaming i/o\n", dev_name);
            exit(EXIT_FAILURE);
        }
        break;
    default:
        break;
    }

//    CLEAR(cropcap);

//    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
//    if (0 == xioctl(fd, VIDIOC_CROPCAP, &cropcap)) {
//        crop.type == V4L2_BUF_TYPE_VIDEO_CAPTURE;
//        crop.c = cropcap.defrect;

//        if (-1 == xioctl(fd, VIDIOC_S_CROP, &crop)) {
//            switch (errno) {
//            case EINVAL:
//                printf("1111");
//                /* Cropping not supported. */
//                break;
//            default:
//                /* Errors ignored. */
//                printf("22222, the error is : %d", errno);
//                break;
//            }
//        }
//    } else {
//        printf("3333");
//    }

    CLEAR(fmt);
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
//    if (force_format) {
        fmt.fmt.pix.width = 640;
        fmt.fmt.pix.height = 480;
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
        fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;

        printf("set %dx%d YUYV format\n", fmt.fmt.pix.width, fmt.fmt.pix.height);
        if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt)) {
            errno_exit("VIDIOC_S_FMT");
        }
//    } else {
//        if (-1 == xioctl(fd, VIDIOC_G_FMT, &fmt))
//            errno_exit("VIDIOC_G_FMT");

//        printf("get %dx%d YUYV format %c%c%c%c\n", fmt.fmt.pix.width, fmt.fmt.pix.height, fmt.fmt.pix.pixelformat & 0xff, fmt.fmt.pix.pixelformat >> 8 & 0xff, fmt.fmt.pix.pixelformat >> 16 & 0xff, fmt.fmt.pix.pixelformat >> 24 & 0xff);

//        struct v4l2_fmtdesc fmtdesc;
//        fmtdesc.index=0;
//        fmtdesc.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
//        while(ioctl(fd,VIDIOC_ENUM_FMT,&fmtdesc)!=-1) {
//            if(fmtdesc.pixelformat & fmt.fmt.pix.pixelformat) {
//                printf("format:%s\n", fmtdesc.description);
////                break;
//            }

//            fmtdesc.index++;
//        }

//    }

    min = fmt.fmt.pix.width * 2;
    if (fmt.fmt.pix.bytesperline < min) {
        fmt.fmt.pix.bytesperline = min;
    }

    min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
    if (fmt.fmt.pix.sizeimage < min) {
        fmt.fmt.pix.sizeimage = min;
    }

    switch (io) {
    case IO_METHOD_READ:
        initRead(fmt.fmt.pix.sizeimage);
        break;
    case IO_METHOD_MMAP:
        initMmap();
        break;
    case IO_METHOD_USERPTR:
        initUserp(fmt.fmt.pix.sizeimage);
        break;
    default:
        break;
    }

}
void closeDevice(void)
{
    if (-1 == close(fd))
        errno_exit("close");

    fd = -1;
}
void openDevice(void)
{
    struct stat st;
    if (-1 == stat(dev_name, &st)) {
        fprintf(stderr, "can not identify '%s': %d, %s\n",
                dev_name, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (!S_ISCHR(st.st_mode)) {
        fprintf(stderr, "%s is no device\n", dev_name);
        exit(EXIT_FAILURE);
    }

//    fd = open(dev_name, O_RDWR | O_NONBLOCK, 0);
    fd = open("/dev/video0", O_RDWR);
    if (-1 == fd) {
        fprintf(stderr, "can not open '%s': %d, %s\n",
                dev_name, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[])
{
//    QCoreApplication a(argc, argv);

    dev_name = "/dev/video0";

    for (;;) {
        int idx;
        int c;

        c = getopt_long(argc, argv,
                        short_options, long_options, &idx);

        if (-1 == c)
            break;

        switch (c) {
        case 0: /* getopt_long() flag */
            break;

        case 'd':
            dev_name = optarg;
            break;

        case 'h':
            usage(stdout, argc, argv);
            exit(EXIT_SUCCESS);

        case 'm':
            io = IO_METHOD_MMAP;
            break;

        case 'r':
            io = IO_METHOD_READ;
            break;

        case 'u':
            io = IO_METHOD_USERPTR;
            break;

        case 'o':
            out_buf++;
            break;

        case 'f':
            force_format++;
            break;

        case 'c':
            errno = 0;
            frame_count = strtol(optarg, NULL, 0);
            if (errno)
                errno_exit(optarg);
            break;

        default:
            usage(stderr, argc, argv);
            exit(EXIT_FAILURE);
        }
    }

    openDevice();
    initDevice();

    startCaptuing();
    mainloop();
    stopCaptuing();
    unInitDevice();
    closeDevice();
    fprintf(stderr, "\n");
    return 0;
//    return a.exec();
}

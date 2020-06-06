#include "v4l2capture.h"
//#include "common.h"

v4l2Capture::v4l2Capture()
{

}

//void v4l2Capture::errno_exit(const char *s)
//{
//    fprintf(stderr, "$s error %d, %s\n", s, errno, strerror(errno));
//    exit(EXIT_FAILURE);
//}

//int v4l2Capture::xioctl(int fh, int request, void *arg)
//{
//    int r;
//    do {
//        r = ioctl(fh, request, arg);
//    } while(-1 == r && EINTR == errno);

//    return r;
//}

//void v4l2Capture::processImage(const void *p, int size)
//{
//    if (out_buf) {
//        fwrite(p, size, 1, stdout);
//    }

//    fflush(stderr);
//    fprintf(stderr, ".");
//    fflush(stdout);
//}

//void v4l2Capture::storeImage(const char *buf_start, int size, int index)
//{
//    char path[20];

//    snprintf(path, sizeof(path), ".yuyv%d.yuv", index);
//    int fd = open(path, O_WRONLY|O_CREAT, 00700);
//    if (-1 == fd) {
//        fprintf(stderr, "Cannot open '%s': %d, %s\n",
//                path, errno, strerror(errno));
//        exit(EXIT_FAILURE);
//    }

//    write(fd, buf_start, size);
//    close(fd);
//}

//int v4l2Capture::readFrame(void)
//{
//    struct v4l2_buffer buf;
//    unsigned int i;

//    switch (io) {
//    case IO_METHOD_READ:
//        if (-1 == read(fd, buffers[0].start, buffers[0].length)) {
//            switch (errno) {
//            case EAGAIN:
//                return 0;
//            case EIO:
//            default:
//                errno_exit("read");
//            }
//        }

//        processImage(buffers[0].start, buffers[0].length);
//        break;
//    case IO_METHOD_MMAP:
//        CLEAR(buf);

//        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
//        buf.memory = V4L2_MEMORY_MMAP;
//        if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf)) {
//            switch (errno) {
//            case EAGAIN:
//                return 0;
//            case EIO:
//            default:
//                errno_exit("VIDIOC_DQBUF");
//            }
//        }

//        assert(buf.index < n_buffers);

//        processImage(buffers[buf.index].start, buf.bytesused);
//        storeImage((char *)(buffers[buf.index].start), buf.bytesused, buf.index);

//        if (-1 == xioctl(fd, VIDIOC_QBUF, &buf)) {
//            errno_exit("VIDIOC_QBUF");
//        }

//        break;

//        break;
//    case IO_METHOD_USERPTR:
//        CLEAR(buf);

//        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
//        buf.memory = V4L2_MEMORY_USERPTR;

//        if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf)) {
//            switch (io) {
//            case EAGAIN:
//                return 0;
//            case EIO:
//            default:
//                errno_exit("VIDIOC_DQBUF");
//            }
//        }

//        for (i = 0; i < n_buffers; ++i) {
//            if (buf.m.userptr == (unsigned long)buffers[i].start && buf.length == buffers[i].length)
//                break;
//        }

//        assert(i < n_buffers);

//        processImage((void *)buf.m.userptr, buf.bytesused);

//        if (-1 == xioctl(fd, VIDIOC_QBUF, &buf)) {
//            errno_exit("VIDIOC_QBUF");
//        }
//        break;
//    default:
//        break;
//    }

//    return 1;
//}

//void v4l2Capture::mainloop(void)
//{
//    unsigned int count;
//    count = frame_count;

//    while(count-- > 0) {
//        for(;;) {
//            fd_set fds;
//            struct timeval tv;
//            int r;

//            FD_ZERO(&fds);
//            FD_SET(fd, &fds);

//            tv.tv_sec = 2;
//            tv.tv_usec = 0;

//            r = select(fd + 1, &fds, NULL, NULL, &tv);

//            if (-1 == r) {
//                if (EINTR == errno)
//                    continue;

//                errno_exit("select");
//            }

//            if (0 == r) {
//                fprintf(stderr, "select timeout\n");
//                exit(EXIT_FAILURE);
//            }

//            if (readFrame())
//                break;
//        }
//    }
//}

//void v4l2Capture::stopCaptuing(void)
//{
//    enum v4l2_buf_type type;
//    switch (io) {
//    case IO_METHOD_READ:
//        break;
//    case IO_METHOD_MMAP:
//    case IO_METHOD_USERPTR:
//        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
//        if (-1 == xioctl(fd, VIDIOC_STREAMOFF, &type)) {
//            errno_exit("VIDIOC_STREAMOFF");
//        }
//        break;
//    default:
//        break;
//    }
//}

//void v4l2Capture::startCaptuing(void)
//{
//    unsigned int i;
//    enum v4l2_buf_type type;

//    switch (io) {
//    case IO_METHOD_READ:
//        break;
//    case IO_METHOD_MMAP:
//        for (i = 0; i < n_buffers; ++i) {
//            struct v4l2_buffer buf;

//            CLEAR(buf);

//            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
//            buf.memory = V4L2_MEMORY_MMAP;
//            buf.index = i;

//            if (-1 == xioctl(fd, VIDIOC_QBUF, &buf)) {
//                errno_exit("VIDIOC_QBUF");
//            }
//        }

//        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
//        if (-1 == xioctl(fd, VIDIOC_STREAMON, &type)) {
//            errno_exit("VIDIOC_STREAMON");
//        }
//        break;
//    case IO_METHOD_USERPTR:
//        for (i = 0; i < n_buffers; ++i) {
//            struct v4l2_buffer buf;

//            CLEAR(buf);
//            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
//            buf.memory = V4L2_MEMORY_USERPTR;
//            buf.index = i;
//            buf.m.userptr = (unsigned long)buffers[i].start;
//            buf.length = buffers[i].length;

//            if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
//                errno_exit("VIDIOC_QBUF");
//        }

//        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
//        if (-1 == xioctl(fd, VIDIOC_STREAMON, &type)) {
//            errno_exit("VIDIOC_STREAMON");
//        }
//        break;
//    default:
//        break;
//    }
//}

//void v4l2Capture::unInitDevice(void)
//{
//    unsigned int i;

//    switch (io) {
//    case IO_METHOD_READ:
//        free(buffers[0].start);
//        break;
//    case IO_METHOD_MMAP:
//        for (i = 0; i < n_buffers; ++i) {
//            if (-1 == munmap(buffers[i].start, buffers[i].length))
//                errno_exit("munmap");
//        }
//        break;
//    case IO_METHOD_USERPTR:
//        for (i = 0; i < n_buffers; ++i) {
//            free(buffers[i].start);
//        }
//        break;
//    default:
//        break;
//    }

//    free(buffers);
//}

//void v4l2Capture::initRead(unsigned int buffer_size)
//{
//    buffers = (buffer *)calloc(1, sizeof(*buffers));
//    if (!buffers) {
//        fprintf(stderr, "out of memory\n");
//        exit(EXIT_FAILURE);
//    }

//    buffers[0].length = buffer_size;
//    buffers[0].start = malloc(buffer_size);

//    if (!buffers[0].start) {
//        fprintf(stderr, "out of memory\n");
//        exit(EXIT_FAILURE);
//    }
//}

//void v4l2Capture::initMmap(void)
//{
//    struct v4l2_requestbuffers req;

//    CLEAR(req);

//    req.count = 4;
//    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
//    req.memory = V4L2_MEMORY_MMAP;

//    if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) {
//        if (EINVAL == errno) {
//            fprintf(stderr, "%s does not support"
//                            "memory mapping\n", dev_name);
//            exit(EXIT_FAILURE);
//        } else {
//            errno_exit("VIDIOC_REQBUFS");
//        }
//    }

//    if (req.count < 2) {
//        fprintf(stderr, "Insufficient buffer memory on %s\n", dev_name);
//        exit(EXIT_FAILURE);
//    }

//    buffers = (buffer *)calloc(req.count, sizeof(*buffers));

//    if (!buffers) {
//        fprintf(stderr, "out of memory\n");
//        exit(EXIT_FAILURE);
//    }

//    for(n_buffers = 0; n_buffers < req.count; ++n_buffers) {
//        struct v4l2_buffer buf;

//        CLEAR(buf);

//        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
//        buf.memory = V4L2_MEMORY_MMAP;
//        buf.index = n_buffers;

//        if (-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf)) {
//            errno_exit("VIDIOC_QUERYBUF");
//        }

//        buffers[n_buffers].length = buf.length;
//        printf("buffers[%s].length=%d\n", n_buffers, buffers[n_buffers].length);
//        buffers[n_buffers].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);

//        if (MAP_FAILED == buffers[n_buffers].start) {
//            errno_exit("mmap");
//        }
//    }
//}

//void v4l2Capture::initUserp(unsigned int buffer_size)
//{
//    struct v4l2_requestbuffers req;

//    CLEAR(req);

//    req.count = 4;
//    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
//    req.memory = V4L2_MEMORY_USERPTR;

//    if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) {
//        if (EINVAL == errno) {
//            fprintf(stderr, "%s does not support"
//                            "user pointer i/o\n", dev_name);
//            exit(EXIT_FAILURE);
//        } else {
//            errno_exit("VIDIOC_REQBUFS");
//        }
//    }

//    buffers = (buffer *)calloc(1, sizeof(*buffers));

//    if (!buffers) {
//        fprintf(stderr, "out of memory\n");
//        exit(EXIT_FAILURE);
//    }

//    for(n_buffers = 0; n_buffers < 4; ++n_buffers) {
//        buffers[n_buffers].length = buffer_size;
//        buffers[n_buffers].start = malloc(buffer_size);

//        if (!buffers[n_buffers].start) {
//            fprintf(stderr, "out of memory\n");
//            exit(EXIT_FAILURE);
//        }
//    }
//}

//void v4l2Capture::initDevice(void)
//{
//    v4l2_capability cap;
//    v4l2_cropcap cropcap;
//    v4l2_crop crop;
//    v4l2_format fmt;
//    unsigned int min;

//    if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &cap)) {
//        if (EINVAL == errno) {
//            fprintf(stderr, "%s is no v4l2 device\n", dev_name);
//            exit(EXIT_FAILURE);
//        } else {
//            errno_exit("VIDEO_QUERYCAP");
//        }
//    }

//    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
//        fprintf(stderr, "%s is no video capture device\n", dev_name);
//        exit(EXIT_FAILURE);
//    }

//    switch (io) {
//    case IO_METHOD_READ:
//        if (!(cap.capabilities & V4L2_CAP_READWRITE)) {
//            fprintf(stderr, "%s does not support read i/o\n", dev_name);
//            exit(EXIT_FAILURE);
//        }
//        break;
//    case IO_METHOD_MMAP:
//    case IO_METHOD_USERPTR:
//        if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
//            fprintf(stderr, "%s does not support streaming i/o\n", dev_name);
//            exit(EXIT_FAILURE);
//        }
//        break;
//    default:
//        break;
//    }

//    CLEAR(cropcap);

//    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
//    if (0 == xioctl(fd, VIDIOC_CROPCAP, &cropcap)) {
//        crop.type == V4L2_BUF_TYPE_VIDEO_CAPTURE;
//        crop.c = cropcap.defrect;

//        if (-1 == xioctl(fd, VIDIOC_S_CROP, &crop)) {
//            switch (errno) {
//            case EINVAL:
//                /* Cropping not supported. */
//                break;
//            default:
//                /* Errors ignored. */
//                break;
//            }
//        }
//    } else {

//    }

//    CLEAR(fmt);
//    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
//    if (force_format) {
//        fmt.fmt.pix.width = 640;
//        fmt.fmt.pix.height = 480;
//        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
//        fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;

//        printf("set %dx%d YUYV format\n", fmt.fmt.pix.width, fmt.fmt.pix.height);
//        if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt)) {
//            errno_exit("VIDIOC_S_FMT");
//        }
//    } else {
//        if (-1 == xioctl(fd, VIDIOC_G_FMT, &fmt))
//            errno_exit("VIDIOC_G_FMT");
//    }

//    min = fmt.fmt.pix.width * 2;
//    if (fmt.fmt.pix.bytesperline < min) {
//        fmt.fmt.pix.bytesperline = min;
//    }

//    min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
//    if (fmt.fmt.pix.sizeimage < min) {
//        fmt.fmt.pix.sizeimage = min;
//    }

//    switch (io) {
//    case IO_METHOD_READ:
//        initRead(fmt.fmt.pix.sizeimage);
//        break;
//    case IO_METHOD_MMAP:
//        initMmap();
//        break;
//    case IO_METHOD_USERPTR:
//        initUserp(fmt.fmt.pix.sizeimage);
//        break;
//    default:
//        break;
//    }

//}
//void v4l2Capture::closeDevice(void)
//{
//    if (-1 == close(fd))
//        errno_exit("close");

//    fd = -1;
//}
//void v4l2Capture::openDevice(void)
//{
//    struct stat st;
//    if (-1 == stat(dev_name, &st)) {
//        fprintf(stderr, "can not identify '%s': %d, %s\n",
//                dev_name, errno, strerror(errno));
//        exit(EXIT_FAILURE);
//    }

//    if (!S_ISCHR(st.st_mode)) {
//        fprintf(stderr, "%s is no device\n", dev_name);
//        exit(EXIT_FAILURE);
//    }

//    fd = open(dev_name, O_RDWR | O_NONBLOCK, 0);
//    if (-1 == fd) {
//        fprintf(stderr, "can not open '%s': %d, %s\n",
//                dev_name, errno, strerror(errno));
//        exit(EXIT_FAILURE);
//    }
//}

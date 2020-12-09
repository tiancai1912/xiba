#include "myv4l2.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <poll.h>

#define CAPTURE_WIDTH 160
#define CAPTURE_HEIGHT 120

static int get_pixel_depth(unsigned int fmt)
{
    int depth = 0;
    switch (fmt) {
    case V4L2_PIX_FMT_NV12:
    case V4L2_PIX_FMT_NV21:
    case V4L2_PIX_FMT_YUV420:
    case V4L2_PIX_FMT_YVU420:
        depth = 12;
        break;
    case V4L2_PIX_FMT_RGB565:
    case V4L2_PIX_FMT_YUYV:
    case V4L2_PIX_FMT_YVYU:
    case V4L2_PIX_FMT_UYVY:
    case V4L2_PIX_FMT_VYUY:
    case V4L2_PIX_FMT_NV16:
    case V4L2_PIX_FMT_NV61:
    case V4L2_PIX_FMT_YUV422P:
        depth = 16;
        break;
    case V4L2_PIX_FMT_RGB32:
        depth = 32;
        break;
    }

    return depth;
}


MyV4l2::MyV4l2() :
    m_exit(false)
{

}

int MyV4l2::setHue(int value)
{
    int ret = v4l2_s_ctrl(m_fd, V4L2_CID_HUE, 10);
    if (ret < 0) {
        printf("set hue[%d] failed!", value);
        return -1;
    }

    printf("set hue[%d] success!\n", value);
    return 0;
}

int MyV4l2::setBrightness(int value)
{
    int ret = v4l2_s_ctrl(m_fd, V4L2_CID_BRIGHTNESS, 10);
    if (ret < 0) {
        printf("set brightness[%d] failed!", value);
        return -1;
    }

    printf("set brightness[%d] success!\n", value);
    return 0;
}

int MyV4l2::setSaturation(int value)
{
    int ret = v4l2_s_ctrl(m_fd, V4L2_CID_SATURATION, 10);
    if (ret < 0) {
        printf("set saturation[%d] failed!", value);
        return -1;
    }

    printf("set saturation[%d] success!\n", value);
    return 0;
}

int MyV4l2::setContrast(int value)
{
    int ret = v4l2_s_ctrl(m_fd, V4L2_CID_CONTRAST, 10);
    if (ret < 0) {
        printf("set contrast[%d] failed!", value);
        return -1;
    }

    printf("set contrast[%d] success!\n", value);
    return 0;
}

int MyV4l2::openDevice(std::string device)
{
    m_fd = v4l2_open("/dev/video0", O_RDWR | O_NONBLOCK);
    if (m_fd < 0) {
        printf("open device[%s] failed!\n", device.c_str());
    }

    struct v4l2_capability cap;
    int ret = v4l2_querycap(m_fd, &cap);
    if (ret < 0) {
        printf("query cap failed!\n");
    }

    printf("card: %s", cap.card);
    printf("driver: %s", cap.driver);
    printf("version: %d", cap.version);
    printf("device caps: %d", cap.device_caps);
    printf("support video capture: %d", (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE));
    printf("support video mmap: %d", (cap.capabilities & V4L2_CAP_STREAMING));

    int width = CAPTURE_WIDTH;
    int height = CAPTURE_HEIGHT;

    ret = v4l2_s_fmt(m_fd, &width, &height, V4L2_PIX_FMT_YUYV, V4L2_BUF_TYPE_VIDEO_CAPTURE);
    if (ret < 0) {
        printf("set fmt failed!\n");
        return -1;
    }

    //init bufs
    m_bufs = v4l2_reqbufs(m_fd, V4L2_BUF_TYPE_VIDEO_CAPTURE, 3);
    if (m_bufs == NULL) {
        printf("req bufs failed!\n");
        return -1;
    }

    ret = v4l2_querybuf(m_fd, m_bufs);
    if (ret < 0) {
        printf("query buf failed!\n");
        return -1;
    }

    ret = v4l2_mmap(m_fd, m_bufs);
    if (ret < 0) {
        printf("mmap buf failed!\n");
        return -1;
    }

    ret = v4l2_qbuf_all(m_fd, m_bufs);
    if (ret < 0) {
        printf("qbuf failed!\n");
        return -1;
    }

    return 0;
}

void MyV4l2::closeDevice()
{
    int ret = v4l2_munmap(m_fd, m_bufs);
    if (ret < 0) {
        printf("v4l2 munmap failed!\n");
        return;
    }

    ret = v4l2_close(m_fd);
    if (ret < 0) {
        printf("close device failed!\n");
        return;
    }
}

int MyV4l2::startCaptureLoop()
{
    int ret = v4l2_streamon(m_fd);
    if (ret < 0) {
        printf("v4l2 stream on failed!\n");
        return -1;
    }

    m_thread = new MyThread(this);
    m_thread->start();

    return 0;
}

int MyV4l2::stopCaptureLoop()
{
    m_exit = true;
    m_thread->stop();

    int ret = v4l2_streamoff(m_fd);
    if (ret < 0) {
        printf("stream off failed!\n");
        return -1;
    }

    return 0;
}

RawVideoData * MyV4l2::getFrame()
{
    m_mutex.lock();
    RawVideoData *temp = m_queue.front();
    m_queue.pop();
    m_mutex.unlock();

    return temp;
}

////////////////////////////////////

int MyV4l2::v4l2_open(const char *name, int flag)
{
    int fd = open(name, flag);
    if (fd < 0) {
        printf("ERR(%s): failed to open %s\n", __func__, name);
        return -1;
    }

    m_fd = fd;
    return fd;
}

int MyV4l2::v4l2_close(int fd)
{
    if (close(fd)) {
        printf("ERR(%s): failed to close v4l2 dev\n", __func__);
        return -1;
    }

    m_fd = -1;
    return 0;
}

int MyV4l2::v4l2_querycap(int fd, struct v4l2_capability *cap)
{
    if (ioctl(fd, VIDIOC_QUERYCAP, cap) < 0) {
        printf("ERR(%s): VIDIOC_QUERYCAP failed\n", __func__);
        return -1;
    }

    return 0;
}

int MyV4l2::v4l2_enuminput(int fd, int index, char *name)
{
    struct v4l2_input input;
    int found = 0;

    input.index = 0;
    while(!ioctl(fd, VIDIOC_ENUMINPUT, &input)) {
        //printf("input: %s\n", input.name);
        if (input.index == (uint32_t)index) {
            found = 1;
            strcpy(name, (char *)input.name);
        }

        ++input.index;
    }

    if (!found) {
        printf("%s: can't find input dev\n", __func__);
        return -1;
    }

    return 0;
}

int MyV4l2::v4l2_s_input(int fd, int index)
{
    struct v4l2_input input;
    input.index = index;
    if (ioctl(fd, VIDIOC_S_INPUT, &input) < 0) {
        printf("ERR(%s): VIDIOC_S_INPUT failed\n", __func__);
        return -1;
    }

    return 0;
}

int MyV4l2::v4l2_enum_fmt(int fd, unsigned int fmt, enum v4l2_buf_type type)
{
    struct v4l2_fmtdesc fmtdesc;
    int found = 0;
//    fmt_desc.clear();

    fmtdesc.type = type;
    fmtdesc.index = 0;
    while(!ioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc)){
//        printf("fmt desc: %s\n", fmtdesc.description);
//        printf("fmt pix format: %d\n", fmtdesc.pixelformat);
//        fmt_desc += (char *)(fmtdesc.description);
//        fmt_desc += "\n";
        if (fmtdesc.pixelformat == fmt) {
            found = 1;
            break;
        }

        fmtdesc.index++;
    }

    if (!found) {
        printf("%s: unsupported pixel format\n", __func__);
        return -1;
    }

    return 0;
}

int MyV4l2::v4l2_s_fmt(int fd, int *width, int *height, unsigned int fmt, enum v4l2_buf_type type)
{
    struct v4l2_format v4l2_fmt;
    struct v4l2_pix_format pixfmt;

    memset(&v4l2_fmt, 0, sizeof (struct v4l2_format));
    v4l2_fmt.type = type;

    memset(&pixfmt, 0, sizeof(pixfmt));

    pixfmt.width = *width;
    pixfmt.height = *height;
    pixfmt.pixelformat = fmt;
    pixfmt.sizeimage = ((*width) * (*height) * get_pixel_depth(fmt)) / 8;
    pixfmt.field = V4L2_FIELD_INTERLACED;
    v4l2_fmt.fmt.pix = pixfmt;

    if (ioctl(fd, VIDIOC_S_FMT, &v4l2_fmt) < 0) {
        printf("ERR(%s): VIDIOC_S_FMT failed\n", __func__);
        return -1;
    }

    *width = v4l2_fmt.fmt.pix.width;
    *height = v4l2_fmt.fmt.pix.height;

    return 0;
}

struct MyV4l2::v4l2_buf* MyV4l2::v4l2_reqbufs(int fd, enum v4l2_buf_type type, int nr_bufs)
{
    struct v4l2_requestbuffers req;
    struct v4l2_buf *v4l2_buf;

    req.count = nr_bufs;
    req.type = type;
    req.memory = V4L2_MEMORY_MMAP;

    if (ioctl(fd, VIDIOC_REQBUFS, &req) < 0) {
        printf("ERR(%s):VIDIOC_REQBUFS failed\n", __func__);
        return NULL;
    }

    v4l2_buf = (struct v4l2_buf *)malloc(sizeof(struct v4l2_buf));
    v4l2_buf->nr_bufs = req.count;
    v4l2_buf->type = type;
    v4l2_buf->buf = (struct v4l2_buf_unit *)malloc(sizeof(struct v4l2_buf_unit) * v4l2_buf->nr_bufs);

    memset(v4l2_buf->buf, 0, sizeof(struct v4l2_buf_unit) * v4l2_buf->nr_bufs);
    return v4l2_buf;
}

int MyV4l2::v4l2_querybuf(int fd, struct v4l2_buf* v4l2_buf)
{
    struct v4l2_buffer buf;
    struct v4l2_buf_unit *buf_unit;

    for (int i = 0; i < v4l2_buf->nr_bufs; i++) {
        buf.type = v4l2_buf->type;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (ioctl(fd, VIDIOC_QUERYBUF, &buf) < 0) {
            printf("ERR(%s):VIDIOC_QUERYBUF failed\n", __func__);
            return -1;
        }

        buf_unit = &v4l2_buf->buf[i];
        buf_unit->index = i;
        buf_unit->offset = buf.m.offset;
        buf_unit->length = buf.length;
        buf_unit->start = NULL;
    }

    return 0;
}

int MyV4l2::v4l2_mmap(int fd, struct v4l2_buf* v4l2_buf)
{
    int i = 0;
    struct v4l2_buf_unit *buf_unit;

    for (i = 0; i < v4l2_buf->nr_bufs; i++) {
        buf_unit = &v4l2_buf->buf[i];
        buf_unit->start = mmap(0, buf_unit->length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf_unit->offset);
        if (buf_unit->start == NULL) {
            goto err;
        }
    }

    return 0;

err:
    while(--i >= 0) {
        buf_unit = &v4l2_buf->buf[i];
        munmap(buf_unit->start, buf_unit->length);
        buf_unit->start = NULL;
    }

    return -1;
}

int MyV4l2::v4l2_munmap(int fd, struct v4l2_buf* v4l2_buf)
{
    int i = 0;
    struct v4l2_buf_unit *buf_unit;

    for (i = 0; i < v4l2_buf->nr_bufs; i++) {
        buf_unit = &v4l2_buf->buf[i];
        munmap(buf_unit->start, buf_unit->length);
        buf_unit->start = NULL;
    }

    return 0;
}

int MyV4l2::v4l2_relbufs(struct v4l2_buf* v4l2_buf)
{
    int i = 0;

    free(v4l2_buf->buf);
    free(v4l2_buf);

    return 0;
}

int MyV4l2::v4l2_streamon(int fd)
{
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (ioctl(fd, VIDIOC_STREAMON, &type) < 0) {
        printf("ERR(%s):VIDIOC_STREAMON failed\n", __func__);
        return -1;
    }

    if (v4l2_poll(fd) < 0) {
        return -1;
    }

    return 0;
}

int MyV4l2::v4l2_streamoff(int fd)
{
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (ioctl(fd, VIDIOC_STREAMOFF, &type) < 0) {
        printf("ERR(%s):VIDIOC_STREAMOFF failed\n", __func__);
        return -1;
    }

    return 0;
}

int MyV4l2::v4l2_qbuf(int fd, struct v4l2_buf_unit* buf)
{
    struct v4l2_buffer v4l2_buf;
    v4l2_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    v4l2_buf.memory = V4L2_MEMORY_MMAP;
    v4l2_buf.index = buf->index;

    if (ioctl(fd, VIDIOC_QBUF, &v4l2_buf) < 0) {
        printf("ERR(%s):VIDIOC_QBUF failed\n", __func__);
        return -1;
    }

    return 0;
}

int MyV4l2::v4l2_qbuf_all(int fd, struct v4l2_buf* v4l2_buf)
{
    int i;
    for (i = 0; i < v4l2_buf->nr_bufs; i++) {
        if (v4l2_qbuf(fd, &v4l2_buf->buf[i])) {
            return -1;
        }
    }

    return 0;
}

struct MyV4l2::v4l2_buf_unit* MyV4l2::v4l2_dqbuf(int fd, struct v4l2_buf* v4l2_buf)
{
    struct v4l2_buffer buffer;

    buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buffer.memory = V4L2_MEMORY_MMAP;

    if (ioctl(fd, VIDIOC_DQBUF, &buffer) < 0) {
        printf("ERR(%s):VIDIOC_DQBUF failed, dropped frame\n", __func__);
        return NULL;
    }

    return &v4l2_buf->buf[buffer.index];
}

int MyV4l2::v4l2_g_ctrl(int fd, unsigned int id)
{
    struct v4l2_control ctrl;
    ctrl.id = id;

    if (ioctl(fd, VIDIOC_G_CTRL, &ctrl) < 0) {
        printf("ERR(%s):VIDIOC_G_CTRL(id = 0x%x(%d)) failed\n",
               __func__, id, id - V4L2_CID_PRIVATE_BASE);
        return -1;
    }

    return ctrl.value;
}

int MyV4l2::v4l2_s_ctrl(int fd, unsigned int id, unsigned int value)
{
    struct v4l2_control ctrl;
    ctrl.id = id;
    ctrl.value = value;

    if (ioctl(fd, VIDIOC_S_CTRL, &ctrl) < 0) {
        printf("ERR(%s): VIDICO_S_CTRL(id = %#x (%d), value = %d) failed\n",
               __func__, id, id - V4L2_CID_PRIVATE_BASE, value);
        return -1;
    }

    return ctrl.value;
}

int MyV4l2::v4l2_g_parm(int fd, struct v4l2_streamparm* streamparm)
{
    streamparm->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (ioctl(fd, VIDIOC_G_PARM, streamparm) < 0) {
        printf("ERR(%s): VIDIOC_G_PARM failed\n", __func__);
        return -1;
    }

    return 0;
}

int MyV4l2::v4l2_s_parm(int fd, struct v4l2_streamparm *streamparm)
{
    streamparm->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (ioctl(fd, VIDIOC_S_PARM, &streamparm) < 0) {
        printf("ERR(%s): VIDIOC_S_PARM failed\n", __func__);
        return -1;
    }

    return 0;
}

int MyV4l2::v4l2_poll(int fd)
{
    int ret;
    struct pollfd poll_fds[1];
    poll_fds[0].fd = fd;
    poll_fds[0].events = POLLIN;

    ret = poll(poll_fds, 1, 2000);
    if (ret < 0) {
        printf("ERR(%s): poll error\n", __func__);
        return -1;
    }

    if (ret == 0) {
        printf("ERR(%s): No data in 10 sec..\n", __func__);
        return -1;
    }

//    fd_set fds;
//    struct timeval tv;
//    int r;

//    FD_ZERO(&fds);
//    FD_SET(fd, &fds);

//    tv.tv_sec = 20;
//    tv.tv_usec = 0;

//    r = select(fd + 1, &fds, NULL, NULL, &tv);
//    if (-1 == r) {
//        if (EINTR == errno) {
//            printf("select failed!\n");
//            return -1;
//        }
////        errno_exit("select");
//    }

//    if (0 == r) {
//        printf("select timeout");
//        return -1;
////        fprintf(stderr, "select timeout\n");
////        exit(EXIT_FAILURE);
//    }


    return 0;
}

void MyV4l2::run()
{
    int ret = 0;
    while(!m_exit) {
        ret = v4l2_poll(m_fd);
        if (ret < 0) {
            printf("poll failed!\n");
            continue;
        }

        MyV4l2::v4l2_buf_unit *unit = v4l2_dqbuf(m_fd, m_bufs);
        if (unit == NULL) {
            printf("dqbuf failed!\n");
            continue;
        }

        //push queue
        RawVideoData *item = new RawVideoData();
        item->data = (unsigned char *)malloc(unit->length);
        memset(item->data, 0, unit->length);
        memcpy(item->data, unit->start, unit->length);
        m_mutex.lock();
        if (m_queue.size() <= 10) {
            m_queue.push(item);
        } else {
            RawVideoData *temp = m_queue.front();
            delete temp->data;
            temp->data = nullptr;
            m_queue.pop();
        }
        m_mutex.unlock();

        //callback
        if (m_callback != nullptr) {
            RawVideoData *data = new RawVideoData();
            data->data = (unsigned char *)(unit->start);
            data->length = unit->length;
            m_callback->callback(data);
        }

        ret = v4l2_qbuf(m_fd, unit);
        if (ret != 0) {
            printf("v4l2 qbuf failed!");
            continue;
        }
    }
}

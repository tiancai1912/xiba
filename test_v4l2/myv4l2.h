#ifndef MYV4L2_H
#define MYV4L2_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <linux/videodev2.h>
#include "mythread.h"

#include <queue>
#include <mutex>

using namespace std;

struct RawVideoData
{
    int length;
    unsigned char *data;
};


class MyV4l2 : public MyThread::ThreadRun
{
public:
    MyV4l2();

    class CaptureCallback
    {
    public:
        virtual void callback(RawVideoData *data) = 0;
    };

    int setHue(int value);
    int setBrightness(int value);
    int setSaturation(int value);
    int setContrast(int value);

    int openDevice(std::string device);
    void closeDevice();

    int startCaptureLoop();
    int stopCaptureLoop();

    RawVideoData * getFrame();

    void setCaptureCallback(CaptureCallback *cb) { m_callback = cb; }

private:
    struct v4l2_buf_unit {
        int                index;
        void*              start;
        uint32_t           length;
        uint32_t           offset;
    };

    struct v4l2_buf {
        struct v4l2_buf_unit* buf;
        int nr_bufs;
        enum v4l2_buf_type type;
    };

    int v4l2_open(const char *name, int flag);
    int v4l2_close(int fd);
    int v4l2_querycap(int fd, struct v4l2_capability *cap);
    int v4l2_enuminput(int fd, int index, char *name);
    int v4l2_s_input(int fd, int index);
    int v4l2_enum_fmt(int fd, unsigned int fmt, enum v4l2_buf_type type);
    int v4l2_s_fmt(int fd, int *width, int *height, unsigned int fmt, enum v4l2_buf_type type);
    struct v4l2_buf* v4l2_reqbufs(int fd, enum v4l2_buf_type type, int nr_bufs);
    int v4l2_querybuf(int fd, struct v4l2_buf* v4l2_buf);
    int v4l2_mmap(int fd, struct v4l2_buf* v4l2_buf);
    int v4l2_munmap(int fd, struct v4l2_buf* v4l2_buf);
    int v4l2_relbufs(struct v4l2_buf* v4l2_buf);
    int v4l2_streamon(int fd);
    int v4l2_streamoff(int fd);
    int v4l2_qbuf(int fd, struct v4l2_buf_unit* buf);
    int v4l2_qbuf_all(int fd, struct v4l2_buf* v4l2_buf);
    struct v4l2_buf_unit* v4l2_dqbuf(int fd, struct v4l2_buf* v4l2_buf);
    int v4l2_g_ctrl(int fd, unsigned int id);
    int v4l2_s_ctrl(int fd, unsigned int id, unsigned int value);
    int v4l2_g_parm(int fd, struct v4l2_streamparm* streamparm);
    int v4l2_s_parm(int fd, struct v4l2_streamparm *streamparm);
    int v4l2_poll(int fd);

    virtual void run();

private:


private:
    int m_fd;

    MyV4l2::v4l2_buf *m_bufs;

    MyThread *m_thread;
    bool m_exit;
    CaptureCallback *m_callback;

    //mutex
    std::mutex m_mutex;
    std::queue<RawVideoData *> m_queue;

};

#endif // MYV4L2_H

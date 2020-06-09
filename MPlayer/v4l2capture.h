#ifndef V4L2CAPTURE_H
#define V4L2CAPTURE_H

#include <QString>
#include "threadutil.h"

class v4l2Capture : public ThreadUtil::Callback
{
public:
    v4l2Capture();

    bool init(QString dev_name);
    void unInit();

    bool startCapture();
    void stopCapture();

    class Callback {
    public:
        virtual void onFrameCapture(const char *data, int length, int index) = 0;
    };

    void setCallback(Callback *callback) {
        m_callback = callback;
    }


    const char* getOneFrame();
    bool getFrame();

    QString enumDevices(const char *dev_name);
    const char *captureVideoFrame();

protected:
    void run() override;

private:
    bool captureFrame();

protected:
    void errno_exit(const char *s);
    int xioctl(int fh, int request, void *arg);
    void processImage(const void *p, int size);
    QString storeImage(const char *buf_start, int size, int index);
    int readFrame(QString &path);
    void mainloop(void);
    void stopCaptuing(void);
    void startCaptuing(void);
    void unInitDevice(void);
    void initRead(unsigned int buffer_size);
    void initMmap(void);
    void initUserp(unsigned int buffer_size);
    void initDevice(void);
    void closeDevice(void);
    void openDevice(QString dev_name);

private:

    Callback *m_callback;

};

class HandleFrame : public v4l2Capture::Callback
{
public:
    HandleFrame() {}

    virtual void onFrameCapture(const char *data, int length, int index) override;
};

#endif // V4L2CAPTURE_H

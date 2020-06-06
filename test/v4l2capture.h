#ifndef V4L2CAPTURE_H
#define V4L2CAPTURE_H

//static void usage(FILE &fp, int argc, char **argv) {
//    fprintf(fp,
//            "Usage: %s [option]\n\n"
//            "Version 0.1\n"
//            "Option:\n"
//            "-d | --device name"
//            "-h | --help"
//            "-m | --mmap"
//            "-r | --read"
//            "-u | --userp"
//            "-o | --output"
//            "-f | --format"
//            "-c | --count"
//            "",
//            argv[0], dev_name, frame_count);
//}


class v4l2Capture
{
public:
    v4l2Capture();

    bool init();
    void unInit();

    bool startCapture();
    void stopCapture();

    bool getFrame();

//    void errno_exit(const char *s);
//    int xioctl(int fh, int request, void *arg);
//    void processImage(const void *p, int size);
//    void storeImage(const char *buf_start, int size, int index);
//    int readFrame(void);
//    void mainloop(void);
//    void stopCaptuing(void);
//    void startCaptuing(void);
//    void unInitDevice(void);
//    void initRead(unsigned int buffer_size);
//    void initMmap(void);
//    void initUserp(unsigned int buffer_size);
//    void initDevice(void);
//    void closeDevice(void);
//    void openDevice(void);


private:
//    char            *dev_name;
//    enum io_method   io = IO_METHOD_MMAP;
//    int              fd = -1;
//    buffer          *buffers;
//    unsigned int     n_buffers;
//    int              out_buf;
//    int              force_format;
//    int              frame_count = 4;


};

#endif // V4L2CAPTURE_H

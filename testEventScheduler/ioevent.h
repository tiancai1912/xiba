#ifndef IOEVENT_H
#define IOEVENT_H

typedef void (*EventCallback) (void*);

class IOEvent
{
public:

    enum IOEventType
    {
        EVENT_NONE = 0,
        EVENT_READ = 1,
        EVENT_WRITE = 2,
        EVENT_ERROR = 3,
    };

    ~IOEvent();

    static IOEvent * createNew(int fd, void *arg);
    static IOEvent * createNew(int fd);

    int getFd() const { return mFd; }
    int getEvent() const { return mEvent; }
    void setREvent(int event) { mREvent = event; }
    void setArg(void *arg) { mArg = arg;}

    void setReadCallback(EventCallback cb) { mReadCallback = cb; }
    void setWriteCallback(EventCallback cb) { mWriteCallback = cb; }
    void setErrorCallback(EventCallback cb) { mErrorCallback = cb; }

    void enableReadHandling() { mEvent |= EVENT_READ; }
    void enableWriteHandling() { mEvent |= EVENT_WRITE; }
    void enableErrorHandling() { mEvent |= EVENT_ERROR; }
    void disableReadHandling() { mEvent &= ~EVENT_READ; }
    void disableWriteHandling() { mEvent &= ~EVENT_WRITE; }
    void disableErrorHandling() { mEvent &= ~EVENT_ERROR; }

    bool isNoneHandling() const { return (mEvent == EVENT_NONE); }
    bool isReadHandling() const { return (mEvent & EVENT_READ) != 0; }
    bool isWriteHandling() const { return (mEvent & EVENT_WRITE) != 0; }
    bool isErrorHandling() const { return (mEvent & EVENT_ERROR) != 0; }

    void handleEvent();


protected:
    IOEvent(int fd, void *arg);

private:
    int mFd;
    void *mArg;
    int mEvent;
    int mREvent;

    EventCallback mReadCallback;
    EventCallback mWriteCallback;
    EventCallback mErrorCallback;
};

#endif // IOEVENT_H

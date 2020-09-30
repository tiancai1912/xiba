#include "ioevent.h"

IOEvent::IOEvent(int fd, void *arg) :
    mFd(fd),
    mArg(arg),
    mEvent(EVENT_NONE),
    mREvent(EVENT_NONE),
    mReadCallback(nullptr),
    mWriteCallback(nullptr),
    mErrorCallback(nullptr)
{

}

IOEvent * IOEvent::createNew(int fd, void *arg)
{
    if (fd < 0) {
        return nullptr;
    }

    return new IOEvent(fd, arg);
}

IOEvent * IOEvent::createNew(int fd)
{
    if (fd < 0) {
        return nullptr;
    }

    return new IOEvent(fd, (void *)0);
}

void IOEvent::handleEvent()
{
    if (mReadCallback && (mREvent & EVENT_READ)) {
        mReadCallback(mArg);
    }

    if (mWriteCallback && (mREvent & EVENT_WRITE)) {
        mWriteCallback(mArg);
    }

    if (mErrorCallback && (mREvent & EVENT_ERROR)) {
        mErrorCallback(mArg);
    }
}

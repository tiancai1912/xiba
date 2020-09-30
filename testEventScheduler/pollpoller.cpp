#include "pollpoller.h"

static const int pollTimeout = 10000;


PollPoller * PollPoller::createNew()
{
    return new PollPoller();
}

PollPoller::PollPoller()
{

}

PollPoller::~PollPoller()
{

}

bool PollPoller::addIoEvent(IOEvent *event)
{
    return updateIoEvent(event);
}

bool PollPoller::updateIoEvent(IOEvent *event)
{
    int fd = event->getFd();
    if (fd < 0) {
        printf("failed to add event\n");
        return false;
    }

    IOEventMap::iterator it = mEventMap.find(fd);
    if (it != mEventMap.end()) {
        PollFdMap::iterator it = mPollFdMap.find(fd);
        if (it == mPollFdMap.end()) {
            printf("can't find fd in map\n");
            return false;
        }

        int index = it->second;
        struct pollfd &pfd = mPollFdList.at(index);
        pfd.events = 0;
        pfd.revents = 0;

        if (event->isReadHandling()) {
            pfd.events |= POLLIN;
        } else if (event->isWriteHandling()) {
            pfd.events |= POLLOUT;
        } else if (event->isErrorHandling()) {
            pfd.events |= POLLERR;
        }
    } else {
        struct pollfd pfd;
        pfd.fd = fd;
        pfd.events = 0;
        pfd.revents = 0;

        if (event->isReadHandling()) {
            pfd.events |= POLLIN;
        } else if (event->isWriteHandling()) {
            pfd.events |= POLLOUT;
        } else if (event->isErrorHandling()) {
            pfd.events |= POLLERR;
        }

        mPollFdList.push_back(pfd);
        mEventMap.insert(std::make_pair(fd, event));
        mPollFdMap.insert(std::make_pair(fd, mPollFdList.size() - 1));
    }

    return true;
}

bool PollPoller::removeIoEvent(IOEvent *event)
{
    int fd = event->getFd();
    if (mEventMap.find(fd) == mEventMap.end()) {
        return false;
    }

    PollFdMap::iterator it = mPollFdMap.find(fd);
    if (it == mPollFdMap.end()) {
        return false;
    }

    int index = it->second;
    if (index != mPollFdList.size() - 1) {
        iter_swap(mPollFdList.begin() + index, mPollFdList.end() - 1);

        int tmpFd = mPollFdList.at(index).fd;
        it = mPollFdMap.find(tmpFd);
        it->second = index;
    }

    mPollFdList.pop_back();
    mPollFdMap.erase(fd);
    mEventMap.erase(fd);

    return true;
}

void PollPoller::handleEvent()
{
    int nums = 0;
    int fd = 0;
    int events = 0;
    int revents = 0;

    if (mPollFdList.empty()) {
        return;
    }

    nums = poll(&*mPollFdList.begin(), mPollFdList.size(), pollTimeout);
    if (nums < 0) {
        printf("poll error\n");
        return;
    }

    for (PollFdList::iterator it = mPollFdList.begin();
         it !=  mPollFdList.end() && nums > 0; ++it) {
        events = it->revents;
        if (events > 0) {
            revents = 0;
            fd = it->fd;
            IOEventMap::iterator it = mEventMap.find(fd);
            if (it == mEventMap.end()) {
                return;
            }

            if (events & POLLIN || events & POLLPRI || events & POLLRDHUP) {
                revents |= IOEvent::EVENT_READ;
            }

            if (events & POLLOUT) {
                revents |= IOEvent::EVENT_WRITE;
            }

            if (events & POLLERR) {
                revents |= IOEvent::EVENT_ERROR;
            }

            it->second->setREvent(revents);
            mEvents.push_back(it->second);

            --nums;
        }
    }

    for (std::vector<IOEvent *>::iterator it = mEvents.begin(); it != mEvents.end(); ++it) {
        (*it)->handleEvent();
    }

    mEvents.clear();

}

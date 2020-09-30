#ifndef POLLPOLLER_H
#define POLLPOLLER_H

#include "ioevent.h"
#include "poller.h"
#include <vector>
#include <map>
#include <poll.h>

class PollPoller : public Poller
{
public:

    static PollPoller * createNew();
    virtual ~PollPoller();

    virtual bool addIoEvent(IOEvent *event);
    virtual bool updateIoEvent(IOEvent *event);
    virtual bool removeIoEvent(IOEvent *event);
    virtual void handleEvent();

protected:

    PollPoller();

private:
    typedef std::vector<struct pollfd> PollFdList;
    PollFdList mPollFdList;

    typedef std::map<int, int> PollFdMap;
    PollFdMap mPollFdMap;

    std::vector<IOEvent *> mEvents;
};

#endif // POLLPOLLER_H

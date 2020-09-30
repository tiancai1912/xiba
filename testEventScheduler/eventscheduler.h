#ifndef EVENTSCHEDULER_H
#define EVENTSCHEDULER_H

#include "poller.h"
#include "ioevent.h"

class EventScheduler
{
public:
    typedef void (*Callback) (void *);

    enum PollerType
    {
        POLLER_SELECT,
        POLLER_POLL,
        POLLER_EPOLL,
    };

    static EventScheduler * createNew(PollerType type);

    virtual ~EventScheduler();
    bool addIOEvent(IOEvent *event);
    bool updateIOEvent(IOEvent *event);
    bool removeIOEvent(IOEvent *event);

    void loop();

private:
    static void handleReadCallback(void* arg);
    void handleRead();

protected:
    EventScheduler(PollerType type, int fd);

private:
    bool mQuit;
    Poller* mPoller;
};

#endif // EVENTSCHEDULER_H

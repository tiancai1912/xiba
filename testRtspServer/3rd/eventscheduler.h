#ifndef EVENTSCHEDULER_H
#define EVENTSCHEDULER_H

#include "poller.h"
#include "ioevent.h"

#include "timer2.h"

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

    Timer2::TimerId addTimedEventRunAfter(TimerEvent *event, Timer2::TimeInterval delay);
    Timer2::TimerId addTimedEventRunAt(TimerEvent *event, Timer2::TimeInterval when);
    Timer2::TimerId addTimedEventRunEvery(TimerEvent *event, Timer2::TimeInterval interval);
    bool removeTimedEvent(Timer2::TimerId timerid);

    void loop();

private:
    static void handleReadCallback(void* arg);
    void handleRead();

protected:
    EventScheduler(PollerType type, int fd);

private:
    bool mQuit;
    Poller *mPoller;

    TimerManager *mTimerManager;
};

#endif // EVENTSCHEDULER_H

#include <sys/eventfd.h>
#include <unistd.h>
#include <stdint.h>

#include "eventscheduler.h"
#include "pollpoller.h"

static int createEventFd()
{
    int evtFd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtFd < 0) {
        printf("failed to create event fd\n");
        return -1;
    }

    return evtFd;
}


EventScheduler::EventScheduler(PollerType type, int fd) :
    mQuit(false)
{
    mPoller = PollPoller::createNew();

    mTimerManager = TimerManager::createNew((PollPoller *)mPoller);
}

EventScheduler::~EventScheduler()
{
    delete mPoller;
}

EventScheduler * EventScheduler::createNew(PollerType type)
{
    if (type != POLLER_SELECT && type != POLLER_POLL && type != POLLER_EPOLL) {
        return nullptr;
    }

    int evtFd = createEventFd();
    if (evtFd < 0) {
        return nullptr;
    }

    return new EventScheduler(type, evtFd);
}

bool EventScheduler::addIOEvent(IOEvent *event)
{
    return mPoller->addIoEvent(event);
}

bool EventScheduler::updateIOEvent(IOEvent *event)
{
    return mPoller->updateIoEvent(event);
}

bool EventScheduler::removeIOEvent(IOEvent *event)
{
    return mPoller->removeIoEvent(event);
}

Timer2::TimerId EventScheduler::addTimedEventRunAfter(TimerEvent *event, Timer2::TimeInterval delay)
{
    Timer2::TimeStamp when = Timer2::getCurTime();
    when += delay;

    return mTimerManager->addTimer(event, when, 0);
}

Timer2::TimerId EventScheduler::addTimedEventRunAt(TimerEvent *event, Timer2::TimeInterval when)
{
    return mTimerManager->addTimer(event, when, 0);
}

Timer2::TimerId EventScheduler::addTimedEventRunEvery(TimerEvent *event, Timer2::TimeInterval interval)
{
    Timer2::TimeStamp when = Timer2::getCurTime();
    when += interval;

    return mTimerManager->addTimer(event, when, interval);
}

bool EventScheduler::removeTimedEvent(Timer2::TimerId timerid)
{
    return mTimerManager->removeTimer(timerid);
}

void EventScheduler::loop()
{
    while(mQuit != true) {
        mPoller->handleEvent();
    }
}

void EventScheduler::handleReadCallback(void* arg)
{
    if (!arg) {
        return;
    }

    EventScheduler *scheduler = (EventScheduler *)arg;
    scheduler->handleRead();
}

void EventScheduler::handleRead()
{
    //todo
//    uint64_t one;
//    while(::read())
}

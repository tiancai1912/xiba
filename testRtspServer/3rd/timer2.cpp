#include "timer2.h"

#include <sys/timerfd.h>

static int timerFdCreate(int clockid, int flags)
{
    return timerfd_create(clockid, flags);
}

static bool timerFdSetTime(int fd, Timer2::TimeStamp when, Timer2::TimeInterval period)
{
    struct itimerspec newVal;

    newVal.it_value.tv_sec = when / 1000; //ms->s
    newVal.it_value.tv_nsec = when % 1000 * 1000 * 1000; //ms->ns
    newVal.it_interval.tv_sec = period / 1000;
    newVal.it_interval.tv_nsec = period % 1000 * 1000 * 1000;

    if (timerfd_settime(fd, TFD_TIMER_ABSTIME, &newVal, NULL) < 0)
        return false;

    return true;
}

Timer2::Timer2(TimerEvent *event, TimeStamp timestamp, TimeInterval interval) :
    mTimerEvent(event),
    mTimeStamp(timestamp),
    mTimeInterval(interval)
{
    if (interval > 0) {
        mRepeat = true;
    } else {
        mRepeat = false;
    }
}

Timer2::~Timer2()
{

}

Timer2::TimeStamp Timer2::getCurTime()
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (now.tv_sec * 1000 + now.tv_nsec / 1000000);
}

void Timer2::handleEvent()
{
    if (!mTimerEvent) {
        return;
    }

    mTimerEvent->handleEvent();
}

TimerManager * TimerManager::createNew(PollPoller *poller)
{
    if (!poller) {
        return NULL;
    }

    int timerFd = timerFdCreate(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);

    if (timerFd < 0) {
        printf("failed to create timer fd\n");
        return NULL;
    }

    return new TimerManager(timerFd, poller);
}

TimerManager::TimerManager(int timerFd, PollPoller *poller):
    mTimerId(timerFd),
    mPoller(poller),
    mLastTimerId(0)
{
    mTimerIOEvent = IOEvent::createNew(mTimerId, this);
    mTimerIOEvent->setReadCallback(handleRead);
    mTimerIOEvent->enableReadHandling();
    modifyTimeout();
    mPoller->addIoEvent(mTimerIOEvent);
}

TimerManager::~TimerManager()
{
    mPoller->removeIoEvent(mTimerIOEvent);
    delete mTimerIOEvent;
}

Timer2::TimerId TimerManager::addTimer(TimerEvent *event, Timer2::TimeStamp timeStamp, Timer2::TimeInterval timeInterval)
{
    Timer2 timer(event, timeStamp, timeInterval);

    ++mLastTimerId;
    mTimers.insert(std::make_pair(mLastTimerId, timer));
    mEvents.insert(std::make_pair(TimerIndex(timeStamp, mLastTimerId), timer));

    modifyTimeout();
    return mLastTimerId;
}

bool TimerManager::removeTimer(Timer2::TimerId timerId)
{
    std::map<Timer2::TimerId, Timer2>::iterator it = mTimers.find(timerId);
    if (it != mTimers.end()) {
        Timer2::TimeStamp timestamp = it->second.mTimeStamp;
        Timer2::TimerId timerid = it->first;
        mEvents.erase(TimerIndex(timestamp, timerid));
        mTimers.erase(timerid);
    }

    modifyTimeout();
    return true;
}

void TimerManager::modifyTimeout()
{
    std::multimap<TimerIndex, Timer2>::iterator it = mEvents.begin();
    if (it != mEvents.end()) {
        Timer2 timer = it->second;
        timerFdSetTime(mTimerId, timer.mTimeStamp, timer.mTimeInterval);
    } else {
        timerFdSetTime(mTimerId, 0, 0);
    }
}

void TimerManager::handleRead(void *arg)
{
    if (!arg) {
        return;
    }

    TimerManager *timermanager = (TimerManager *)arg;
    timermanager->handleTimerEvent();
}

void TimerManager::handleTimerEvent()
{
    if (!mTimers.empty()) {
        int64_t timePoint = Timer2::getCurTime();
        while(!mTimers.empty() && mEvents.begin()->first.first <= timePoint) {
            Timer2::TimerId timerId = mEvents.begin()->first.second;
            Timer2 timer = mEvents.begin()->second;

            timer.handleEvent();
            mEvents.erase(mEvents.begin());
            if (timer.mRepeat == true) {
                timer.mTimeStamp = timePoint + timer.mTimeInterval;
                mEvents.insert(std::make_pair(TimerIndex(timer.mTimeStamp, timerId), timer));
            } else {
                mTimers.erase(timerId);
            }
        }
    }

    modifyTimeout();
}





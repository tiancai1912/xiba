#ifndef TIMER2_H
#define TIMER2_H

#include <map>
#include <stdint.h>
#include "pollpoller.h"
#include "ioevent.h"

class Timer2
{
public:
    typedef uint32_t TimerId;
    typedef int64_t TimeStamp; //ms
    typedef uint32_t TimeInterval; // ms

    ~Timer2();

    static TimeStamp getCurTime();

private:
    friend class TimerManager;
    Timer2(TimerEvent *event, TimeStamp timestamp, TimeInterval interval);
    void handleEvent();

private:
    TimerEvent *mTimerEvent;
    TimeStamp mTimeStamp;
    TimeInterval mTimeInterval;
    bool mRepeat;
};

class TimerManager
{
public:
    static TimerManager * createNew(PollPoller *poller);
    TimerManager(int timerFd, PollPoller *poller);
    ~TimerManager();

    Timer2::TimerId addTimer(TimerEvent *event, Timer2::TimeStamp timeStamp, Timer2::TimeInterval timeInterval);
    bool removeTimer(Timer2::TimerId timerId);

private:
    void modifyTimeout();
    static void handleRead(void * arg);
    void handleTimerEvent();

private:
    PollPoller *mPoller;
    int mTimerId;
    std::map<Timer2::TimerId, Timer2> mTimers;

    typedef std::pair<Timer2::TimeStamp, Timer2::TimerId> TimerIndex;
    std::multimap<TimerIndex, Timer2> mEvents;

    uint32_t mLastTimerId;
    IOEvent *mTimerIOEvent;
};


#endif // TIMER2_H

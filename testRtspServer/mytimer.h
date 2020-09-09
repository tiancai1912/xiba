#ifndef MYTIMER_H
#define MYTIMER_H

#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <sys/time.h>

// global declaration
typedef void (*Fun)(void);

class Timer;
class TimerManager;

// header file for Timer
class Timer
{
public:
    enum TimerType { ONCE, CIRCLE };

    Timer(TimerManager& manager);
    ~Timer();

    void Start(Fun fun, unsigned interval, TimerType timeType = CIRCLE);
    void Stop();

private:
    void OnTimer(unsigned long long now);

private:
    friend class TimerManager;
    TimerManager& manager_;
    TimerType timerType_;
    Fun timerFun_;
    unsigned interval_;
    unsigned long long expires_;

    int heapIndex_;
};

// header file for TimerManager
class TimerManager
{
public:
    static unsigned long long GetCurrentMillisecs();
    void DetectTimers();

private:
    friend class Timer;
    void AddTimer(Timer* timer);
    void RemoveTimer(Timer* timer);

    void UpHeap(int index);
    void DownHeap(int index);
    void SwapHeap(int, int index2);

private:
    struct HeapEntry
    {
        unsigned long long time;
        Timer* timer;
    };
    std::vector<HeapEntry> heap_;
};

#endif // MYTIMER_H

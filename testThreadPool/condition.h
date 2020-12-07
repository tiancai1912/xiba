#ifndef CONDITION_H
#define CONDITION_H

#include <thread>
#include <mutex>
#include <condition_variable>


struct condition1
{
    std::mutex mutex;
    std::unique_lock<std::mutex> lock;
    std::condition_variable cond;
};


void conditionInit(condition1 *cond);
void conditionLock(condition1 *cond);
void conditionUnlock(condition1 *cond);
void conditionWait(condition1 *cond);
bool conditionTimeWait(condition1 *cond, const timespec *ms);
void conditionSignal(condition1 *cond);
void conditionBroadcast(condition1 *cond);
void conditionDestroy(condition1 *cond);


#endif // CONDITION_H

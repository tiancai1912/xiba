#include "condition.h"

void conditionInit(condition1 *cond)
{
    cond->lock = std::unique_lock<std::mutex>(cond->mutex);
    cond->lock.unlock();
}

void conditionLock(condition1 *cond)
{
    cond->lock.lock();
}

void conditionUnlock(condition1 *cond)
{
    cond->lock.unlock();
}

void conditionWait(condition1 *cond)
{
    cond->cond.wait(cond->lock);
}

bool conditionTimeWait(condition1 *cond, const struct timespec * ms)
{
    return (bool)cond->cond.wait_for(cond->lock, std::chrono::milliseconds(ms->tv_sec * 1000));
}

void conditionSignal(condition1 *cond)
{
    cond->cond.notify_one();
}

void conditionBroadcast(condition1 *cond)
{
    cond->cond.notify_all();
}

void conditionDestroy(condition1 *cond)
{
}

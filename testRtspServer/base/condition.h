#ifndef CONDITION_H
#define CONDITION_H

#include <pthread.h>
#include "mutex.h"

class Condition
{
public:
    static Condition *createNew();

    ~Condition();

    void wait(Mutex *mutex);
    bool waitTimeout(Mutex *mutex, int ms);
    void signal();
    void broadcast();

private:
    Condition();

private:
    pthread_cond_t mCond;
};

#endif // CONDITION_H

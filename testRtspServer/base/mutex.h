#ifndef MUTEX_H
#define MUTEX_H

#include <pthread.h>

class Mutex
{
public:
    ~Mutex();

    static Mutex *createNew();
    void lock();
    void unlock();

    pthread_mutex_t *get() { return &mMutex; }

private:
    Mutex();

private:
    pthread_mutex_t mMutex;

};

class MutexLockGuard
{
public:
    MutexLockGuard(Mutex *mutex);
    ~MutexLockGuard();

private:
    Mutex *mMutex;
};

#endif // MUTEX_H

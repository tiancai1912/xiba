#include "mutex.h"

Mutex::Mutex()
{
    pthread_mutex_init(&mMutex, NULL);
}


Mutex::~Mutex()
{
    pthread_mutex_destroy(&mMutex);
}

Mutex * Mutex::createNew()
{
    return new Mutex();
}

void Mutex::lock()
{
    pthread_mutex_lock(&mMutex);
}

void Mutex::unlock()
{
    pthread_mutex_unlock(&mMutex);
}

MutexLockGuard::MutexLockGuard(Mutex *mutex) :
    mMutex(mutex)
{
    mMutex->lock();
}

MutexLockGuard::~MutexLockGuard()
{
    mMutex->unlock();
}


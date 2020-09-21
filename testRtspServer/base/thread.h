#ifndef THREAD_H
#define THREAD_H

#include <pthread.h>

class Thread
{
public:
    ~Thread();

    bool start(void *arg);
    bool detach();
    bool join();
    bool cancel();
    pthread_t getThreadId() const;


protected:
    Thread();

    virtual void run(void *arg) = 0;

private:
    static void *threadRun(void *arg);

private:
    void *mArg;
    bool mIsStart;
    bool mIsDetach;
    pthread_t mThreadId;
};

#endif // THREAD_H

#ifndef THREADPOOL_H
#define THREADPOOL_H

#include "condition.h"

class Threadpool
{
public:
    Threadpool();

    struct Task {
        void * (*run)(void *args);
        void *arg;
        struct Task *next;
    };

    struct threadpool {
      condition1 cond;
      Task *first;
      Task *last;
      int counter;
      int idle;
      int maxThreads;
      int quit;
    };

    void init(threadpool *pool, int threads);
    void addTask(threadpool *pool, void * (*run)(void *args), void *arg);
    void destroy(threadpool *pool);

};

#endif // THREADPOOL_H

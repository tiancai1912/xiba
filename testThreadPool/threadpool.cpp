#include "threadpool.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>

void *threadRoutine(void *arg)
{
    struct timespec abstime;
    int timeout;
    printf("thread %d is starting\n", (int)pthread_self());
    Threadpool::threadpool *pool = (Threadpool::threadpool *)arg;
    if (pool == NULL) {
        printf("invalid param\n");
    }

//    while(1) {
        timeout = 0;
        conditionLock(&(pool->cond));
//        pool->idle++;

//        while(pool->first == NULL && !pool->quit) {
//            printf("thread %d is waiting\n", (int)pthread_self());
//            clock_gettime(CLOCK_REALTIME, &abstime);
//            abstime.tv_sec += 2;
//            bool status;
//            status = conditionTimeWait(&(pool->cond), &abstime);
//            if (status == false) {
//                printf("thread %d wait time out\n", (int)pthread_self());
//                timeout = 1;
//                break;
//            }
//        }

//        pool->idle--;
//        if (pool->first != NULL) {
//            Threadpool::Task *task = pool->first;
//            pool->first = task->next;
//            conditionUnlock(&(pool->cond));
//            task->run(task->arg);
//            free(task);
//            conditionLock(&(pool->cond));
//        }

//        if (pool->quit && pool->first == NULL) {
//            pool->counter--;
//            if (pool->counter == 0) {
//                conditionSignal(&(pool->cond));
//            }

//            conditionUnlock(&(pool->cond));
//            break;
//        }

//        if (timeout == 1) {
//            pool->counter--;
//            conditionUnlock(&(pool->cond));
//            break;
//        }

//        conditionUnlock(&(pool->cond));

//        break;
//    }

    printf("thread %d is exiting\n", (int)pthread_self());
//    return;
}


Threadpool::Threadpool()
{

}

void Threadpool::init(threadpool *pool, int threads)
{
    conditionInit(&(pool->cond));
    pool->first = NULL;
    pool->last = NULL;
    pool->counter = 0;
    pool->idle = 0;
    pool->maxThreads = threads;
    pool->quit = 0;
}

void Threadpool::addTask(threadpool *pool, void * (*run)(void *args), void *arg)
{
    Task *task = (Task *)malloc(sizeof(Task));
    task->run = run;
    task->arg = arg;
    task->next = NULL;

    conditionLock(&(pool->cond));

    if (pool->first == NULL) {
        pool->first = task;
    } else {
        pool->last->next = task;
    }

    pool->last = task;

    if (pool->idle > 0) {
        printf("haha\n");
        conditionSignal(&(pool->cond));
    } else if (pool->counter < pool->maxThreads) {
        printf("xixi\n");
        std::thread *thread = new std::thread(threadRoutine, pool);
        pool->counter++;
        thread->join();
    }

    conditionUnlock(&(pool->cond));
}

void Threadpool::destroy(threadpool *pool)
{
    if (pool->quit)
        return;

    conditionLock(&(pool->cond));
    pool->quit = 1;
    if (pool->counter > 0) {
        if (pool->idle > 0) {
            conditionBroadcast(&(pool->cond));
        }

        while(pool->counter) {
            conditionWait(&(pool->cond));
        }
    }

    conditionUnlock(&(pool->cond));
    conditionDestroy(&(pool->cond));
}


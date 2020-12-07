#ifndef TESTCONDITION_H
#define TESTCONDITION_H

#include <pthread.h>

typedef struct condition_t {
    pthread_mutex_t pmutex;
    pthread_cond_t pcond;
}condition_t;

int condition_init(condition_t *cond);
int condition_lock(condition_t *cond);
int condition_unlock(condition_t *cond);
int condition_wait(condition_t *cond);
int condition_timewait(condition_t *cond, const timespec *abstime);
int condition_signal(condition_t *cond);
int condition_broadcast(condition_t *cond);
int condition_destroy(condition_t *cond);



#endif // TESTCONDITION_H

#ifndef PISTACHIO_PTHREAD_H
#define PISTACHIO_PTHREAD_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void *impl;
} pthread_t;

typedef int pthread_attr_t;
typedef struct { int unused; } pthread_mutex_t;
typedef int pthread_mutexattr_t;

int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                   void *(*start_routine)(void *), void *arg);
int pthread_join(pthread_t thread, void **retval);

int pthread_mutex_init(pthread_mutex_t *mutex,
                       const pthread_mutexattr_t *attr);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);
int pthread_mutex_destroy(pthread_mutex_t *mutex);

#ifdef __cplusplus
}
#endif

#endif // PISTACHIO_PTHREAD_H

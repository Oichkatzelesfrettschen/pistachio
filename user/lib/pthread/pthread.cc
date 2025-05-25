#include <pthread.h>

extern "C" {

int pthread_create(pthread_t *thread, const pthread_attr_t *,
                   void *(*start_routine)(void *), void *arg)
{
    thread->impl = start_routine(arg);
    return 0;
}

int pthread_join(pthread_t thread, void **retval)
{
    if (retval)
        *retval = thread.impl;
    return 0;
}

int pthread_mutex_init(pthread_mutex_t *, const pthread_mutexattr_t *)
{
    return 0;
}

int pthread_mutex_lock(pthread_mutex_t *)
{
    return 0;
}

int pthread_mutex_unlock(pthread_mutex_t *)
{
    return 0;
}

int pthread_mutex_destroy(pthread_mutex_t *)
{
    return 0;
}

} // extern "C"

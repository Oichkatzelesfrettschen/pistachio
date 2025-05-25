#include "posix.h"
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <vector>
#include <algorithm>

// These stub implementations directly invoke the host system calls.
// Future versions will forward requests to user-level servers using
// the exokernel IPC helpers.

using timer_vec = std::vector<posix_timer*>;
static timer_vec timers;

void _posix_timer_register(posix_timer *t)
{
    timers.push_back(t);
}

void _posix_timer_unregister(posix_timer *t)
{
    auto it = std::remove(timers.begin(), timers.end(), t);
    timers.erase(it, timers.end());
}

size_t posix_timer_count(void)
{
    return timers.size();
}

int posix_open(const char *path, int flags, unsigned mode)
{
    return open(path, flags, mode);
}

ssize_t posix_read(int fd, void *buf, size_t count)
{
    return read(fd, buf, count);
}

ssize_t posix_write(int fd, const void *buf, size_t count)
{
    return write(fd, buf, count);
}

pid_t posix_fork(void)
{
    return fork();
}

int posix_nanosleep(const struct timespec *req, struct timespec *rem)
{
    if (!req)
        return -1;

    posix_timer t;
    clock_gettime(CLOCK_MONOTONIC, &t.expiry);
    t.expiry.tv_sec += req->tv_sec;
    t.expiry.tv_nsec += req->tv_nsec;
    if (t.expiry.tv_nsec >= 1000000000L) {
        t.expiry.tv_sec += 1;
        t.expiry.tv_nsec -= 1000000000L;
    }
    _posix_timer_register(&t);

    int ret;
    do {
        ret = nanosleep(req, rem);
    } while (ret < 0 && errno == EINTR);

    _posix_timer_unregister(&t);
    return ret;
}


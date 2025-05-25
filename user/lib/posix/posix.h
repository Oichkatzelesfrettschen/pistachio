#ifndef POSIX_LIB_H
#define POSIX_LIB_H

#include <stddef.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

int posix_open(const char *path, int flags, unsigned mode);
ssize_t posix_read(int fd, void *buf, size_t count);
ssize_t posix_write(int fd, const void *buf, size_t count);
pid_t posix_fork(void);

struct timespec;

typedef struct posix_timer {
    struct timespec expiry;
} posix_timer;

void _posix_timer_register(posix_timer *t);
void _posix_timer_unregister(posix_timer *t);
size_t posix_timer_count(void);

int posix_nanosleep(const struct timespec *req, struct timespec *rem);

#ifdef __cplusplus
}
#endif

#endif /* POSIX_LIB_H */

#ifndef POSIX_LIB_H
#define POSIX_LIB_H

#include <stddef.h>
#include <sys/types.h>
#include <l4/thread.h>

#ifdef __cplusplus
extern "C" {
#endif

int posix_open(const char *path, int flags, unsigned mode);
ssize_t posix_read(int fd, void *buf, size_t count);
ssize_t posix_write(int fd, const void *buf, size_t count);
pid_t posix_fork(void);

/* Capability aware IPC wrappers */
int posix_mq_open_cap(L4_ThreadId_t cap);
int posix_sem_open_cap(L4_ThreadId_t cap);
int posix_shm_open_cap(L4_ThreadId_t cap);

#ifdef __cplusplus
}
#endif

#endif /* POSIX_LIB_H */

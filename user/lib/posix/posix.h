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
int posix_dup(int fd);
int posix_close(int fd);
int posix_mkfifo(const char *path, unsigned mode);
pid_t posix_fork(void);

#ifdef __cplusplus
}
#endif

#endif /* POSIX_LIB_H */

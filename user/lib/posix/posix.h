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
int posix_mprotect(void *addr, size_t len, int prot);
int posix_msync(void *addr, size_t len, int flags);

#ifdef __cplusplus
}
#endif

#endif /* POSIX_LIB_H */

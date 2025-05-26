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

/* Socket option helpers */
int posix_setsockopt(int fd, int level, int optname,
                     const void *optval, socklen_t optlen);
int posix_getsockopt(int fd, int level, int optname,
                     void *optval, socklen_t *optlen);

/* Internet address conversion helpers */
int posix_inet_pton(int af, const char *src, void *dst);
const char *posix_inet_ntop(int af, const void *src,
                             char *dst, socklen_t size);

#ifdef __cplusplus
}
#endif

#endif /* POSIX_LIB_H */

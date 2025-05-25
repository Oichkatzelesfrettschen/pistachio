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

int posix_sigaction(int sig, const struct sigaction *act,
                    struct sigaction *oldact);
int posix_sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
int posix_killpg(pid_t pgid, int sig);

#ifdef __cplusplus
}
#endif

#endif /* POSIX_LIB_H */

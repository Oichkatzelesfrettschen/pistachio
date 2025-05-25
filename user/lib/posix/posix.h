#ifndef POSIX_LIB_H
#define POSIX_LIB_H

#include <stddef.h>
#include <sys/types.h>
#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

int posix_open(const char *path, int flags, unsigned mode);
ssize_t posix_read(int fd, void *buf, size_t count);
ssize_t posix_write(int fd, const void *buf, size_t count);
pid_t posix_fork(void);
int posix_link(const char *oldpath, const char *newpath);
int posix_unlink(const char *path);
char *posix_getcwd(char *buf, size_t size);
int posix_chdir(const char *path);
int posix_execve(const char *path, char *const argv[], char *const envp[]);
pid_t posix_wait(int *status);
pid_t posix_waitpid(pid_t pid, int *status, int options);
int posix_socket(int domain, int type, int protocol);
int posix_bind(int sockfd, const struct sockaddr *addr, socklen_t len);
int posix_listen(int sockfd, int backlog);
int posix_accept(int sockfd, struct sockaddr *addr, socklen_t *len);

#ifdef __cplusplus
}
#endif

#endif /* POSIX_LIB_H */

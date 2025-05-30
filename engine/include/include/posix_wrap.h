#ifndef POSIX_WRAP_H
#define POSIX_WRAP_H

#include <stddef.h>
#include <sys/types.h>
#include <sys/socket.h>

int lx_link(const char *oldpath, const char *newpath);
int lx_unlink(const char *path);
char *lx_getcwd(char *buf, size_t size);
int lx_chdir(const char *path);
int lx_execvep(const char *file, char *const argv[], char *const envp[]);

pid_t lx_waitpid(pid_t pid, int *status, int options);
pid_t lx_wait(int *status);

int lx_socket_cloexec(int domain, int type, int protocol);
int lx_accept_cloexec(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int lx_send_fd(int sockfd, int fd);
int lx_recv_fd(int sockfd);

#endif /* POSIX_WRAP_H */

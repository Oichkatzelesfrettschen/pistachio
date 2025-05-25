#include "posix.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/socket.h>

// These stub implementations directly invoke the host system calls.
// Future versions will forward requests to user-level servers using
// the exokernel IPC helpers.

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

int posix_link(const char *oldpath, const char *newpath)
{
    return link(oldpath, newpath);
}

int posix_unlink(const char *path)
{
    return unlink(path);
}

char *posix_getcwd(char *buf, size_t size)
{
    return getcwd(buf, size);
}

int posix_chdir(const char *path)
{
    return chdir(path);
}

int posix_execve(const char *path, char *const argv[], char *const envp[])
{
    return execve(path, argv, envp);
}

pid_t posix_wait(int *status)
{
    return wait(status);
}

pid_t posix_waitpid(pid_t pid, int *status, int options)
{
    return waitpid(pid, status, options);
}

int posix_socket(int domain, int type, int protocol)
{
    return socket(domain, type, protocol);
}

int posix_bind(int sockfd, const struct sockaddr *addr, socklen_t len)
{
    return bind(sockfd, addr, len);
}

int posix_listen(int sockfd, int backlog)
{
    return listen(sockfd, backlog);
}

int posix_accept(int sockfd, struct sockaddr *addr, socklen_t *len)
{
    return accept(sockfd, addr, len);
}


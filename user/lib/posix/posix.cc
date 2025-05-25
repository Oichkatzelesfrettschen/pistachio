#include "posix.h"
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

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

int posix_execv(const char *path, char *const argv[])
{
    return execv(path, argv);
}

pid_t posix_spawn(const char *path, char *const argv[])
{
    pid_t pid = fork();
    if (pid == 0) {
        execv(path, argv);
        _exit(127);
    }
    return pid;
}

pid_t posix_waitpid(pid_t pid, int *status, int options)
{
    return waitpid(pid, status, options);
}


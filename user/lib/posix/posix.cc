#include "posix.h"
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <map>

namespace {
enum Rights { Read = 1, Write = 2 };
static std::map<int, int> fd_rights;

static int rights_from_flags(int flags)
{
    int r = 0;
    if ((flags & O_RDWR) || (flags & O_RDONLY))
        r |= Rights::Read;
    if ((flags & O_RDWR) || (flags & O_WRONLY))
        r |= Rights::Write;
    return r;
}
}

// These stub implementations directly invoke the host system calls.
// Future versions will forward requests to user-level servers using
// the exokernel IPC helpers.

int posix_open(const char *path, int flags, unsigned mode)
{
    int fd = open(path, flags, mode);
    if (fd >= 0)
        fd_rights[fd] = rights_from_flags(flags);
    return fd;
}

ssize_t posix_read(int fd, void *buf, size_t count)
{
    auto it = fd_rights.find(fd);
    if (it == fd_rights.end()) {
        errno = EBADF;
        return -1;
    }
    if (!(it->second & Rights::Read)) {
        errno = EACCES;
        return -1;
    }
    return read(fd, buf, count);
}

ssize_t posix_write(int fd, const void *buf, size_t count)
{
    auto it = fd_rights.find(fd);
    if (it == fd_rights.end()) {
        errno = EBADF;
        return -1;
    }
    if (!(it->second & Rights::Write)) {
        errno = EACCES;
        return -1;
    }
    return write(fd, buf, count);
}

pid_t posix_fork(void)
{
    return fork();
}


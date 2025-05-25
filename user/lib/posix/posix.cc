#include "posix.h"
#include <fcntl.h>
#include <unistd.h>
#include <map>
#include <string>
#include <vector>

// These stub implementations directly invoke the host system calls.
// Future versions will forward requests to user-level servers using
// the exokernel IPC helpers.

/*
 * Simple in-process FIFO support. mkfifo() creates a pipe and stores it
 * in an internal table keyed by pathname.  open/read/write/dup/close
 * recognise file descriptors that refer to these FIFOs and operate on
 * the underlying pipe endpoints.
 */

struct FifoEntry {
    int fds[2];      // 0: read end, 1: write end
    int refcnt;
    std::string name;
};

static std::map<std::string, int> fifo_by_name;
static std::vector<FifoEntry> fifo_table;
static std::map<int, std::pair<int,int>> fifo_handles; // fd -> {index,end}
static int next_fifo_fd = 10000; // distinguish from host descriptors

static int alloc_fifo_handle(int idx, int end)
{
    int fd = next_fifo_fd++;
    fifo_handles[fd] = {idx, end};
    fifo_table[idx].refcnt++;
    return fd;
}

int posix_mkfifo(const char *path, unsigned mode)
{
    if (fifo_by_name.count(path))
        return -1;
    int fds[2];
    if (pipe(fds) < 0)
        return -1;
    FifoEntry e;
    e.fds[0] = fds[0];
    e.fds[1] = fds[1];
    e.refcnt = 0;
    e.name = path;
    fifo_table.push_back(e);
    fifo_by_name[path] = fifo_table.size() - 1;
    (void)mode;
    return 0;
}

int posix_open(const char *path, int flags, unsigned mode)
{
    auto it = fifo_by_name.find(path);
    if (it != fifo_by_name.end()) {
        int idx = it->second;
        int end = ((flags & O_ACCMODE) == O_WRONLY) ? 1 : 0;
        return alloc_fifo_handle(idx, end);
    }
    return open(path, flags, mode);
}

ssize_t posix_read(int fd, void *buf, size_t count)
{
    auto it = fifo_handles.find(fd);
    if (it != fifo_handles.end()) {
        int idx = it->second.first;
        int end = it->second.second;
        return read(fifo_table[idx].fds[end], buf, count);
    }
    return read(fd, buf, count);
}

ssize_t posix_write(int fd, const void *buf, size_t count)
{
    auto it = fifo_handles.find(fd);
    if (it != fifo_handles.end()) {
        int idx = it->second.first;
        int end = it->second.second;
        return write(fifo_table[idx].fds[end], buf, count);
    }
    return write(fd, buf, count);
}

int posix_dup(int fd)
{
    auto it = fifo_handles.find(fd);
    if (it != fifo_handles.end())
        return alloc_fifo_handle(it->second.first, it->second.second);
    return dup(fd);
}

int posix_close(int fd)
{
    auto it = fifo_handles.find(fd);
    if (it != fifo_handles.end()) {
        int idx = it->second.first;
        fifo_handles.erase(it);
        if (--fifo_table[idx].refcnt == 0) {
            close(fifo_table[idx].fds[0]);
            close(fifo_table[idx].fds[1]);
            fifo_by_name.erase(fifo_table[idx].name);
            fifo_table[idx].fds[0] = -1;
            fifo_table[idx].fds[1] = -1;
            fifo_table[idx].name.clear();
        }
        return 0;
    }
    return close(fd);
}

pid_t posix_fork(void)
{
    return fork();
}


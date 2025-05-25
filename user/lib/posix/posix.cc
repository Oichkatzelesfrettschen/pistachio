#include "posix.h"
#include <cstdint>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <unordered_map>

// Keep track of the protection flags for every page that has been mapped
// through the POSIX helpers. The key is the page aligned address.
static std::unordered_map<void *, int> page_prot;
static long page_size = 0;

static long get_page_size() {
  if (!page_size)
    page_size = sysconf(_SC_PAGESIZE);
  return page_size;
}

static void remember_prot(void *addr, size_t len, int prot) {
  long ps = get_page_size();
  uintptr_t start =
      (reinterpret_cast<uintptr_t>(addr) & ~(static_cast<uintptr_t>(ps) - 1));
  uintptr_t end = (reinterpret_cast<uintptr_t>(addr) + len + ps - 1) &
                  ~(static_cast<uintptr_t>(ps) - 1);
  for (uintptr_t p = start; p < end; p += ps)
    page_prot[reinterpret_cast<void *>(p)] = prot;
}

static void forget_prot(void *addr, size_t len) {
  long ps = get_page_size();
  uintptr_t start =
      (reinterpret_cast<uintptr_t>(addr) & ~(static_cast<uintptr_t>(ps) - 1));
  uintptr_t end = (reinterpret_cast<uintptr_t>(addr) + len + ps - 1) &
                  ~(static_cast<uintptr_t>(ps) - 1);
  for (uintptr_t p = start; p < end; p += ps)
    page_prot.erase(reinterpret_cast<void *>(p));
}

// Map memory and record its initial protection flags. Currently this simply
// forwards to the host mmap implementation.
static void *tracked_mmap(void *addr, size_t length, int prot, int flags,
                          int fd, off_t offset) {
  void *ret = mmap(addr, length, prot, flags, fd, offset);
  if (ret != MAP_FAILED)
    remember_prot(ret, length, prot);
  return ret;
}

static int tracked_munmap(void *addr, size_t length) {
  int r = munmap(addr, length);
  if (r == 0)
    forget_prot(addr, length);
  return r;
}

// These stub implementations directly invoke the host system calls.
// Future versions will forward requests to user-level servers using
// the exokernel IPC helpers.

int posix_open(const char *path, int flags, unsigned mode) {
  return open(path, flags, mode);
}

ssize_t posix_read(int fd, void *buf, size_t count) {
  return read(fd, buf, count);
}

ssize_t posix_write(int fd, const void *buf, size_t count) {
  return write(fd, buf, count);
}

pid_t posix_fork(void) { return fork(); }

int posix_mprotect(void *addr, size_t len, int prot) {
  int ret = mprotect(addr, len, prot);
  if (ret == 0)
    remember_prot(addr, len, prot);
  return ret;
}

int posix_msync(void *addr, size_t len, int flags) {
  return msync(addr, len, flags);
}

#include <l4io.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <stdio.h>

int main(void)
{
    const char *fname = "memdemo.tmp";
    int fd = open(fname, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    size_t ps = (size_t)getpagesize();
    if (ftruncate(fd, ps) < 0) {
        perror("ftruncate");
        close(fd);
        return 1;
    }

    char *p = (char *)mmap(NULL, ps, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (p == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    strcpy(p, "hello world\n");
    if (msync(p, ps, MS_SYNC) < 0)
        perror("msync");
    if (mprotect(p, ps, PROT_READ) < 0)
        perror("mprotect");

    munmap(p, ps);
    close(fd);
    unlink(fname);
    printf("memory demo done\n");
    return 0;
}

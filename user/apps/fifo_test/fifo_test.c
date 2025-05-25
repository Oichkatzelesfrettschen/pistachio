#include "posix.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    const char *name = "testfifo";
    if (posix_mkfifo(name, 0600) < 0) {
        perror("mkfifo");
        return 1;
    }
    int w = posix_open(name, O_WRONLY, 0);
    int r = posix_open(name, O_RDONLY, 0);
    if (w < 0 || r < 0) {
        perror("open");
        return 1;
    }

    const char *msg = "fifo\n";
    if (posix_write(w, msg, strlen(msg)) != (ssize_t)strlen(msg)) {
        perror("write");
        return 1;
    }

    char buf[16] = {0};
    if (posix_read(r, buf, sizeof(buf)) <= 0) {
        perror("read");
        return 1;
    }

    posix_close(w);
    posix_close(r);

    if (strncmp(buf, msg, strlen(msg)) != 0) {
        fprintf(stderr, "mismatch\n");
        return 1;
    }

    puts("fifo test ok");
    return 0;
}

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(void) {
    const char *fname = "posix_test.tmp";
    const char *msg = "hello\n";
    int fd = open(fname, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open");
        return 1;
    }
    ssize_t written = write(fd, msg, strlen(msg));
    if (written != (ssize_t)strlen(msg)) {
        perror("write");
        close(fd);
        unlink(fname);
        return 1;
    }
    close(fd);

    char buf[16] = {0};
    fd = open(fname, O_RDONLY);
    if (fd < 0) {
        perror("open");
        unlink(fname);
        return 1;
    }
    ssize_t n = read(fd, buf, sizeof(buf) - 1);
    if (n < 0) {
        perror("read");
        close(fd);
        unlink(fname);
        return 1;
    }
    close(fd);
    unlink(fname);

    if (strncmp(buf, msg, strlen(msg)) != 0) {
        fprintf(stderr, "content mismatch\n");
        return 1;
    }

    puts("file test ok");
    return 0;
}

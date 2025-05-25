#include "../../lib/posix/posix.h"
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int main(void) {
    const char *src = "posix_sample.tmp";
    const char *dst = "posix_sample.link";
    int fd = posix_open(src, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open");
        return 1;
    }
    posix_write(fd, "sample\n", 7);
    close(fd);
    if (posix_link(src, dst) < 0) {
        perror("link");
        return 1;
    }
    posix_unlink(src);
    posix_unlink(dst);
    return 0;
}

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "../../user/lib/posix/posix.h"

int main(void) {
    const char *fname = "perm_test.tmp";
    int fd = posix_open(fname, O_RDONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open");
        return 1;
    }
    ssize_t ret = posix_write(fd, "x", 1);
    if (ret != -1 || errno != EACCES) {
        fprintf(stderr, "write should fail with EACCES\n");
        close(fd);
        unlink(fname);
        return 1;
    }
    close(fd);
    unlink(fname);
    puts("permissions test ok");
    return 0;
}

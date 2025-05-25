#include "../../lib/posix/posix.h"
#include <stdio.h>

int main(void) {
    char buf[256];
    if (!posix_getcwd(buf, sizeof(buf))) {
        perror("getcwd");
        return 1;
    }
    printf("current dir: %s\n", buf);
    if (posix_chdir("/")) {
        perror("chdir");
        return 1;
    }
    if (!posix_getcwd(buf, sizeof(buf))) {
        perror("getcwd");
        return 1;
    }
    printf("new dir: %s\n", buf);
    return 0;
}

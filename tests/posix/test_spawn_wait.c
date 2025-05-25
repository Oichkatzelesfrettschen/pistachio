#include "posix.h"
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void) {
    char *const args[] = {(char *)"./build/tests/posix/spawn_child", NULL};
    pid_t pid = posix_spawn(args[0], args);
    if (pid < 0) {
        perror("posix_spawn");
        return 1;
    }

    int status = 0;
    if (posix_waitpid(pid, &status, 0) < 0) {
        perror("posix_waitpid");
        return 1;
    }

    if (!WIFEXITED(status) || WEXITSTATUS(status) != 7) {
        fprintf(stderr, "unexpected status %d\n", status);
        return 1;
    }

    puts("spawn wait test ok");
    return 0;
}

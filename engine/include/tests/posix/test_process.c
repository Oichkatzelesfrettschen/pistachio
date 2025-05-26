#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }
    if (pid == 0) {
        puts("child running");
        _exit(42);
    }

    int status = 0;
    if (waitpid(pid, &status, 0) < 0) {
        perror("waitpid");
        return 1;
    }

    if (!WIFEXITED(status) || WEXITSTATUS(status) != 42) {
        fprintf(stderr, "unexpected status %d\n", status);
        return 1;
    }

    puts("process test ok");
    return 0;
}

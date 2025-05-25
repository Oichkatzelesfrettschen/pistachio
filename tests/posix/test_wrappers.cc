#include "../../user/lib/posix/posix.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void) {
    char saved[256];
    if (!posix_getcwd(saved, sizeof(saved))) {
        perror("getcwd");
        return 1;
    }
    if (posix_chdir("/tmp")) {
        perror("chdir");
        return 1;
    }
    char buf[256];
    if (!posix_getcwd(buf, sizeof(buf))) {
        perror("getcwd");
        return 1;
    }
    if (strcmp(buf, "/tmp") != 0) {
        fprintf(stderr, "unexpected cwd %s\n", buf);
        return 1;
    }
    posix_chdir(saved);

    const char *src = "posix_wrap.tmp";
    const char *dst = "posix_wrap.link";
    int fd = posix_open(src, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open");
        return 1;
    }
    posix_write(fd, "x", 1);
    close(fd);
    if (posix_link(src, dst) < 0) {
        perror("link");
        return 1;
    }
    posix_unlink(src);
    posix_unlink(dst);

    pid_t pid = posix_fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }
    if (pid == 0) {
        char *const args[] = {(char*)"/bin/true", NULL};
        char *const env[] = {NULL};
        posix_execve("/bin/true", args, env);
        _exit(1);
    }
    int status = 0;
    if (posix_waitpid(pid, &status, 0) != pid) {
        perror("waitpid");
        return 1;
    }
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        fprintf(stderr, "child failed\n");
        return 1;
    }
    puts("wrapper test ok");
    return 0;
}

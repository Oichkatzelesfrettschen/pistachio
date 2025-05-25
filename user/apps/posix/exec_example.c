#include "../../lib/posix/posix.h"
#include <stdio.h>
#include <unistd.h>

int main(void) {
    char *const args[] = {"/bin/echo", "hello from execve", NULL};
    char *const env[] = {"FOO=bar", NULL};
    posix_execve("/bin/echo", args, env);
    perror("execve");
    return 1;
}

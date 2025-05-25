#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(void) {
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        perror("socket");
        return 1;
    }

    int val = 1;
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0) {
        perror("setsockopt");
        close(s);
        return 1;
    }

    int got = 0;
    socklen_t len = sizeof(got);
    if (getsockopt(s, SOL_SOCKET, SO_REUSEADDR, &got, &len) < 0) {
        perror("getsockopt");
        close(s);
        return 1;
    }

    if (got != 1) {
        fprintf(stderr, "expected 1 got %d\n", got);
        close(s);
        return 1;
    }

    puts("sockopt test ok");
    close(s);
    return 0;
}

#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

int main(void) {
    struct in_addr addr;
    if (inet_pton(AF_INET, "127.0.0.1", &addr) != 1) {
        perror("inet_pton");
        return 1;
    }

    char buf[INET_ADDRSTRLEN] = {0};
    const char *res = inet_ntop(AF_INET, &addr, buf, sizeof(buf));
    if (!res) {
        perror("inet_ntop");
        return 1;
    }

    if (strcmp(buf, "127.0.0.1") != 0) {
        fprintf(stderr, "unexpected result %s\n", buf);
        return 1;
    }

    puts("address helper test ok");
    return 0;
}

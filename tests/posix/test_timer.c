#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdio.h>
#include <time.h>

int main(void) {
    struct timespec req = {0};
    req.tv_sec = 0;
    req.tv_nsec = 100000000; // 100 ms
    struct timespec start, end;

    if (clock_gettime(CLOCK_MONOTONIC, &start) != 0) {
        perror("clock_gettime");
        return 1;
    }
    if (nanosleep(&req, NULL) != 0) {
        perror("nanosleep");
        return 1;
    }
    if (clock_gettime(CLOCK_MONOTONIC, &end) != 0) {
        perror("clock_gettime");
        return 1;
    }

    long elapsed_ms = (end.tv_sec - start.tv_sec) * 1000;
    elapsed_ms += (end.tv_nsec - start.tv_nsec) / 1000000;

    if (elapsed_ms < 50 || elapsed_ms > 200) {
        fprintf(stderr, "nanosleep duration out of range: %ldms\n", elapsed_ms);
        return 1;
    }

    puts("timer test ok");
    return 0;
}

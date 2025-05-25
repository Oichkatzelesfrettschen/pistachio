#include "posix.h"
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

static volatile sig_atomic_t handled = 0;

static void handler(int sig)
{
    (void)sig;
    handled = 1;
}

int main()
{
    struct sigaction sa{};
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    if (posix_sigaction(SIGUSR1, &sa, nullptr) < 0) {
        perror("sigaction");
        return 1;
    }

    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    if (posix_sigprocmask(SIG_BLOCK, &mask, nullptr) < 0) {
        perror("sigprocmask block");
        return 1;
    }

    if (posix_killpg(0, SIGUSR1) < 0) {
        perror("killpg");
        return 1;
    }

    if (handled) {
        fprintf(stderr, "handler executed while blocked\n");
        return 1;
    }

    if (posix_sigprocmask(SIG_UNBLOCK, &mask, nullptr) < 0) {
        perror("sigprocmask unblock");
        return 1;
    }

    for (int i = 0; i < 10 && !handled; ++i) {
        usleep(10000);
    }

    if (!handled) {
        fprintf(stderr, "handler did not execute\n");
        return 1;
    }

    puts("signal test ok");
    return 0;
}

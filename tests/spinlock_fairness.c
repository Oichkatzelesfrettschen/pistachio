#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>

#ifndef TEST_DURATION
#define TEST_DURATION 2
#endif

#ifndef FAIRNESS_RATIO
#define FAIRNESS_RATIO 0.8
#endif

typedef struct {
    atomic_uint next;
    atomic_uint serving;
} ticketlock_t;

static inline void ticketlock_init(ticketlock_t *lock) {
    atomic_init(&lock->next, 0);
    atomic_init(&lock->serving, 0);
}

static inline void ticketlock_lock(ticketlock_t *lock) {
    unsigned ticket = atomic_fetch_add_explicit(&lock->next, 1, memory_order_relaxed);
    while (atomic_load_explicit(&lock->serving, memory_order_acquire) != ticket) {
        ;
    }
}

static inline void ticketlock_unlock(ticketlock_t *lock) {
    atomic_fetch_add_explicit(&lock->serving, 1, memory_order_release);
}

struct thread_data {
    ticketlock_t *lock;
    volatile int *stop;
    unsigned long count;
};

static void *worker(void *arg) {
    struct thread_data *td = arg;
    while (!*td->stop) {
        ticketlock_lock(td->lock);
        td->count++;
        ticketlock_unlock(td->lock);
    }
    return NULL;
}

int main(int argc, char **argv) {
    int n_threads = 4;
    if (argc > 1)
        n_threads = atoi(argv[1]);

    pthread_t *threads = calloc(n_threads, sizeof(*threads));
    struct thread_data *td = calloc(n_threads, sizeof(*td));
    if (!threads || !td) {
        perror("alloc");
        return 1;
    }

    ticketlock_t lock;
    ticketlock_init(&lock);
    volatile int stop = 0;

    for (int i = 0; i < n_threads; ++i) {
        td[i].lock = &lock;
        td[i].stop = &stop;
        td[i].count = 0;
        if (pthread_create(&threads[i], NULL, worker, &td[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }

    sleep(TEST_DURATION);
    stop = 1;

    for (int i = 0; i < n_threads; ++i)
        pthread_join(threads[i], NULL);

    unsigned long min = td[0].count, max = td[0].count;
    for (int i = 0; i < n_threads; ++i) {
        printf("thread %d: %lu\n", i, td[i].count);
        if (td[i].count < min)
            min = td[i].count;
        if (td[i].count > max)
            max = td[i].count;
    }

    free(threads);
    free(td);

    double ratio = (double)min / (double)max;
    if (ratio < FAIRNESS_RATIO) {
        fprintf(stderr, "Fairness check failed: min/max ratio %.2f < %.2f\n", ratio, (double)FAIRNESS_RATIO);
        return 1;
    }

    return 0;
}


#include <pthread.h>   // thread primitives
#include <stdatomic.h> // C11 atomic operations
#include <stdio.h>     // printf
#include <stdlib.h>    // calloc, free
#include <unistd.h>    // sleep

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
  unsigned ticket =
      atomic_fetch_add_explicit(&lock->next, 1, memory_order_relaxed);
  while (atomic_load_explicit(&lock->serving, memory_order_acquire) != ticket) {
    ;
  }
}

static inline void ticketlock_unlock(ticketlock_t *lock) {
  atomic_fetch_add_explicit(&lock->serving, 1, memory_order_release);
}

typedef struct thread_data {
  ticketlock_t *lock;  // lock to protect the counter
  volatile int *stop;  // flag to stop worker threads
  unsigned long count; // number of lock acquisitions
} thread_data;

static void *worker(void *arg) {
  // Cast the opaque argument back to our thread context
  auto *td = static_cast<thread_data *>(arg);
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

  // Allocate thread handles and per-thread data
  pthread_t *threads =
      static_cast<pthread_t *>(calloc(n_threads, sizeof(pthread_t)));
  thread_data *td =
      static_cast<thread_data *>(calloc(n_threads, sizeof(thread_data)));
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
    fprintf(stderr, "Fairness check failed: min/max ratio %.2f < %.2f\n", ratio,
            (double)FAIRNESS_RATIO);
    return 1;
  }

  return 0;
}

#include "posix.h"
#include <exo_ipc.h>

/*
 * These stubs simply forward to the exokernel IPC helpers.
 * Real implementations will format requests and interpret replies
 * according to the POSIX servers' protocols.
 */

int posix_mq_open_cap(L4_ThreadId_t cap)
{
    return exo_send(cap) == EXO_IPC_OK ? 0 : -1;
}

int posix_sem_open_cap(L4_ThreadId_t cap)
{
    return exo_send(cap) == EXO_IPC_OK ? 0 : -1;
}

int posix_shm_open_cap(L4_ThreadId_t cap)
{
    return exo_send(cap) == EXO_IPC_OK ? 0 : -1;
}



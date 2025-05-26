#ifndef EXO_IPC_H
#define EXO_IPC_H

#include <l4/ipc.h>
#include <l4/thread.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Status codes returned by the exokernel IPC helpers. The values
 * roughly mirror L4 error codes but are independent from the
 * architecture specific constants.
 */

typedef enum {
    EXO_IPC_OK = 0,
    EXO_IPC_TIMEOUT,
    EXO_IPC_INVALID_PARTNER,
    EXO_IPC_OVERFLOW,
    EXO_IPC_ERROR,
} exo_ipc_status;

exo_ipc_status exo_send(L4_ThreadId_t to);
exo_ipc_status exo_recv(L4_ThreadId_t from, L4_ThreadId_t *sender);

#ifdef __cplusplus
}
#endif

#endif /* EXO_IPC_H */

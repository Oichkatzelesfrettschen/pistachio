#ifndef DOOR_H
#define DOOR_H

#include <exo_ipc.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline exo_ipc_status door_call(L4_ThreadId_t door)
{
    return exo_send(door);
}

#ifdef __cplusplus
}
#endif

#endif /* DOOR_H */

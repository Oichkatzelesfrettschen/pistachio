#ifndef L4_USER_IPC_H
#define L4_USER_IPC_H

#include <l4/types.h>
#include <l4/message.h>
#include <l4/thread.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Wrapper around the kernel IPC primitive. When the destination
 * is the current thread and the call does not receive from another
 * partner the operation can be satisfied in user mode.
 */
L4_MsgTag_t L4_UserIpc(L4_ThreadId_t to,
                      L4_ThreadId_t FromSpecifier,
                      L4_Word_t Timeouts,
                      L4_ThreadId_t *from);

#ifdef __cplusplus
}
#endif

#endif /* L4_USER_IPC_H */

#pragma once
#include <l4/types.h>
#include <l4/thread.h>
#include <l4/message.h>
#include <l4/compiler.h>

#ifdef __cplusplus
extern "C" {
#endif

[[nodiscard]] L4_ThreadId_t L4_CDECL mailbox_recv_t(L4_MsgTag_t *tag, L4_Time_t timeout);

static inline L4_ThreadId_t Recv_T(L4_MsgTag_t *tag, L4_Time_t timeout)
{
    return mailbox_recv_t(tag, timeout);
}

#ifdef __cplusplus
}
#endif

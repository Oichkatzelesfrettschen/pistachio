#pragma once
#include <l4/ipc.h>
#include <l4/thread.h>
#include <l4/message.h>

/**
 * \brief Status codes for IPC helper wrappers.
 */

enum class exo_ipc_status {
    Ok = 0,
    SendTimeout = 1,
    NoPartner = 2,
    Cancelled = 3,
    MsgOverflow = 4,
    XferTimeoutCurrent = 5,
    XferTimeoutPartner = 6,
    Aborted = 7,
};

/** Convert an IPC message tag and the current thread's error code to a status. */
[[nodiscard]] static inline exo_ipc_status exo_ipc_from_tag(L4_MsgTag_t tag)
{
    if (L4_IpcSucceeded(tag))
        return exo_ipc_status::Ok;

    L4_Word_t err = (L4_ErrorCode() >> 1) & 0x7;
    switch (err) {
    case 1: return exo_ipc_status::SendTimeout;
    case 2: return exo_ipc_status::NoPartner;
    case 3: return exo_ipc_status::Cancelled;
    case 4: return exo_ipc_status::MsgOverflow;
    case 5: return exo_ipc_status::XferTimeoutCurrent;
    case 6: return exo_ipc_status::XferTimeoutPartner;
    case 7: return exo_ipc_status::Aborted;
    default: return exo_ipc_status::Ok;
    }
}

[[nodiscard]] static inline exo_ipc_status exo_call(L4_ThreadId_t to)
{
    return exo_ipc_from_tag(L4_Call(to));
}

[[nodiscard]] static inline exo_ipc_status exo_send(L4_ThreadId_t to)
{
    return exo_ipc_from_tag(L4_Send(to));
}

[[nodiscard]] static inline exo_ipc_status exo_reply(L4_ThreadId_t to)
{
    return exo_ipc_from_tag(L4_Reply(to));
}

[[nodiscard]] static inline exo_ipc_status exo_receive(L4_ThreadId_t from)
{
    return exo_ipc_from_tag(L4_Receive(from));
}


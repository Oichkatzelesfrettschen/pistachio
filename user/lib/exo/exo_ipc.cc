#include <exo_ipc.h>
#include <l4/message.h>

// Simple wrappers around the L4 IPC primitives that translate
// the kernel error codes into the exo_ipc_status enumeration.

static exo_ipc_status translate_error(L4_MsgTag_t tag)
{
    if (L4_IpcSucceeded(tag))
        return EXO_IPC_OK;

    L4_Word_t err = L4_ErrorCode();
    // L4 error codes are architecture specific. We map the most
    // common ones used by the examples into portable values.
    switch (err) {
    case L4_ERROR_INVALID_THREAD:
        return EXO_IPC_INVALID_PARTNER;
    default:
        return EXO_IPC_ERROR;
    }
}

exo_ipc_status exo_send(L4_ThreadId_t to)
{
    return translate_error(L4_Send(to));
}

exo_ipc_status exo_recv(L4_ThreadId_t from, L4_ThreadId_t *sender)
{
    L4_ThreadId_t src;
    L4_MsgTag_t tag = L4_Receive(from, &src);
    if (sender)
        *sender = src;
    return translate_error(tag);
}

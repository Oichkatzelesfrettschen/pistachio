#include <l4/ipc_user.h>
#include <l4/ipc.h>

extern "C" L4_MsgTag_t
L4_UserIpc(L4_ThreadId_t to, L4_ThreadId_t FromSpecifier,
           L4_Word_t Timeouts, L4_ThreadId_t *from)
{
    /* Fast path for self IPC without receive phase. */
    if (L4_IsThreadEqual(to, L4_Myself()) && L4_IsNilThread(FromSpecifier)) {
        if (from) {
            *from = L4_nilthread;
        }
        return L4_MsgTag();
    }

    /* Fallback to kernel primitive. */
    return L4_Ipc(to, FromSpecifier, Timeouts, from);
}

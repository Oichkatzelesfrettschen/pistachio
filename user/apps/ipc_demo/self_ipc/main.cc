#include <l4/ipc.h>
#include <stdio.h>

int main() {
    L4_MsgTag_t tag = L4_UserIpc(L4_Myself(), L4_nilthread,
                                 L4_Timeouts(L4_Never, L4_Never), nullptr);
    if (L4_IpcSucceeded(tag))
        printf("self IPC succeeded\n");
    else
        printf("self IPC failed: %lu\n", L4_ErrorCode());
    return 0;
}

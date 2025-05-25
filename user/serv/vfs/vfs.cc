#include <l4/ipc.h>
#include <stdio.h>

int main()
{
    printf("VFS server started\n");
    while (1) {
        L4_ThreadId_t from = L4_nilthread;
        L4_Receive(from, &from);
        L4_Reply(from);
    }
    return 0;
}

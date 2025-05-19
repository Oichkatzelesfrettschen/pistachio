#include <l4/thread.h>
#include <stdio.h>

int main()
{
    printf("Scheduler server started\n");

    while (1)
    {
        /* simple round-robin placeholder */
        L4_ThreadSwitch(L4_nilthread);
    }
    return 0;
}

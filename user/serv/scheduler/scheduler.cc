#include <l4/thread.h>
#include <l4/ipc.h>
#include <stdio.h>
#include <deque>
#include "../../src-userland/lib/sched/sched_client.h"

int main()
{
    printf("Scheduler server started\n");

    sched_set_server(L4_Myself());
    std::deque<L4_ThreadId_t> queue;

    while (1)
    {
        SchedRequest req;
        L4_ThreadId_t from;
        sched_wait_request(&req, &from);

        /* enqueue requesting thread */
        queue.push_back(from);

        if (!queue.empty())
        {
            L4_ThreadId_t next = queue.front();
            queue.pop_front();
            sched_switch(next);
        }
    }

    return 0;
}

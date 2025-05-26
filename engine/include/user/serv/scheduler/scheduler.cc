#include <l4/thread.h>
#include <l4/ipc.h>
#include <stdio.h>
#include <deque>
#include "../../lib/sched/sched_client.h"

enum class SchedLabel : L4_Word_t { Request = 0x1234 };

int main()
{
    printf("Scheduler server started\n");

    sched_set_server(L4_Myself());
    std::deque<L4_ThreadId_t> queue;

    while (1)
    {
        SchedRequest req;
        L4_ThreadId_t from;
        L4_MsgTag_t tag = sched_wait_request(&req, &from);
        if (L4_Label(tag) != static_cast<L4_Word_t>(SchedLabel::Request))
            continue;

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

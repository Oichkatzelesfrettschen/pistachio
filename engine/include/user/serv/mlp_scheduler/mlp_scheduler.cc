#include <l4/thread.h>
#include <l4/ipc.h>
#include <stdio.h>
#include <deque>
#include "../../lib/sched/sched_client.h"
#include "../../lib/mlp/mlp.h"

enum class SchedLabel : L4_Word_t { Request = 0x1234 };

int main(int argc, char **argv)
{
    const char *model = argc > 1 ? argv[1] : nullptr;
    mlp_init(model);

    printf("MLP scheduler server started\n");

    sched_set_server(L4_Myself());
    std::deque<L4_ThreadId_t> queue;

    while (1)
    {
        SchedRequest req;
        L4_ThreadId_t from;
        L4_MsgTag_t tag = sched_wait_request(&req, &from);
        if (L4_Label(tag) != static_cast<L4_Word_t>(SchedLabel::Request))
            continue;

        queue.push_back(from);

        if (!queue.empty())
        {
            float feat[1] = { static_cast<float>(queue.size()) };
            int front = mlp_predict(feat);
            L4_ThreadId_t next;
            if (front || queue.size() == 1)
            {
                next = queue.front();
                queue.pop_front();
            }
            else
            {
                next = queue.back();
                queue.pop_back();
            }
            sched_switch(next);
        }
    }

    return 0;
}

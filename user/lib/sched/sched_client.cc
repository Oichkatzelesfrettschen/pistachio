#include "sched_client.h"
#include <cstddef>

static L4_ThreadId_t server_tid = {0};
static const L4_Word_t SCHED_LABEL = 0x1234; /* arbitrary */

void sched_set_server(L4_ThreadId_t tid)
{
    server_tid = tid;
}

L4_ThreadId_t sched_get_server(void)
{
    return server_tid;
}

L4_MsgTag_t sched_send_request(const SchedRequest *req)
{
    L4_Msg_t msg;
    L4_Word_t w[5] = { req->thread.raw, req->time_control,
                       req->processor_control, req->prio_control,
                       req->preemption_control };
    L4_MsgPut(&msg, SCHED_LABEL, 5, w, 0, nullptr);
    L4_MsgLoad(&msg);
    return L4_Call(server_tid);
}

L4_MsgTag_t sched_wait_request(SchedRequest *req, L4_ThreadId_t *from)
{
    L4_Msg_t msg;
    L4_MsgTag_t tag = L4_Wait(from);
    L4_MsgStore(tag, &msg);
    if (L4_Label(tag) == SCHED_LABEL && L4_UntypedWords(tag) == 5) {
        req->thread.raw = L4_MsgWord(&msg, 0);
        req->time_control = L4_MsgWord(&msg, 1);
        req->processor_control = L4_MsgWord(&msg, 2);
        req->prio_control = L4_MsgWord(&msg, 3);
        req->preemption_control = L4_MsgWord(&msg, 4);
    }
    return tag;
}

void sched_switch(L4_ThreadId_t next)
{
    L4_ThreadSwitch(next);
}


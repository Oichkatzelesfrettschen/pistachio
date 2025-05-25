#include "sched_client.h"
#include <array>

static L4_ThreadId_t server_tid = {0};
enum class SchedLabel : L4_Word_t { Request = 0x1234 };

void sched_set_server(L4_ThreadId_t tid)
{
    server_tid = tid;
}

[[nodiscard]] L4_ThreadId_t sched_get_server(void)
{
    return server_tid;
}

[[nodiscard]] L4_MsgTag_t sched_send_request(const SchedRequest *req)
{
    L4_Msg_t msg;
    std::array<L4_Word_t, 5> w { req->thread.raw, req->time_control,
                                 req->processor_control, req->prio_control,
                                 req->preemption_control };
    L4_MsgPut(&msg, static_cast<L4_Word_t>(SchedLabel::Request), w.size(), w.data(), 0, nullptr);
    L4_MsgLoad(&msg);
    return L4_Call(server_tid);
}

[[nodiscard]] L4_MsgTag_t sched_wait_request(SchedRequest *req, L4_ThreadId_t *from)
{
    L4_Msg_t msg;
    L4_MsgTag_t tag = L4_Wait(from);
    L4_MsgStore(tag, &msg);
    if (L4_Label(tag) == static_cast<L4_Word_t>(SchedLabel::Request) && L4_UntypedWords(tag) == 5) {
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


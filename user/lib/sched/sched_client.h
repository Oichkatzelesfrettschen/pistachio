#ifndef SCHED_CLIENT_H
#define SCHED_CLIENT_H

#include <l4/thread.h>
#include <l4/message.h>
#include <l4/ipc.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    L4_ThreadId_t thread;
    L4_Word_t time_control;
    L4_Word_t processor_control;
    L4_Word_t prio_control;
    L4_Word_t preemption_control;
} SchedRequest;

void sched_set_server(L4_ThreadId_t tid);
[[nodiscard]] L4_ThreadId_t sched_get_server(void);

[[nodiscard]] L4_MsgTag_t sched_send_request(const SchedRequest *req);
[[nodiscard]] L4_MsgTag_t sched_wait_request(SchedRequest *req, L4_ThreadId_t *from);
void sched_switch(L4_ThreadId_t next);

#ifdef __cplusplus
}
#endif

#endif /* SCHED_CLIENT_H */

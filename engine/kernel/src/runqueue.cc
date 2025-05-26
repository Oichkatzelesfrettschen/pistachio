#include <runqueue.h>
#include INC_API(queueing.h)

extern "C" void setrunqueue(tcb_t **head, tcb_t *tcb)
{
    if (!tcb)
        return;
    ENQUEUE_LIST_TAIL((*head), tcb, sched_state.ready_list);
}

extern "C" void remrq(tcb_t **head, tcb_t *tcb)
{
    if (!tcb)
        return;
    DEQUEUE_LIST((*head), tcb, sched_state.ready_list);
}

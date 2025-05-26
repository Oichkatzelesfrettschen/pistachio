#include <l4/ipc.h>
#include <l4/message.h>
#include <l4/thread.h>
#include <time.h>
#include <string.h>

#ifndef MAILBOX_BUFSZ
#define MAILBOX_BUFSZ 16
#endif

typedef struct {
    L4_MsgTag_t tags[MAILBOX_BUFSZ];
    L4_ThreadId_t senders[MAILBOX_BUFSZ];
    unsigned head;
} mailbox_ipc_queue_t;

static mailbox_ipc_queue_t mailbox_ipcs;

static inline unsigned long long timeout_to_us(L4_Time_t t)
{
    if (t.raw == 0)
        return ~(unsigned long long)0;
    unsigned long long man = t.raw & 0x3ffU;
    unsigned long long exp = (t.raw >> 10) & 0x1fU;
    return man << exp;
}

static L4_ThreadId_t kernel_ipc_queue_recv_timed(mailbox_ipc_queue_t *q,
                                                 L4_MsgTag_t *tag,
                                                 L4_Time_t timeout)
{
    const struct timespec nap = {0, 1000000}; /* 1ms */
    unsigned long long us = timeout_to_us(timeout);
    L4_Clock_t end;
    if (timeout.raw == 0)
        end.raw = ~0ULL;
    else
        end = L4_ClockAddUsec(L4_SystemClock(), us);

    L4_MsgTag_t t;
    L4_ThreadId_t from;

    for (;;) {
        t = L4_Wait_Timeout(L4_ZeroTime, &from);
        if (!L4_IpcFailed(t)) {
            unsigned idx = q->head % MAILBOX_BUFSZ;
            q->tags[idx] = t;
            q->senders[idx] = from;
            q->head++;
            if (tag)
                *tag = t;
            return from;
        }
        if (timeout.raw != 0 && L4_IsClockLater(L4_SystemClock(), end)) {
            if (tag)
                *tag = t;
            return L4_nilthread;
        }
        nanosleep(&nap, NULL);
    }
}

L4_ThreadId_t mailbox_recv_t(L4_MsgTag_t *tag, L4_Time_t timeout)
{
    static int initialised;
    if (!initialised) {
        memset(&mailbox_ipcs, 0, sizeof(mailbox_ipcs));
        initialised = 1;
    }

    return kernel_ipc_queue_recv_timed(&mailbox_ipcs, tag, timeout);
}

#ifndef __RUNQUEUE_H__
#define __RUNQUEUE_H__

#include INC_API(tcb.h)

extern "C" void setrunqueue(tcb_t **head, tcb_t *tcb);
extern "C" void remrq(tcb_t **head, tcb_t *tcb);

#endif /* __RUNQUEUE_H__ */

# Thread Scheduling Deep Dive

This document explains how Pistachio's user-space scheduler uses `L4_ThreadSwitch` and the `sched_switch` helper to dispatch threads.

## Kernel primitive: `L4_ThreadSwitch`

`L4_ThreadSwitch` is one of the few scheduling-related syscalls that remain in the exokernel. It simply performs a context switch to the specified thread ID without modifying run queues or time accounting. The kernel validates that the destination thread exists and resides in the same address space before transferring control.

On PowerPC the function is defined in `l4/powerpc/syscalls.h` and is implemented via an inline assembly stub. The stub moves the destination thread ID into register `r3`, loads the syscall entry pointer and branches through the function pointer. No additional arguments are inspected and no scheduling policy is applied—user space is responsible for deciding which thread to run next.

```c
// Simplified excerpt from l4/powerpc/syscalls.h
typedef L4_Word_t (*__L4_ThreadSwitch_t)(L4_Word_t);
extern __L4_ThreadSwitch_t __L4_ThreadSwitch;

L4_INLINE void L4_ThreadSwitch(L4_ThreadId_t dest)
{
    L4_Word_t r3 __asm__("r3") = dest.raw;
    __asm__ __volatile__(
        "mtctr %1 ;"
        "bctrl ;"
        : "+r"(r3)
        : "r"(__L4_ThreadSwitch)
        : "r4", "r5", "r6", "r7", "r8", "r9", "r10", "memory");
}
```

## User library helper: `sched_switch`

The scheduler libraries provide the simple wrapper `sched_switch` defined in `user/lib/sched/sched_client.cc`. It merely forwards the target thread ID to `L4_ThreadSwitch`. Keeping this call in a helper allows servers to abstract the low-level syscall and potentially inject additional logic (e.g., logging or accounting) without touching every scheduling site.

```c++
// user/lib/sched/sched_client.cc
void sched_switch(L4_ThreadId_t next)
{
    L4_ThreadSwitch(next);
}
```

## Server usage

Both `user/serv/scheduler` and `user/serv/mlp_scheduler` use `sched_switch` after dequeuing the next runnable thread. They receive scheduling requests over IPC, push the requesting thread onto a queue and then select the next thread to run. Once chosen, they invoke `sched_switch` which calls `L4_ThreadSwitch` to hand execution to that thread.

This design keeps the kernel's role minimal while allowing sophisticated policies in user space. By tweaking the queue management and selection algorithm inside the servers, developers can implement round-robin, priority based or machine‑learning driven scheduling without altering kernel code.

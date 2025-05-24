# Server IPC Interfaces

This document outlines the expected IPC messages used by the example
resource managers.  Each service defines a small message format which
is exchanged over L4 IPC.

## Memory Server

The memory server receives requests consisting of a four word message.
`MR0` contains the label `0`.  `MR1`--`MR3` encode the following
structure:

```c
struct mem_request {
    mem_opcode op;    /* MEM_ALLOC or MEM_FREE */
    L4_Word_t  size;  /* bytes to allocate or free */
    L4_Word_t  addr;  /* address when freeing */
};
```

For a `MEM_ALLOC` request the server returns the allocated address in
`MR1`.  `MEM_FREE` requests return `0`.

## Scheduler Server

Scheduler messages use label `0x1234`.  Five untyped words encode the
`sched_client.h` `SchedRequest` structure.  Threads send a request and
then block until the server replies.  The server switches to the next
thread using `L4_ThreadSwitch` and replies when the time slice is
consumed.

## Device Servers

Future device servers will follow the same pattern: a unique message
label and a packed request structure starting at `MR1`.  Clients will
send device specific operations (read/write, interrupt handling, and
configuration) and wait for the server's reply.

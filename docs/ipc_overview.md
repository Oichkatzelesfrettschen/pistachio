# IPC Overview

This document summarises how L4 IPC is used by the example servers and where additional services can plug in.

## Core primitives

`L4_Ipc` is the kernel syscall that sends a message to one thread while optionally receiving from another. The operation uses the message registers (`MR0`..`MR63`) for payload. `MR0` stores the `L4_MsgTag_t` which encodes the message label, counts of untyped and typed words and several flags. `L4_UserIpc` wraps the syscall and performs a user mode fast path when the destination is the current thread and no receive phase is required.

Most high level calls such as `L4_Call`, `L4_Send` and `L4_Receive` are implemented on top of `L4_UserIpc` in `l4/ipc.h`.

## Message formats

The example resource managers follow a simple convention detailed in `docs/server_interfaces.md`. `MR0` contains a label that identifies the service while the remaining untyped words carry a packed request or reply structure starting at `MR1`.

### Memory server

The memory server expects four untyped words. `MR0` is the label `0`, and `MR1`--`MR3` match the `mem_request` structure defined in `l4/memory.h`:

```c
struct mem_request {
    mem_opcode op;    /* MEM_ALLOC or MEM_FREE */
    L4_Word_t  size;  /* bytes to allocate or free */
    L4_Word_t  addr;  /* address when freeing */
};
```

For an allocation request the server returns the allocated address in `MR1`.

### Scheduler server

Scheduler messages use label `0x1234`. Five untyped words encode the `SchedRequest` structure from `user/lib/sched/sched_client.h`. Threads send a request with `L4_Call` and block until the server replies or performs a `L4_ThreadSwitch` to schedule the next thread.

## Defining additional services

New servers should pick a unique label and define packed request/reply structures. Client libraries can use `L4_MsgPut` or direct `L4_LoadMR` calls to copy these structures into `MR1` and beyond. Replies typically mirror the same layout. The examples under `user/serv/` provide small reference implementations.

## Cap'n Proto integration

Cap'n Proto RPC framing can ride on top of these IPC primitives. A service could send Cap'n Proto segments using the message registers or map shared buffers via flexpages to avoid copying large payloads. Framing information would be defined in the Cap'n Proto schema while delivery still uses `L4_Ipc` or `L4_UserIpc`. This would allow rich RPC semantics without changing the underlying kernel interface.

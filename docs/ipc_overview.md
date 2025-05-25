# IPC Overview

This document summarises how L4 IPC is used by the example servers and where additional services can plug in.

## Core primitives

`L4_Ipc` is the kernel syscall that sends a message to one thread while optionally receiving from another. The operation uses the message registers (`MR0`..`MR63`) for payload. `MR0` stores the `L4_MsgTag_t` which encodes the message label, counts of untyped and typed words and several flags. `L4_UserIpc` wraps the syscall and performs a user mode fast path when the destination is the current thread and no receive phase is required.

Most high level calls such as `L4_Call`, `L4_Send` and `L4_Receive` are implemented on top of `L4_UserIpc` in `l4/ipc.h`.
`Recv_T` from `mailbox_t.h` builds on these helpers and waits for an incoming
message with a timeout. It repeatedly performs a non-blocking wait and sleeps
for short periods using `nanosleep` until either a message is delivered or the
timeout expires. The function returns the sender's thread ID and stores the
received tag in the supplied pointer.

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

## `exo_ipc_status` enumeration

To simplify reasoning about IPC results, Pistachio provides a small helper
enumeration defined in `l4/exo_ipc.h`. The helpers convert the raw message tag
and thread error code into one of the following status values:

| Value | Meaning |
|-------|--------------------------------------------------------------|
| `Ok` | Operation completed successfully. |
| `SendTimeout` | The send phase timed out. |
| `NoPartner` | The destination thread does not exist. |
| `Cancelled` | The IPC was cancelled before completion. |
| `MsgOverflow` | More words were sent than fit into the partner's buffer. |
| `XferTimeoutCurrent` | Copy phase timed out in the current thread. |
| `XferTimeoutPartner` | Copy phase timed out in the partner thread. |
| `Aborted` | Transfer was aborted due to page faults or interrupts. |

The `exo_call`, `exo_send`, `exo_reply` and `exo_receive` helpers return this
enumeration instead of a raw `L4_MsgTag_t`, making it easier for unit tests and
applications to check for specific failure modes.

## Wrapper helpers

`exo_ipc.h` provides small wrapper functions that directly correspond to the
kernel IPC primitives. Each helper simply invokes the matching `L4_*` call and
converts the resulting message tag and error code into an `exo_ipc_status`
value:

| Helper | Kernel call | Notes |
|--------|-------------|-------|
| `exo_call(to)` | `L4_Call(to)` | Send a message and wait for a reply. |
| `exo_send(to)` | `L4_Send(to)` | Asynchronous send operation. |
| `exo_reply(to)` | `L4_Reply(to)` | Reply to a waiting partner. |
| `exo_receive(from)` | `L4_Receive(from)` | Block until a message arrives. |

The standalone C API declared in `include/exo_ipc.h` exposes similar wrappers
(`exo_send` and `exo_recv`) for plain C code. These functions return the same
status codes and make it easy to write portable tests without dealing with the
architecture specific error values.

## Capability-based IPC objects

Higher level facilities such as message queues, semaphores and shared memory are
designed around capabilities. Each object is represented by an opaque capability
value which is passed to the relevant server over IPC. The server validates the
capability and performs the requested operation on behalf of the caller. This
keeps the kernel interface minimal while still allowing multiple user-space
implementations of these abstractions.


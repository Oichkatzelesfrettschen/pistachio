# POSIX Compatibility Layer

Pistachio's exokernel exposes only primitive operations such as threads, IPC and
address spaces.  Traditional UNIX functionality therefore lives entirely in
user space.  A small set of servers and libraries can implement enough of the
POSIX interface for applications to run without increasing the kernel's
responsibilities.

## Architecture Overview

The kernel provides the minimal mechanisms described in
[`docs/exokernel_migration.md`](exokernel_migration.md).  User‑level resource
servers build on these primitives: the memory server manages heap allocations
and the scheduler server selects which thread runs next.  A POSIX subsystem sits
on top of these resource managers and offers familiar system calls to
applications.

```
Applications
   |
POSIX syscall library (libposix)
   |
+-------------------+-----------------+
|       Servers that implement POSIX   |
+-------------------------------------+
       | memory | scheduler | VFS | device |
```

Applications link against a user-space library that translates system calls into
IPC messages.  Each server implements a subset of the POSIX APIs and relies on
the kernel only for low level IPC and context switching.

## Required User-Level Components

- **Virtual File System (VFS) server** – handles path resolution and dispatches
  file operations to specific file system or device servers.  It maintains
  per-process file descriptor tables and enforces permissions.
- **Process server** – implements `fork`, `exec` and other process management
  primitives.  It coordinates address space creation with the memory server and
  arranges for threads to start running via the scheduler.
- **Device servers** – expose block devices, terminals and other hardware to the
  VFS.  They follow the message formats outlined in
  [`docs/server_interfaces.md`](server_interfaces.md).
- **POSIX syscall library** – a small C library that provides the standard
  functions (`open`, `read`, `write`, `fork`, etc.) and forwards them to the
  above servers using the IPC helpers from `user/lib/exo`.

Additional helpers such as a signal delivery library or a time server can be
added to flesh out more of the specification.

## Using Existing IPC and Servers

The POSIX library communicates with servers using the IPC conventions already in
place.  File-related calls package their arguments into the message registers
and send them to the VFS server.  Memory allocation requests use the existing
`mem_request` structures defined in `l4/memory.h`.  Scheduling decisions are
coordinated through the `sched_client` helpers so that blocking operations yield
control until a reply is received.

By layering the POSIX subsystem on top of the exokernel's primitives, the kernel
remains small while user-level servers provide the rich API expected by
applications.

## Threading Support

`libpthread` offers a very small subset of the POSIX threads interface.  Only
`pthread_create()` and `pthread_join()` merely execute the start routine synchronously in the calling thread. Mutex functions exist only as stubs and provide no mutual exclusion. Condition variables and the rest of the API are unimplemented.

## Reference Specification

Full copies of the POSIX specification are available under `docs/ben-books`. The `susv4-2018` HTML tree contains the Single UNIX Specification, version 4 (2018). Consult these documents when implementing system calls or verifying behaviour.


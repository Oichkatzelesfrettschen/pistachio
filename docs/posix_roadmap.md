# POSIX Compliance Roadmap

This document synthesises the long term plan for implementing a full POSIX
compatibility layer on top of Pistachio's exokernel.  The design draws from the
existing exokernel migration notes and the partial servers found under
`user/serv/`.  Each phase expands the current stubs into a feature complete
user-space subsystem while keeping the kernel minimal.

## Phase I – Build and Documentation Foundations

1. **Unified build system** – integrate the existing Makefiles and CMake build
   helpers into a single workflow that supports cross compilation.  Refer to
   [`docs/building.md`](building.md) for current instructions.
2. **Core headers** – establish a stable header layout under `include/` for the
   POSIX wrappers and capability-aware APIs.
3. **Documentation tooling** – extend the `docs/` tree with automatically
   generated API references and security guidelines.

## Phase II – Memory Management Subsystem

1. **Virtual memory manager** – extend the example `memory` server
   (`user/serv/memory`) with page protection tracking and capability checks.
2. **POSIX wrappers** – implement `px_mprotect`, `px_msync` and `px_mmap` in
   `user/lib/posix` using IPC messages to the memory server.
3. **Testing** – create unit tests under `tests/` that exercise protection and
   mapping behaviour.

## Phase III – Process Management & Scheduling

1. **Run queue structures** – flesh out `user/serv/scheduler` with a proper run
   queue and capability-aware state management.
2. **Process server** – build on `user/serv/process` to provide `px_waitpid`,
   `px_execve` and `px_spawn` interfaces.
3. **Scheduling integration** – coordinate with the scheduler server to enforce
   preemptive multitasking and resource accounting.

## Phase IV – Inter-Process Communication

1. **Signal handling** – implement `px_sigaction` and a signal delivery helper
   library.  Update the process server to manage signal contexts.
2. **FIFO channels** – extend the VFS server with in-memory FIFO nodes backed by
   the existing IPC primitives.
3. **Validation** – add regression tests covering signal registration and FIFO
   read/write behaviour.

## Phase V – File System and I/O

1. **Capability-aware VFS** – replace the current `vfs` stub with a server that
   resolves paths, tracks file descriptors per process and dispatches to device
   servers.  See [`docs/server_interfaces.md`](server_interfaces.md) for message
   conventions.
2. **Directory operations** – provide `px_opendir`, `px_readdir` and
   `px_closedir` wrappers in `libposix`.
3. **Atomic file operations** – design a transaction mechanism to guarantee
   consistency when multiple updates occur.

## Phase VI – Networking Infrastructure

1. **Socket server** – implement a user-level network stack that receives IPC
   requests from the POSIX library's socket helpers (`posix_setsockopt`,
   `posix_getsockopt`).
2. **IPv4 utilities** – supply address conversion helpers and error handling
   consistent with the POSIX specification.

## Phase VII – Threading and Concurrency

1. **Process-based threads** – offer a lightweight `pthread` API built on the
   process and scheduler servers.
2. **Synchronization primitives** – expose mutexes and condition variables as
   capability-protected objects managed in user space.
3. **Documentation** – describe the limitations of the process-based threading
   model in a new `docs/threading.md` guide.

## Phase VIII – Timing Facilities

1. **Timer subsystem** – add kernel hooks for high precision sleeps and expose
   them through `px_nanosleep` and `k_nanosleep` wrappers.
2. **Accounting** – track per-process timer usage to prevent abuse.

## Phase IX – Testing and Validation Framework

1. **Modular test suite** – extend the existing `tests/` directory with a
   dependency-aware runner that exercises each subsystem.
2. **Integration tests** – combine servers and libraries into end-to-end
   scenarios using the `kickstart` utility described in the README.
3. **POSIX conformance** – consult the specification copies under
   `docs/ben-books` when validating behaviour.

## Phase X – Deployment and Examples

1. **Example applications** – add small demo programs under `user/apps` showing
   how to interact with the new APIs.
2. **Deployment tooling** – script the launch of servers and demos with
   `kickstart` so that newcomers can reproduce a minimal POSIX environment.

This phased plan builds upon Pistachio's existing exokernel structure.  By
keeping policy in user space and relying on capability checks throughout, the
system can eventually support a broad subset of POSIX while preserving the
kernel's minimal design.

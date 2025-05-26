# Migration to an Exokernel Design

This document sketches the high level plan for transforming Pistachio's existing microkernel into a more minimal exokernel. The intent is to keep the kernel as small as possible while still supplying enough hooks to implement system functionality in user space.

The source tree uses modern language features.  All C++ code now targets the
C++23 standard.

## Goals

- Reduce kernel complexity by stripping all but the most fundamental mechanisms from the trusted computing base.
- Expose low level hardware access so that userland services can implement custom policies.
- Preserve existing ABI compatibility where feasible to ease the transition.
- Provide clear boundaries between what the kernel guarantees and what is delegated to user code.

## Minimal Kernel Responsibilities

Only the following duties remain inside the kernel:

- **CPU scheduling hooks**: primitives that allow user space schedulers to control which thread runs next while the kernel enforces safe context switches.
- **Memory protection**: establishment and enforcement of address spaces so that user code cannot corrupt the kernel or other tasks.
- **Low‑level hardware multiplexing**: interrupt dispatch, MMU setup and maintenance, and safe access to privileged hardware features.
- **Basic IPC stubs**: minimal message passing facilities used by user servers to implement richer communication models.

The kernel itself performs no policy decisions beyond protecting resources and multiplexing the processor and memory.

## Userland Services and Libraries

Functionality traditionally located in the microkernel will move to userland components. Examples include:

- **Process management**: creation and destruction of processes or tasks, implemented as a user level server.
- **Detailed thread scheduling**: complex scheduling strategies live entirely in user space libraries or daemons.
- **Device drivers and filesystems**: all I/O management is performed by separate servers with direct hardware access where necessary.
- **Networking stacks and protocol handling**: network layers run as libraries or dedicated processes using the kernel's minimal IPC support.

By delegating these services the system gains flexibility: different applications may link against or communicate with specialised libraries that implement the policies best suited for their workload.

## Memory Server

The memory server replaces the kernel's built‑in pager.  It receives
allocation and deallocation requests from applications and returns raw
memory frames.  All higher level allocators build on top of this
service.  The implementation in `engine/serv/memory` is intentionally
minimal to demonstrate how a user program can manage physical
resources using only the kernel's IPC primitives.

Build the server with::

    $ make -C engine/serv/memory

Once compiled it can be started with `kickstart` together with the
kernel image::

    $ engine/util/kickstart/kickstart -roottask=engine/serv/memory/memory

## Scheduler Server

Scheduling policies are likewise moved out of the kernel.  The
`engine/serv/scheduler` example provides a rudimentary round‑robin
scheduler that repeatedly calls `L4_ThreadSwitch` to hand the CPU to
other threads.  More advanced policies can be implemented in the same
way without enlarging the kernel.

Compile the scheduler server via::

    $ make -C engine/serv/scheduler

It may then be launched as an additional task using `kickstart`::

    $ engine/util/kickstart/kickstart \
          -roottask=engine/serv/memory/memory \
          engine/serv/scheduler/scheduler


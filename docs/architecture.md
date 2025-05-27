# Pistachio Architecture Overview

This document summarises the key design goals and structure of the Pistachio
exokernel.  It draws on existing documentation to provide a quick orientation
for new contributors.  Use the `scripts/inventory.py` utility to list directory
contents and locate the components referenced below.

## Exokernel Goals

The project aims to reduce kernel responsibilities to the bare minimum.  The
main objectives are:

- Reduce kernel complexity by stripping all but the most fundamental mechanisms
  from the trusted computing base.
- Expose low level hardware access so that userland services can implement
  custom policies.
- Preserve existing ABI compatibility where feasible to ease the transition.
- Provide clear boundaries between what the kernel guarantees and what is
  delegated to user code.

Only a few duties remain inside the kernel: scheduling hooks, memory protection,
low-level hardware multiplexing and basic IPC stubs.  All higher level services
live in user space.

## POSIX Compatibility

A small collection of servers and libraries implement enough of the POSIX
interface for standard applications.  The kernel offers only primitive threads,
IPC and address spaces.  User-level resource servers such as the memory and
scheduler managers provide functionality required by the POSIX subsystem.
Applications link against a syscall library that translates requests into IPC
messages exchanged with these servers.

## Build Systems

Pistachio retains its original Makefiles while also providing CMake and Meson
build descriptions.  The top level `Makefile`, `CMakeLists.txt` and
`meson.build` files allow the tree to be built with your preferred tool.  CMake
wraps the existing Makefiles whereas the Meson scripts demonstrate a more modern
native build.

## Source Tree Layout

- `engine/kernel` – kernel sources and configuration files.
- `engine/include/user/serv` – user-level servers such as `memory`, `scheduler`
  and `vfs`.
- `engine/include/user/lib` – user-space support libraries including the exokernel
  IPC helpers.
- `engine/include/user/util` – utilities like `kickstart` used to start the
  kernel and servers.

Run `scripts/inventory.py` from the repository root to display these directories
and others for easier navigation.

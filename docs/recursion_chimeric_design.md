# Recursion-Chimeric Kernel Architecture

This note explores how Pistachio's ongoing exokernel migration can blend recursion-based abstractions with microkernel-style message passing. The intent is to achieve a "chimeric" design: keeping the kernel minimal while allowing user code to construct higher level facilities by recursively composing small services.

## Overview

1. **Minimal core** – The kernel exposes just enough primitives for scheduling, memory protection and IPC.
2. **Recursive servers** – User-space components can themselves spawn lightweight servers that further delegate responsibilities, forming chains of decreasing trust.
3. **Chimeric layering** – Where performance is critical, services may link directly to one another, effectively fusing layers. Less demanding components communicate over the standard message interfaces.

## Goals

- Allow services to be composed and optimised recursively without expanding the kernel.
- Support experimentation with unconventional policies by mixing microkernel and exokernel techniques.
- Maintain POSIX compatibility through user-space libraries layered atop this design.

For a more complete description of the exokernel transition see [exokernel_migration.md](exokernel_migration.md).

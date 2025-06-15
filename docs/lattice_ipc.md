# Wait-For Graph Policy

Blocking IPC operations now participate in a kernel managed wait-for graph. Each
sender waiting on a receiver registers an edge from the sender to the target.
If adding the edge would create a cycle, the kernel aborts the IPC with the
`IPC_ERR_DEADLOCK` error code. When the wait completes or is aborted, the edge is
removed.

This conservative policy prevents classic circular wait deadlocks and keeps the
kernel's scheduling lattice acyclic.

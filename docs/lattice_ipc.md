# Lattice IPC

The `wormhole` subsystem allows lattice-based messages to be forwarded
between machines. A simple daemon is provided under
`engine/include/user/apps/wormhole`.

## Building the daemon

```bash
mkdir build && cd build
cmake .. && make wormhole_daemon
```

## Running

Start the daemon on a server machine:

```bash
./wormhole_daemon 5555
```

Clients can then connect to `tcp://<server>:5555` and exchange lattice
messages using the protocol implemented in `wormhole.cpp`.

# Wait-For Graph Policy

Blocking IPC operations now participate in a kernel managed wait-for graph. Each
sender waiting on a receiver registers an edge from the sender to the target.
If adding the edge would create a cycle, the kernel aborts the IPC with the
`IPC_ERR_DEADLOCK` error code. When the wait completes or is aborted, the edge is
removed.

This conservative policy prevents classic circular wait deadlocks and keeps the
kernel's scheduling lattice acyclic.


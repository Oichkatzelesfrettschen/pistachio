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

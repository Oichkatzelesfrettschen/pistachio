# MLP Scheduler

The `mlp_scheduler` server demonstrates using a small multi-layer
perceptron to choose the next runnable thread.  The implementation uses
the [Eigen](https://eigen.tuxfamily.org/) library for matrix operations.
Provide Eigen under `third_party/eigen` before building.

## Building

Clone Eigen into the `third_party` directory and build the library and
server:

```bash
$ git clone https://gitlab.com/libeigen/eigen.git third_party/eigen
$ make -C engine/lib/mlp
$ make -C engine/serv/mlp_scheduler
```

## Running

Launch the scheduler alongside the memory server with `kickstart`:

```bash
$ engine/util/kickstart/kickstart \
      -roottask=engine/serv/memory/memory \
      engine/serv/mlp_scheduler/mlp_scheduler
```

The scheduler loads a model file specified on the command line if
provided.

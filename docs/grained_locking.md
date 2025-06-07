# Fine-Grained Locking

Fine-grained locking splits a critical section into multiple smaller locks so
unrelated operations can proceed in parallel. This reduces contention when many
threads operate on different resources. In the codebase you can see fine-grained
strategies in the CLH lock implementation under `engine/include/smp/lock.h` and
within the readers–writers lock at `engine/include/include/pi/rwlock.c`.

Fine-grained locks contrast with coarse-grained approaches which use a single
lock to guard large subsystems. Coarse locking is easier to reason about but can
limit scalability when many processors compete for the same lock.

Key benefits of fine-grained locking include:

- **Improved concurrency** – operations that work on disjoint data can proceed
  simultaneously.
- **Better scalability** – avoids serialization bottlenecks on multi-core
  systems.
- **Reduced wait times** – threads block only when accessing the same specific
  resource.

Drawbacks are increased complexity and the potential for deadlocks when the
locks are acquired in inconsistent orders. To avoid these pitfalls, fine-grained
locks should use a well-defined locking hierarchy and be documented
comprehensively.

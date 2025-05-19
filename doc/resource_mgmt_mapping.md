# Resource Management Modules Mapping

This document lists modules in `kernel/src` that provide resource management services. The goal is to identify which components could potentially be relocated to user level to minimize the kernel's responsibilities.

| Module/File | Description | Suggested Location |
|-------------|-------------|-------------------|
| `generic/kmemory.cc`, `generic/kmemory.h` | Simple allocator for kernel memory. | Kernel (required for internal allocations) |
| `generic/mapping.cc`, `generic/mapping.h` | Mapping database management (page tracking). | Userland memory server |
| `generic/mapping_alloc.cc` | Buffer allocator for mapping database. | Userland memory server |
| `generic/mdb.cc`, `generic/mdb.h` | Mapping database implementation. | Userland memory server |
| `generic/mdb_mem.cc`, `generic/mdb_mem.h` | Memory-specific mapping database helpers. | Userland memory server |
| `generic/memregion.h` | Memory region descriptors. | Userland utilities |
| `generic/linear_ptab.h`, `generic/linear_ptab_walker.cc` | Page-table walking helpers. | Kernel |
| `generic/vrt.cc`, `generic/vrt.h` | Virtual resource table management. | Userland |
| `arch/*/mmu.h`, `arch/*/pgent.h`, `arch/*/ptab.h` | Architecture specific paging structures. | Kernel |
| `glue/v4-*/schedule.h` | Per‑architecture scheduler glue. | Userland scheduler |
| `api/v4/schedule.cc`, `api/v4/schedule.h` | Scheduler entry points. | Userland scheduler |
| `api/v4/sched-rr/*`, `api/v4/sched-hs/*` | Specific scheduling policies. | Userland scheduler |
| `glue/v4-*/space.cc`, `glue/v4-*/space.h` | Space and address‑space helpers. | Kernel (hardware specific) |
| `glue/v4-*/resources.cc`, `glue/v4-*/resources.h` | Thread resource bookkeeping (FPU, copy areas). | Kernel |


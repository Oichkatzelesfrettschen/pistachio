#pragma once

#include <stdint.h>
#include <sys/types.h>

/* Minimal Mach compatible types used by the modernised VM layer. */
typedef int            kern_return_t;
typedef uintptr_t      vm_offset_t;
typedef int            vm_prot_t;
typedef int            mach_port_t;

typedef struct { int dummy; } mutex_t;

#ifndef KERN_SUCCESS
#define KERN_SUCCESS 0
#endif

#ifndef KERN_FAILURE
#define KERN_FAILURE 1
#endif

/* Protection bits */
#ifndef VM_PROT_READ
#define VM_PROT_READ     0x1
#define VM_PROT_WRITE    0x2
#define VM_PROT_EXECUTE  0x4
#endif

/*
 * Address space descriptor used by the modernised VM layer.
 */
typedef struct aspace {
    vm_offset_t pml_root;  /* page map level root */
    mach_port_t pager_cap; /* capability for associated pager */
    mutex_t vm_lock;       /* protects mappings */
} aspace_t;

/* Information passed to the user level pager when a page fault occurs. */
typedef struct pf_info {
    vm_offset_t addr;  /* faulting address */
    vm_prot_t   prot;  /* access that caused the fault */
    int         flags; /* future use */
} pf_info_t;


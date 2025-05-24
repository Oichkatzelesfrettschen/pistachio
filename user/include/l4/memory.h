/*********************************************************************
 * Simple memory allocation API
 *********************************************************************/
#pragma once
#include <l4/types.h>
#include <l4/thread.h>

enum mem_opcode {
    MEM_ALLOC = 0,
    MEM_FREE  = 1,
};

struct mem_request {
    mem_opcode op;
    L4_Word_t  size;
    L4_Word_t  addr;
};

L4_Word_t memory_alloc(L4_ThreadId_t server, L4_Word_t size);
void memory_free(L4_ThreadId_t server, L4_Word_t addr, L4_Word_t size);
/**
 * Return the default memory server thread ID as reported by the kernel.
 */
L4_ThreadId_t memory_server(void);

/*********************************************************************
 * Simple memory allocation API
 *********************************************************************/
#pragma once
#include <l4/types.h>
#include <l4/thread.h>
#include <l4/compiler.h>

enum class mem_opcode : L4_Word_t {
    Alloc = 0,
    Free  = 1,
};

struct mem_request {
    mem_opcode op;
    L4_Word_t  size;
    L4_Word_t  addr;
};

[[nodiscard]] L4_Word_t L4_CDECL memory_alloc(L4_ThreadId_t server, L4_Word_t size);
void L4_CDECL memory_free(L4_ThreadId_t server, L4_Word_t addr, L4_Word_t size);
/**
 * Return the default memory server thread ID as reported by the kernel.
 */
[[nodiscard]] L4_ThreadId_t L4_CDECL memory_server(void);

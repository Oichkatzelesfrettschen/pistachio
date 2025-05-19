#include <types.h>

/* Default memory server thread ID: user task after root server */
static threadid_t memory_server_id;

extern "C" void *user_mem_alloc(word_t size)
{
    (void)size;
    /* TODO: perform IPC to memory_server_id and return allocated pointer */
    return nullptr;
}

extern "C" void user_mem_free(void *addr, word_t size)
{
    (void)addr; (void)size;
    /* TODO: send free request to memory_server_id */
}

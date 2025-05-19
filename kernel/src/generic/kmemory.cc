#include <kmemory.h>

extern "C" void *user_mem_alloc(word_t size);
extern "C" void user_mem_free(void *addr, word_t size);

/* Minimal kernel wrapper calling into user memory server */

kmem_t kmem;

SECTION(SEC_INIT) void kmem_t::init(void *start, void *end)
{
    (void)start; (void)end;
}

void kmem_t::free(void *address, word_t size)
{
    user_mem_free(address, size);
}

void *kmem_t::alloc(word_t size)
{
    return user_mem_alloc(size);
}

void *kmem_t::alloc_aligned(word_t size, word_t alignment, word_t mask)
{
    (void)alignment; (void)mask;
    return user_mem_alloc(size);
}

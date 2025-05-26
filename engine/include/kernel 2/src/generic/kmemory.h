#ifndef __KMEMORY_H__
#define __KMEMORY_H__

#include <types.h>

typedef word_t kmem_group_t;

class kmem_t
{
    void free(void *address, word_t size);
    void *alloc(word_t size);
    void *alloc_aligned(word_t size, word_t alignment, word_t mask);
public:
    void init(void *start, void *end);
    void free(kmem_group_t *, void *address, word_t size) { free(address, size); }
    void *alloc(kmem_group_t *, word_t size) { return alloc(size); }
    void *alloc_aligned(kmem_group_t *, word_t size, word_t alignment, word_t mask)
    { return alloc_aligned(size, alignment, mask); }
};

extern kmem_t kmem;

#endif

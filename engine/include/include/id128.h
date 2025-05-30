#pragma once
#include <stdint.h>

typedef unsigned _BitInt(128) id128_t;

static inline id128_t id128_from_u64(uint64_t v)
{
    return (id128_t)v;
}

static inline uint64_t id128_to_u64(id128_t v)
{
    return (uint64_t)v;
}

#ifndef CAPNP_HELPERS_H
#define CAPNP_HELPERS_H

#include <cstddef>
#include <cstdint>
#include <l4/memory.h>

namespace capnp {

static const size_t MEM_REQ_WORDS = 3;

inline void encode_mem_request(const mem_request &req, uint32_t *buf)
{
    buf[0] = static_cast<uint32_t>(req.op);
    buf[1] = static_cast<uint32_t>(req.size);
    buf[2] = static_cast<uint32_t>(req.addr);
}

inline bool decode_mem_request(const uint32_t *buf, size_t words,
                               mem_request &req)
{
    if (words < MEM_REQ_WORDS)
        return false;
    req.op = static_cast<mem_opcode>(buf[0]);
    req.size = buf[1];
    req.addr = buf[2];
    return true;
}

} // namespace capnp

#endif // CAPNP_HELPERS_H

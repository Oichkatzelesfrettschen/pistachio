#include <l4/message.h>
#include <l4/ipc.h>
#include <l4io.h>
#include <cstdlib>
#include <cstring>
#include <l4/memory.h>
#ifdef WITH_CAPNP
#include "../../lib/capnp/capnp.h"
#endif

static const L4_Word_t MEM_LABEL = 0;

static L4_Word_t handle_request(const mem_request &req)
{
    if (req.op == MEM_ALLOC) {
        void *ptr = std::malloc(req.size);
        return (L4_Word_t)ptr;
    } else if (req.op == MEM_FREE) {
        std::free((void*)req.addr);
        return 0;
    }
    return 0;
}

int main()
{
    printf("memory server started\n");
    while (1) {
        L4_Msg_t msg; L4_MsgTag_t tag;
        L4_Word_t label;
        L4_ThreadId_t partner;
        partner = L4_Wait(&tag);
        L4_StoreMR(0, &label);
        if (label != MEM_LABEL) {
            L4_Reply(partner);
            continue;
        }

        mem_request req;
#ifdef WITH_CAPNP
        L4_Word_t raw[capnp::MEM_REQ_WORDS];
        L4_StoreMRs(1, capnp::MEM_REQ_WORDS, raw);
        if (!capnp::decode_mem_request(raw, capnp::MEM_REQ_WORDS, req)) {
            L4_Reply(partner);
            continue;
        }
#else
        L4_StoreMRs(1, sizeof(req)/sizeof(L4_Word_t), (L4_Word_t*)&req);
#endif
        L4_Word_t res = handle_request(req);

        L4_MsgClear(&msg);
        L4_MsgAppendWord(&msg, res);
        L4_Set_MsgLabel(&msg, label);
        L4_MsgLoad(&msg);
        L4_Reply(partner);
    }
    return 0;
}

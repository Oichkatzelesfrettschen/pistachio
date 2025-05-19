#include <l4/message.h>
#include <l4/ipc.h>
#include <l4io.h>
#include <cstdlib>
#include <cstring>
#include <l4/memory.h>

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
        mem_request req;
        L4_StoreMRs(1, sizeof(req)/sizeof(L4_Word_t), (L4_Word_t*)&req);
        L4_Word_t res = handle_request(req);
        L4_MsgClear(&msg);
        L4_MsgAppendWord(&msg, res);
        L4_Set_MsgLabel(&msg, label);
        L4_MsgLoad(&msg);
        L4_Reply(partner);
    }
    return 0;
}

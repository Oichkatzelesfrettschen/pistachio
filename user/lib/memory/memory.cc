#include <l4/memory.h>
#include <l4/ipc.h>
#include <l4/message.h>
#include <l4/kip.h>

#include <stddef.h>


L4_Word_t memory_alloc(L4_ThreadId_t server, L4_Word_t size)
{
    mem_request req;
    req.op = MEM_ALLOC;
    req.size = size;
    req.addr = 0;

    L4_Msg_t msg;
    L4_MsgClear(&msg);
    L4_MsgAppendWord(&msg, 0);
    L4_MsgAppendWord(&msg, req.op);
    L4_MsgAppendWord(&msg, req.size);
    L4_MsgAppendWord(&msg, req.addr);
    L4_MsgLoad(&msg);
    L4_Call(server);
    L4_Word_t ret;
    L4_StoreMR(1, &ret);
    return ret;
}

void memory_free(L4_ThreadId_t server, L4_Word_t addr, L4_Word_t size)
{
    mem_request req;
    req.op = MEM_FREE;
    req.size = size;
    req.addr = addr;

    L4_Msg_t msg;
    L4_MsgClear(&msg);
    L4_MsgAppendWord(&msg, 0);
    L4_MsgAppendWord(&msg, req.op);
    L4_MsgAppendWord(&msg, req.size);
    L4_MsgAppendWord(&msg, req.addr);
    L4_MsgLoad(&msg);
    L4_Call(server);
}

L4_ThreadId_t memory_server(void)
{
    void *kip = L4_KernelInterface(nullptr, nullptr, nullptr);
    L4_Word_t ubase = L4_ThreadIdUserBase(kip);
    return L4_GlobalId(ubase + 3, 1); /* ROOT_VERSION */
}

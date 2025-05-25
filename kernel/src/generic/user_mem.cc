#include <types.h>
#include INC_API(tcb.h)
#include INC_API(kernelinterface.h)
#include INC_API(ipc.h)
#include <l4/memory.h>

/* Default memory server thread ID: user task after root server */
static threadid_t memory_server_id;

extern "C" void set_memory_server_id(threadid_t id)
{
    memory_server_id = id;
}

static void SECTION(".init") init_default_memory_server(void)
{
    threadid_t tid;
    tid.set_global_id(get_kip()->thread_info.get_user_base()+3, ROOT_VERSION);
    set_memory_server_id(tid);
}

extern "C" void *user_mem_alloc(word_t size)
{
    tcb_t *current = get_current_tcb();

    mem_request req;
    req.op = mem_opcode::Alloc;
    req.size = size;
    req.addr = 0;

    msg_tag_t tag;
    tag.set(0, 4, 0);
    current->set_tag(tag);
    current->set_mr(1, 0);
    current->set_mr(2, req.op);
    current->set_mr(3, req.size);
    current->set_mr(4, req.addr);

    tag = current->do_ipc(memory_server_id, memory_server_id, timeout_t::never());
    if (tag.is_error())
        return nullptr;
    return (void*)current->get_mr(1);
}

extern "C" void user_mem_free(void *addr, word_t size)
{
    tcb_t *current = get_current_tcb();

    mem_request req;
    req.op = mem_opcode::Free;
    req.size = size;
    req.addr = (word_t)addr;

    msg_tag_t tag;
    tag.set(0, 4, 0);
    current->set_tag(tag);
    current->set_mr(1, 0);
    current->set_mr(2, req.op);
    current->set_mr(3, req.size);
    current->set_mr(4, req.addr);

    current->do_ipc(memory_server_id, memory_server_id, timeout_t::never());
}

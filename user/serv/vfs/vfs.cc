#include <l4/ipc.h>
#include <stdio.h>
#include <unistd.h>
#include <vector>

enum class VfsLabel : L4_Word_t { Mkfifo = 1 };

struct FifoObj {
    int fds[2];
};

static std::vector<FifoObj> fifo_list;

static L4_Word_t handle_mkfifo(void)
{
    int fds[2];
    if (pipe(fds) < 0)
        return static_cast<L4_Word_t>(-1);
    fifo_list.push_back({{fds[0], fds[1]}});
    return fifo_list.size() - 1; /* capability */
}

int main()
{
    printf("VFS server started\n");
    while (1) {
        L4_ThreadId_t from;
        L4_MsgTag_t tag = L4_Wait(&from);
        L4_Word_t label = L4_Label(tag);
        L4_Word_t res = 0;

        if (label == static_cast<L4_Word_t>(VfsLabel::Mkfifo))
            res = handle_mkfifo();

        L4_Msg_t reply;
        L4_MsgClear(&reply);
        L4_MsgAppendWord(&reply, res);
        L4_Set_MsgLabel(&reply, label);
        L4_MsgLoad(&reply);
        L4_Reply(from);
    }
    return 0;
}

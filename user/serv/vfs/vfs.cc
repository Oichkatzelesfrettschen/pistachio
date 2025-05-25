#include <l4/ipc.h>
#include <l4/message.h>
#include <l4io.h>
#include <unordered_map>
#include <fcntl.h>

enum class VfsLabel : L4_Word_t { Request = 0x3456 };
enum class VfsOp : L4_Word_t { Open = 1, Close = 2 };

static std::unordered_map<int, int> fd_perms;
static int next_fd = 3;

static int perms_from_flags(int flags)
{
    int p = 0;
    if ((flags & O_RDWR) || (flags & O_RDONLY))
        p |= 1;
    if ((flags & O_RDWR) || (flags & O_WRONLY))
        p |= 2;
    return p;
}

int main()
{
    printf("VFS server started\n");
    while (1) {
        L4_MsgTag_t tag; L4_Msg_t msg; L4_ThreadId_t from;
        tag = L4_Wait(&from);
        L4_MsgStore(tag, &msg);

        if (L4_Label(tag) != static_cast<L4_Word_t>(VfsLabel::Request) ||
            L4_UntypedWords(tag) < 1) {
            L4_Reply(from);
            continue;
        }

        VfsOp op = static_cast<VfsOp>(L4_MsgWord(&msg, 0));

        switch (op) {
        case VfsOp::Open: {
            int flags = L4_UntypedWords(tag) > 1 ? L4_MsgWord(&msg, 1) : 0;
            int fd = next_fd++;
            fd_perms[fd] = perms_from_flags(flags);
            L4_MsgClear(&msg);
            L4_MsgAppendWord(&msg, fd);
            L4_Set_MsgLabel(&msg, static_cast<L4_Word_t>(VfsLabel::Request));
            L4_MsgLoad(&msg);
            L4_Reply(from);
            break; }
        case VfsOp::Close: {
            int fd = L4_UntypedWords(tag) > 1 ? L4_MsgWord(&msg, 1) : -1;
            fd_perms.erase(fd);
            L4_Reply(from);
            break; }
        default:
            L4_Reply(from);
            break;
        }
    }
    return 0;
}

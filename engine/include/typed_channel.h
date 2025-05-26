#ifndef TYPED_CHANNEL_H
#define TYPED_CHANNEL_H

#include <exo_ipc.h>
#include <l4/message.h>
#include <span>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    L4_ThreadId_t partner;
} typed_channel_t;

static inline void typed_channel_init(typed_channel_t *ch, L4_ThreadId_t partner)
{
    ch->partner = partner;
}

static inline exo_ipc_status typed_channel_send(typed_channel_t *ch,
                                                const L4_Word_t *words,
                                                size_t count)
{
    L4_Msg_t msg;
    L4_MsgPut(&msg, 0, count, words, 0, nullptr);
    L4_MsgLoad(&msg);
    return exo_send(ch->partner);
}

#ifdef __cplusplus
}
#endif

#endif /* TYPED_CHANNEL_H */

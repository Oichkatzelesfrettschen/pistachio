#pragma once

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>
#include "spinlock.h"

#define MAILBOX_BUFSZ 8

typedef struct {
    uint16_t len;
    uint16_t type;
    pid_t sender;
} exo_msg_hdr_t;

#define EXO_MSG_DATA_MAX 60

typedef struct {
    exo_msg_hdr_t hdr;
    unsigned char data[EXO_MSG_DATA_MAX];
} exo_msg_t;

#define EXO_SUCCESS   0
#define EXO_TIMEOUT  -1
#define EXO_OVERFLOW -2
#define EXO_INVALID  -3

struct mailbox {
    exo_msg_t buf[MAILBOX_BUFSZ];
    int head;
    int tail;
    spinlock_t lock;
};

void ipc_queue_init(struct mailbox *mb);
int kernel_ipc_queue_send(struct mailbox *mb, const exo_msg_t *msg);
int kernel_ipc_queue_recv_timed(struct mailbox *mb, exo_msg_t *out, unsigned int timeout_ms);

extern struct mailbox ipcs;

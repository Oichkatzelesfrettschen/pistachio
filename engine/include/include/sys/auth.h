#pragma once

#include <sys/types.h>
#include <sys/proc.h>

struct acl_entry {
    uid_t   ae_uid;
    const char *ae_op;
    int     ae_allow;
};

void acl_add(uid_t uid, const char *op, int allow);
int authorize(struct proc *p, const char *op);



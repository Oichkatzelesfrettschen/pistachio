#pragma once

#include <sys/proc.h>

void audit_record(struct proc *p, const char *op, int result);



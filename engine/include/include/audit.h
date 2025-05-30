#pragma once

#include <stdint.h>
#include "id128.h"

typedef struct {
    uint32_t op;
    id128_t obj;
    int result;
} audit_entry_t;

#define AUDIT_LOG_SIZE 32

extern audit_entry_t audit_log[AUDIT_LOG_SIZE];

void audit_record(uint32_t op, id128_t obj, int result);


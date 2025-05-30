#pragma once

#include <stdint.h>
#include "cap.h"
#include "id128.h"

typedef struct cap cap_t;

typedef struct {
    const cap_t *subject;
    uint32_t op;
    id128_t obj;
} acl_entry_t;

void acl_add(const cap_t *subject, uint32_t op, id128_t obj);
int authorize(const cap_t *subject, uint32_t op, id128_t obj);

#define CAP_OP_REFINE 1
#define CAP_OP_REVOKE 2


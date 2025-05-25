#include "capnp.h"
#include <stdio.h>

int capnp_parse(const char *buf)
{
    /* This stub just prints the buffer and claims success. */
    if (!buf) {
        return -1;
    }
    printf("capnp_parse: %s\n", buf);
    return 0;
}

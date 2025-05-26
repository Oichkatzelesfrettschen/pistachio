#ifndef __GENERIC__DEVICE_AREA_H__
#define __GENERIC__DEVICE_AREA_H__

#include <types.h>

INLINE addr_t device_area_start()
{
#ifdef DEVICE_AREA_START
    return (addr_t)DEVICE_AREA_START;
#else
    return 0;
#endif
}

#endif /* !__GENERIC__DEVICE_AREA_H__ */

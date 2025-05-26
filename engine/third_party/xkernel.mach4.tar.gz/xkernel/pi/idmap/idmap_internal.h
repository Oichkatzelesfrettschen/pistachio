/* 
 * idmap_internal.h
 *
 * x-kernel v3.2
 *
 * Copyright (c) 1993,1991,1990  Arizona Board of Regents
 *
 *
 * $Revision: 1.7 $
 * $Date: 1993/07/12 23:19:45 $
 */

#define	GETMAPELEM(map, elem) { \
    if ((elem = (map)->freelist) == 0) { \
	elem = (MapElement*)xMalloc(ROUND4 \
	    (ROUND4(sizeof(MapElement))+(map)->keySize)); \
        elem->externalid = (VOID *)(ROUND4((unsigned int)elem+sizeof(MapElement))); \
    } else { \
        (map)->freelist = (elem)->next; \
    } \
}

#define	GETVARMAPELEM(map, elem, size) { \
    if ((elem = (map)->freelist) == 0) { \
        elem = (MapElement*)xMalloc(ROUND4 \
		(ROUND4(sizeof(MapElement))+size)); \
        elem->externalid = (VOID *)(ROUND4((unsigned int)elem+sizeof(MapElement))); \
    } else { \
        (map)->freelist = (elem)->next; \
    } \
}

#define	FREEIT(map, elem) { \
    (elem)->next = (map)->freelist; \
    (map)->freelist = (elem); \
}

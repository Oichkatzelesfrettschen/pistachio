/* 
 * idmap.h
 *
 * x-kernel v3.2
 *
 * Copyright (c) 1993,1991,1990  Arizona Board of Regents
 *
 *
 * $Revision: 1.21 $
 * $Date: 1993/10/02 00:01:54 $
 */

#ifndef idmap_h
#define idmap_h


#define MAX_MAP_KEY_SIZE	100


typedef struct mapelement {
  struct mapelement *next;
  int  elmlen;
  VOID *internalid;
  VOID *externalid;
} MapElement, *Bind;

#define ERR_BIND	((Bind) XK_FAILURE)

typedef struct map	*Map;


typedef xkern_return_t (* XMapResolveFunc)( Map, void *, void ** );
typedef Bind 	       (* XMapBindFunc)( Map, void *, void * );
typedef xkern_return_t (* XMapRemoveFunc)( Map, Bind );
typedef xkern_return_t (* XMapUnbindFunc)( Map, void * );



struct map {
  int tableSize, keySize;
  MapElement *cache;
  MapElement *freelist;
  MapElement **table;
  XMapResolveFunc resolve;
  XMapBindFunc bind;
  XMapRemoveFunc remove;
  XMapUnbindFunc unbind;
};


#define mapResolve(_map, _key, _valPtr) \
  (_map)->resolve(_map, (VOID *)_key, (VOID **)_valPtr)
#define mapBind(_map, _key, _value)   (_map)->bind(_map, (VOID *)_key, (VOID *)(_value))
#define mapRemoveBinding(_map, _binding)   (_map)->remove((_map), (_binding))
#define mapUnbind(_map, _key)   (_map)->unbind((_map), (VOID *)(_key))
#define mapKeySize(_map) ((_map)->keySize)

/* 
 * msgForEach return value flags
 */
#define MFE_CONTINUE	1
#define MFE_REMOVE	2


typedef	int (*MapForEachFun)( void *key, void *value, void *arg );

extern void 	mapClose( Map );
extern Map 	mapCreate( int tableSize, int keySize );
extern void	mapForEach( Map, MapForEachFun, void * );
extern xkern_return_t mapVarResolve(Map, VOID *, int, VOID **);
extern Bind	mapVarBind(Map, VOID *, int, VOID *);

/* 
 * map_init is not part of the public map interface.  It is to be
 * called once at xkernel initialization time.
 */
extern void	map_init( void );



#endif /* idmap_h */

/* 
 * idmap.c
 *
 * x-kernel v3.2
 *
 * Copyright (c) 1993,1991,1990  Arizona Board of Regents
 *
 *
 * $Revision: 1.36 $
 * $Date: 1993/10/02 00:02:15 $
 */

#include "upi.h"
#include "idmap_internal.h"
#include "assert.h"
#include "xk_debug.h"
#ifndef XKMACHKERNEL
#include "x_util.h"
#include "x_libc.h"
#endif XKMACHKERNEL

static struct {
    XMapResolveFunc	resolve;
    XMapBindFunc	bind;
    XMapUnbindFunc	unbind;
    XMapRemoveFunc	remove;
} map_functions[MAX_MAP_KEY_SIZE];
  
int	traceidmap;

#define MAX_MAP_SIZE 1024

#ifdef __STDC__
static int generichash( char *,  int ,  int );
static xkern_return_t	mgenericresolve( Map, VOID *, VOID ** );
static Bind 		mgenericbind( Map, VOID *, VOID * );
static xkern_return_t  	mgenericunbind( Map, VOID * );
static xkern_return_t  	mgenericremove( Map, Bind );
static xkern_return_t  	mgenericerror( Map, VOID *, VOID ** );
static void		removeList( MapElement * );

#else

static int generichash();
static xkern_return_t	mgenericresolve();
static Bind 		mgenericbind();
static xkern_return_t  	mgenericunbind();
static xkern_return_t  	mgenericremove();
static xkern_return_t  	mgenericerror();
#endif /* __STDC__ */

/*
 * Create and return a new map containing a table with nEntries entries 
 * mapping keys of size keySize to integers
 */

Map
mapCreate(nEntries, keySize)
    int nEntries;
    int keySize;
{
    register Map m;
    register int pow2 = 0;
    int entries = nEntries;
    
    xTrace1(idmap, TR_FULL_TRACE, "mapCreate called, %d entries", nEntries);
    xAssert(nEntries);
    while (nEntries) { nEntries >>= 1; pow2++; }
    xAssert(pow2<8*sizeof(int)-1);
    if (entries == 1<<(pow2-1)) nEntries = entries;
    else nEntries = 1<<pow2;
    xAssert(nEntries<=MAX_MAP_SIZE);

    xTrace1(idmap, TR_EVENTS, "mapCreate: actual map size %d\n", nEntries);
    if (keySize > MAX_MAP_KEY_SIZE) {
	return (Map)-1;
    }
    m = (Map)xMalloc(sizeof(*m));
    m->tableSize = nEntries;
    m->keySize = keySize;
    m->cache = 0;
    m->freelist = 0;
    m->table = (MapElement **)xMalloc(nEntries * sizeof(MapElement *));
    bzero((char *)m->table, nEntries * sizeof(MapElement *));
    if (keySize > 0) {
      if (map_functions[keySize].resolve != 0) {
	m->resolve = map_functions[keySize].resolve;
	m->bind    = map_functions[keySize].bind   ;
	m->unbind  = map_functions[keySize].unbind ;
	m->remove  = map_functions[keySize].remove;
      } else {
	m->resolve = mgenericresolve;
	m->bind    = mgenericbind   ;
	m->unbind  = mgenericunbind ;
	m->remove  = mgenericremove;
      }    
    }
    else {  /* this is a map with variable length external id's */
      m->resolve = mgenericerror;
      m->bind =  (XMapBindFunc)mgenericerror;
      m->unbind = mgenericunbind;
      m->remove = mgenericremove;
    }
    return m;
  }


static xkern_return_t mgenericerror( table, extptr, intptr )
     Map table;
     VOID *extptr;
     VOID **intptr;
{
  xTrace0(idmap, TR_ERRORS, "variable length map accessed via mapBind or mapResolve; use mapVarBind or mapVarResolve");
  Kabort("wrong map function");
  return XK_FAILURE;
}


Bind
mapVarBind ( table, ext, len, intern )
    Map table;
    register VOID *ext;
    int len;
    VOID *intern;
{
    Bind     elem_posn, new_elem, prev_elem, *table_posn;
    register char *o_ext;
    int		ind;
    
    ind = generichash((char *)ext, table->tableSize, len);
    table_posn = &table->table[ind];

    elem_posn = *table_posn;
    prev_elem = elem_posn;
    while (elem_posn != 0) {
	o_ext = elem_posn->externalid;
	if (!bcmp(o_ext, (char *)ext, len)) {
	    if (elem_posn->internalid == intern) {
		return(elem_posn);
	    } else {
		return(ERR_BIND);
	    }
	} else {
	    prev_elem = elem_posn;
	    elem_posn = elem_posn->next;
	}
    }
    
    /* Note: Memory allocated here that is not immediately freed  cjt */
    /*
     * Elements must be inserted at the end of the list rather than at the
     * beginning for the semantics mapForEach to work properly.
     */
    GETVARMAPELEM(table, new_elem, len);
    bcopy((char *)ext, (char *)new_elem->externalid, len);
    new_elem->internalid = intern;
    new_elem->next = 0;
    new_elem->elmlen = len;
    table->cache = new_elem;
    if ( prev_elem == 0 ) {
	*table_posn = new_elem;
    } else {
	prev_elem->next = new_elem;
    }
    return new_elem;
}


xkern_return_t
mapVarResolve ( table, ext, len, resPtr )
    Map table;
    register VOID *ext;
    int len;
    register VOID **resPtr;
{
    register Bind elem_posn;
    register char *o_ext;
    
    if ((elem_posn = table->cache) && elem_posn->elmlen == len) {
	o_ext = elem_posn->externalid;
	if (!bcmp(o_ext, (char *)ext, len)) {
	    if ( resPtr ) {
		*resPtr = elem_posn->internalid;
	    }
	    return XK_SUCCESS;
	}
    }
    elem_posn = table->table[generichash(ext, table->tableSize, len)];
    while (elem_posn != 0) {
	o_ext = elem_posn->externalid;
	if (elem_posn->elmlen == len &&
	    !bcmp(o_ext, (char *)ext, len)) {
	    table->cache = elem_posn;
	    if ( resPtr ) {
		*resPtr = elem_posn->internalid;
	    }
	    return XK_SUCCESS;
	} else {
	    elem_posn = elem_posn->next;
	}
    }
    return XK_FAILURE;
}

static void
removeList( e )
    MapElement	*e;
{
    MapElement	*next;

    while ( e != 0 ) {
	xTrace1(idmap, TR_FULL_TRACE, "mapClose removing element %x", (int)e);
	next = e->next;
	xFree((char *)e);
	e = next;
    }
}


void
mapClose(m)
    Map m;
{
    int		i;

    xTrace1(idmap, TR_MAJOR_EVENTS, "mapClose of map %x", m);
    /* 
     * Free any elements still bound into the map
     */
    for ( i=0; i < m->tableSize; i++ ) {
	removeList(m->table[i]);
    }
    /* 
     * Remove freelist
     */
    removeList(m->freelist);
    xFree((char *)m->table);
    xFree((char *)m);
}


void
mapForEach(m, f, arg)
    Map m;
    MapForEachFun f;
    VOID *arg;
{
    char	key[MAX_MAP_KEY_SIZE];
    MapElement	*elem, *prevElem;
    int 	i;
    int		userResult = 0;

    xTrace0(idmap, TR_FULL_TRACE, "mapForEach called");
    for ( i = 0; i < m->tableSize; i++ ) {
	prevElem = 0;
	do {
	    /* 
	     * Set elem to first item in the bucket
	     */
	    elem = m->table[i];
	    if ( prevElem != 0 ) {
		/*
		 * Set elem to the next element after the old elem in the
		 * same hash bucket.  If the previous element has been
		 * removed we will skip the rest of this bucket.  If
		 * the previous element has been removed and
		 * reinserted (with a possibly different key/value),
		 * we will skip it and everything before it in the
		 * list. 
		 */
		for ( ; elem != 0 ; elem = elem->next ) {
		    if ( prevElem == elem ) {
			elem = elem->next;
			break;
		    }
		}
		if ( userResult & MFE_REMOVE ) {
		    m->remove(m, prevElem);
		}
		prevElem = 0;
	    }
	    if ( elem != 0 ) {
		bcopy((char *)elem->externalid, key, m->keySize);
		userResult = f(key, elem->internalid, arg);
		if ( ! (userResult & MFE_CONTINUE) ) {
		    return;
		}
		prevElem = elem;
	    }
	} while ( elem != 0 );
    } 
}


#ifdef XK_USE_INLINE
#  define INLINE	__inline__
#else
#  define INLINE
#endif

#ifdef __STDC__

static INLINE int	hash2( void *, int );
static INLINE int	hash4( void *, int );
static INLINE int	hash6( void *, int );
static INLINE int	hash8( void *, int );
static INLINE int	hash10( void *, int );
static INLINE int	hash12( void *, int );
static INLINE int	hash16( void *, int );

#endif /* __STDC__ */


#define GENCOMPBYTES(s1, s2) (!bcmp(s1, s2, table->keySize))
#define GENCOPYBYTES(s1, s2) bcopy(s2, s1, table->keySize)

#define KEYSIZE 2
#define BINDNAME m2bind
#define REMOVENAME m2remove
#define RESOLVENAME m2resolve
#define UNBINDNAME m2unbind
static INLINE int
hash2(K, tblSize)
    VOID	*K;
    int		tblSize;
{
    u_int	h = 0;
    u_char      h0, h1;

    h0 = *(u_char *)K;
    h1 = *((u_char *)K+1);
    h = (((h0<<2) + h1) ^ ((h1<<2) + h0)) % tblSize;
    xTrace1(idmap, TR_FULL_TRACE, "2 byte key optimized hash returns %d",
	    h);
    return h;
}
#define HASH(K, tblSize, keySize)        hash2(K, tblSize)
#define COMPBYTES(s1, s2) (((char *)(s1))[0] == ((char *)(s2))[0] && 	\
			   ((char *)(s1))[1] == ((char *)(s2))[1])
#define COPYBYTES(s1, s2) {((char *)(s1))[0] = ((char *)(s2))[0];	\
			   ((char *)(s1))[1] = ((char *)(s2))[1];}


#include "idmap_templ.c"

#define KEYSIZE 4
#define BINDNAME m4bind
#define REMOVENAME m4remove
#define RESOLVENAME m4resolve
#define UNBINDNAME m4unbind
static INLINE int
hash4(K, tblSize)
    VOID	*K;
    int		tblSize;
{
    register u_char	h0=*(u_char *)K,
                        h1=*((u_char *)K+1),
                        h2=*((u_char *)K+2),
			h3=*((u_char *)K+3);
    u_int	h;
#ifdef XK_DEBUG
    unsigned long k = h0|(h1<<8)|(h2<<16)|(h3<<24);
#endif

    h = ((h0+(h2<<2)) ^ (h1+(h3<<1)) ^ (h2+(h0<<1)) ^ (h3+h1<<2)) % tblSize;

    xTrace4(idmap, TR_DETAILED, "4 byte key optimized hash of %x (at %x) returns %d in table size %d",
	    k, (K), h, tblSize);
    return h;
  }

#define HASH(K, tblSize, keySize)        hash4(K, tblSize)

#define COMPBYTES(s1, s2) 				\
  ( (LONG_ALIGNED(s1) && LONG_ALIGNED(s2)) ? 		\
   ( *(int *)(s1) == (*(int *)(s2))) :			\
   GENCOMPBYTES((s1), (s2)) )

#define COPYBYTES(s1, s2) {				\
  if ( LONG_ALIGNED(s1) && LONG_ALIGNED(s2) ) {		\
    	*(int *)(s1) = (*(int *)(s2)); 			\
  } else {						\
	GENCOPYBYTES(s1,s2);				\
  }							\
}

#include "idmap_templ.c"

#define KEYSIZE 6
#define BINDNAME m6bind
#define REMOVENAME m6remove
#define RESOLVENAME m6resolve
#define UNBINDNAME m6unbind
static INLINE int
hash6(K, tblSize)
    VOID	*K;
    int		tblSize;
{
    u_int	h = 0;
    register    u_char	*k = (u_char *)K;

    h = hash4(k,tblSize) ^ hash2(k+4,tblSize);
    
    xTrace1(idmap, TR_FULL_TRACE, "6 byte key optimized hash returns %d",
		h);
    return h;
}

#define HASH(K, tblSize, keySize) 	hash6(K, tblSize)

#define COMPBYTES(s1, s2) 				\
  ( (LONG_ALIGNED(s1) && LONG_ALIGNED(s2)) ? 		\
   ( (*(int *)(s1)) == (*(int *)(s2)) &&		\
     (*((short *)(s1)+2)) == (*((short *)(s2)+2)) ) :	\
   GENCOMPBYTES((s1), (s2)) )

#define COPYBYTES(s1, s2) {				\
  if ( LONG_ALIGNED(s1) && LONG_ALIGNED(s2) ) {		\
    	*(int *)(s1) = (*(int *)(s2)); 			\
	*((short *)(s1)+2) = *((short *)(s2)+2); 	\
  } else {						\
	GENCOPYBYTES(s1,s2);				\
  }							\
}


#include "idmap_templ.c"

#define KEYSIZE 8
#define BINDNAME m8bind
#define REMOVENAME m8remove
#define RESOLVENAME m8resolve
#define UNBINDNAME m8unbind
static INLINE int
hash8( K, tblSize )
    VOID	*K;
    int		tblSize;
{
    u_int	h;
    register    u_char	*k = (u_char *)K;

    h = hash4(k,tblSize) ^ hash4(k+4,tblSize);

    xTrace1(idmap, TR_FULL_TRACE, "8 byte key optimized hash returns %d",
		h);
    return h;
}
#define HASH(K, tblSize, keySize)	hash8(K, tblSize)

#define COMPBYTES(s1, s2) 				\
  ( (LONG_ALIGNED(s1) && LONG_ALIGNED(s2)) ? 		\
       ((*(int *)(s1)) == (*(int *)(s2)) && 	    	\
	 (*((int *)(s1)+1)) == (*((int *)(s2)+1))) :	\
    GENCOMPBYTES(s1, s2) )
	  

#define COPYBYTES(s1, s2) {			\
  if ( LONG_ALIGNED(s1) && LONG_ALIGNED(s2) ) {	\
    	*(int *)(s1) = (*(int *)(s2)); 		\
	*((int *)(s1)+1) = *((int *)(s2)+1); 	\
  } else {					\
	GENCOPYBYTES(s1,s2);			\
  }						\
}


#include "idmap_templ.c"

#define KEYSIZE 10
#define BINDNAME m10bind
#define REMOVENAME m10remove
#define RESOLVENAME m10resolve
#define UNBINDNAME m10unbind
static INLINE int
hash10(K, tblSize)
    VOID	*K;
    int		tblSize;
{
    u_int	h;
    register    u_char	*k = (u_char *)K;

    h = hash4(k,tblSize) ^ hash4(k+4,tblSize) ^ hash2(k+8,tblSize);
    
    xTrace1(idmap, TR_FULL_TRACE, "10 byte key optimized hash returns %d",
	    h);
    return h;
}
#define HASH(K, tblSize, keySize) 	hash10(K, tblSize)

#define COMPBYTES(s1, s2) 				\
  ( (LONG_ALIGNED(s1) && LONG_ALIGNED(s2)) ? 		\
   ( (*(int *)(s1)) == (*(int *)(s2)) &&		\
     (*((int *)(s1)+1)) == (*((int *)(s2)+1)) && 	\
     (*((short *)(s1)+4)) == (*((short *)(s2)+4)) ) :	\
   GENCOMPBYTES((s1), (s2)) )

#define COPYBYTES(s1, s2) {				\
  if ( LONG_ALIGNED(s1) && LONG_ALIGNED(s2) ) {		\
    	*(int *)(s1) = (*(int *)(s2)); 			\
	*((int *)(s1)+1) = *((int *)(s2)+1); 		\
	*((short *)(s1)+4) = *((short *)(s2)+4); 	\
  } else {						\
	GENCOPYBYTES(s1,s2);				\
  }							\
}

#include "idmap_templ.c"



#define KEYSIZE 12
#define BINDNAME m12bind
#define REMOVENAME m12remove
#define RESOLVENAME m12resolve
#define UNBINDNAME m12unbind
static INLINE int
hash12( K, tblSize )
    VOID	*K;
    int		tblSize;
{
    u_int	h;
    register    u_char	*k = (u_char *)K;

    h = hash4(k,tblSize) ^ hash4(k+4,tblSize) ^ hash4(k+8,tblSize);
    xTrace1(idmap, TR_FULL_TRACE, "12 byte key optimized hash returns %d",
	    h);
    return h;
}	
#define HASH(K, tblSize, keySize)	hash12(K, tblSize)


#define COMPBYTES(s1, s2) \
  ( (LONG_ALIGNED(s1) && LONG_ALIGNED(s2)) ? 		\
       ((*(int *)(s1)) == (*(int *)(s2)) && 	    	\
	(*((int *)(s1)+1)) == (*((int *)(s2)+1)) && 	\
	(*((int *)(s1)+2)) == (*((int *)(s2)+2))) : 	\
    GENCOMPBYTES(s1, s2) )
	  

#define COPYBYTES(s1, s2) {			\
  if ( LONG_ALIGNED(s1) && LONG_ALIGNED(s2) ) {	\
    	*(int *)(s1) = (*(int *)(s2)); 		\
	*((int *)(s1)+1) = *((int *)(s2)+1); 	\
	*((int *)(s1)+2) = *((int *)(s2)+2); 	\
  } else {					\
	GENCOPYBYTES(s1,s2);			\
  }						\
}

#include "idmap_templ.c"


#define KEYSIZE 16
#define BINDNAME m16bind
#define REMOVENAME m16remove
#define RESOLVENAME m16resolve
#define UNBINDNAME m16unbind
static INLINE int
hash16( K, tblSize )
    VOID	*K;
    int		tblSize;
{
    u_int	h;
    register    u_char	*k = (u_char *)K;

    h = hash4(k,tblSize) ^ hash4(k+4,tblSize) ^ hash4(k+8,tblSize)
        ^ hash4(k+12,tblSize);
    xTrace1(idmap, TR_FULL_TRACE, "16 byte key optimized hash returns %d",
		h);
	return h;
}	
#define HASH(K, tblSize, keySize)	hash16(K, tblSize)


#define COMPBYTES(s1, s2) \
  ( (LONG_ALIGNED(s1) && LONG_ALIGNED(s2)) ? 		\
       ((*(int *)(s1)) == (*(int *)(s2)) && 	    	\
	(*((int *)(s1)+1)) == (*((int *)(s2)+1)) && 	\
	(*((int *)(s1)+2)) == (*((int *)(s2)+2)) && 	\
	(*((int *)(s1)+3)) == (*((int *)(s2)+3))) : 	\
    GENCOMPBYTES(s1, s2) )
	  

#define COPYBYTES(s1, s2) {			\
  if ( LONG_ALIGNED(s1) && LONG_ALIGNED(s2) ) {	\
    	*(int *)(s1) = (*(int *)(s2)); 		\
	*((int *)(s1)+1) = *((int *)(s2)+1); 	\
	*((int *)(s1)+2) = *((int *)(s2)+2); 	\
	*((int *)(s1)+3) = *((int *)(s2)+3); 	\
  } else {					\
	GENCOPYBYTES(s1,s2);			\
  }						\
}

#include "idmap_templ.c"

static int
generichash(key, tableSize, keySize)
    char *key;
    int tableSize;
    int keySize;
{
    unsigned int	hash = 0;
    int	howmanylongs = keySize / 4;

    xTrace2(idmap, TR_FULL_TRACE,
	    "generic idmap hash -- tableSize %d, keySize %d",
	    tableSize, keySize);
    while (howmanylongs--) {
      hash ^= hash4(key, tableSize);
      key += 4;
    }
    if (keySize & 3) {
      if (!((unsigned int)key & 0x2)) { hash ^= hash2(key, tableSize); key+= 2;}
      if (!((unsigned int)key & 0x1))  hash ^=  *(u_char *)key;
    }
    hash &= (tableSize -1);
    xTrace1(idmap, TR_FULL_TRACE, "generic hash returns %d", hash % tableSize);
    return hash % tableSize;
}

#define KEYSIZE generic
#define BINDNAME mgenericbind
#define REMOVENAME mgenericremove
#define RESOLVENAME mgenericresolve
#define UNBINDNAME mgenericunbind
#define HASH(K, tblSize, keySize) generichash(K, tblSize, keySize)
#define GENHASH(K, T, keySize) generichash(K, tblSize, keySize)
#define COMPBYTES(s1, s2) (!bcmp(s1, s2, table->keySize))
#define COPYBYTES(s1, s2) bcopy(s2, s1, table->keySize)

#include "idmap_templ.c"

void
map_init()
{
#define KEYSIZE 2
#define BINDNAME m2bind
#define REMOVENAME m2remove
#define RESOLVENAME m2resolve
#define UNBINDNAME m2unbind
#include "idmap_init.c"
#define KEYSIZE 4
#define BINDNAME m4bind
#define REMOVENAME m4remove
#define RESOLVENAME m4resolve
#define UNBINDNAME m4unbind
#include "idmap_init.c"
#define KEYSIZE 6
#define BINDNAME m6bind
#define REMOVENAME m6remove
#define RESOLVENAME m6resolve
#define UNBINDNAME m6unbind
#include "idmap_init.c"
#define KEYSIZE 8
#define BINDNAME m8bind
#define REMOVENAME m8remove
#define RESOLVENAME m8resolve
#define UNBINDNAME m8unbind
#include "idmap_init.c"
#define KEYSIZE 10
#define BINDNAME m10bind
#define REMOVENAME m10remove
#define RESOLVENAME m10resolve
#define UNBINDNAME m10unbind
#include "idmap_init.c"
#define KEYSIZE 12
#define BINDNAME m12bind
#define REMOVENAME m12remove
#define RESOLVENAME m12resolve
#define UNBINDNAME m12unbind
#include "idmap_init.c"
#define KEYSIZE 16
#define BINDNAME m16bind
#define REMOVENAME m16remove
#define RESOLVENAME m16resolve
#define UNBINDNAME m16unbind
#include "idmap_init.c"
}

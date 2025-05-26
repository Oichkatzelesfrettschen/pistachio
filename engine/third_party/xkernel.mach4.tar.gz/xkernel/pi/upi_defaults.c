/* 
 * upi_defaults.c
 *
 * x-kernel v3.2
 *
 * Copyright (c) 1993,1991,1990  Arizona Board of Regents
 *
 *
 * $Revision: 1.11 $
 * $Date: 1993/07/16 03:18:31 $
 */

#include "xkernel.h"

extern int	traceprotocol;
int	traceprotdef = 0;


#define savePart(pxx, spxx)	\
    bcopy((char *)(pxx), (char *)(spxx), sizeof(Part) * partLen(p))

#define restorePart(pxx, spxx)	\
    bcopy((char *)(spxx), (char *)(pxx), sizeof(Part) * partLen(p))

/* 
 * Check for a valid participant list
 */
#define partCheck(pxx, name, retval)					\
	{								\
	  if ( ! (pxx) || partLen(pxx) < 1 ) {				\
		xTrace1(protdef, TR_ERRORS,				\
			"xDefault %s -- bad participants",		\
			(name));  					\
		return (retval);					\
	  }								\
	}


#ifdef __STDC__

static xkern_return_t	llpOpenDisable(XObj, XObj, XObj, Part *, XObj *, int);

#else

static xkern_return_t  llpOpenDisable();

#endif


xkern_return_t
defaultOpenEnable( map, hlpRcv, hlpType, key )
    Map		map;
    XObj   	hlpRcv, hlpType;
    VOID	*key;
{
    Enable	*e;
    
    xAssert(map);
    xAssert(key);
    xAssert(xIsXObj(hlpRcv));
    xAssert(xIsXObj(hlpType));
    if ( mapResolve(map, key, &e) == XK_FAILURE ) {
	xTrace0(protdef, TR_MORE_EVENTS,
		"openenable -- creating new enable obj");
	e = X_NEW(Enable);
	e->rcnt = 1;
	e->hlpRcv = hlpRcv;
	e->hlpType = hlpType;
	e->binding =  mapBind(map, (char *)key, (int)e);
	if ( e->binding == ERR_BIND ) {
	    xTrace0(protdef, TR_ERRORS,
		    "openenable -- binding failed!");
	    xFree((char *)e);
	    return XK_FAILURE;
	}
    } else {
	xTrace0(protdef, TR_MORE_EVENTS, "openenable -- obj exists");
	if ( e->hlpRcv != hlpRcv || e->hlpType != hlpType ) {
	    xTrace0(protdef, TR_SOFT_ERRORS, "openenable -- hlp mismatch");
	    xTrace4(protdef, TR_SOFT_ERRORS, 
		    "(existing rcv: %x  type %x, paramater rcv: %x  type %x",
		    e->hlpRcv, e->hlpType, hlpRcv, hlpType);
	    return XK_FAILURE;
	}
	e->rcnt++;
	xTrace1(protdef, TR_MORE_EVENTS,
		"openenable -- increasing enable obj ref count to %d",
		e->rcnt);
    }
    return XK_SUCCESS;
}


xkern_return_t
defaultOpenDisable( map, hlpRcv, hlpType, key )
    Map		map;
    XObj   	hlpRcv, hlpType;
    VOID	*key;
{
    Enable	*e;
    
    xAssert(map);
    xAssert(key);
    xAssert(xIsXObj(hlpRcv));
    xAssert(xIsXObj(hlpType));
    if ( mapResolve(map, key, &e) == XK_FAILURE ) {
	xTrace0(protdef, TR_SOFT_ERRORS,
		"opendisable -- no enable obj found");
	return XK_FAILURE;
    }
    if (  e->hlpRcv != hlpRcv || e->hlpType != hlpType ) {
	    xTrace0(protdef, TR_SOFT_ERRORS, "opendisable -- hlp mismatch");
	    xTrace4(protdef, TR_SOFT_ERRORS, 
		    "(existing rcv: %x  type %x, paramater rcv: %x  type %x",
		    e->hlpRcv, e->hlpType, hlpRcv, hlpType);
	return XK_FAILURE;
    }
    e->rcnt--;
    xTrace1(protdef, TR_MORE_EVENTS,
	    "opendisable -- reducing enable obj ref count to %d",
		e->rcnt);
    if ( e->rcnt == 0 ) {
	xTrace0(protdef, TR_MORE_EVENTS,
		"opendisable -- removing enable object");
	mapRemoveBinding(map, e->binding);
	xFree((char *)e);
    }
    return XK_SUCCESS;
}


typedef struct {
    Enable	*e;
    char	key[MAX_MAP_KEY_SIZE];
    int		keySize;
    XObj	hlpRcv;
} FindStuff;


static int
findHlpEnable( key, val, arg )
    VOID	*key, *val, *arg;
{
    FindStuff	*fs = (FindStuff *)arg;
    Enable	*e = (Enable *)val;

    if ( e->hlpRcv == fs->hlpRcv ) {
	fs->e = e;
	bcopy(key, fs->key, fs->keySize);
	return FALSE;
    } else {
	return TRUE;
    }
}


xkern_return_t
defaultOpenDisableAll( map, hlpRcv, f )
    Map			map;
    XObj		hlpRcv;
    DisableAllFunc	f;
{
    FindStuff		fs;
    xkern_return_t	xkr;

    xTrace0(protdef, TR_FUNCTIONAL_TRACE, "openDisableAll");
    fs.hlpRcv = hlpRcv;
    fs.keySize = mapKeySize(map);
    for ( ;; ) {
	fs.e = 0;
	/* 
	 * Restrictions against modifying a map during mapForEach make this awkward ... 
	 */
	mapForEach(map, findHlpEnable, &fs);
	if ( fs.e == 0 ) break;
	xTrace2(protdef, TR_ALWAYS,
		"defaultOpenDisableAll removes binding (rcv %s, type %s)",
		fs.e->hlpRcv->fullName, fs.e->hlpType->fullName);
	if ( f ) {
	    f(fs.key, fs.e);
	}
	xkr = mapRemoveBinding(map, fs.e->binding);
	xAssert(xkr == XK_SUCCESS);
	xFree((char *)fs.e);
    }
    return XK_SUCCESS;
}


xkern_return_t
defaultVirtualOpenEnable( self, map, hlpRcv, hlpType, llp, p )
    Map		map;
    XObj	self, hlpRcv, hlpType, *llp;
    Part	*p;
{
    Part		*sp;
    int			i;
    xkern_return_t	xkr;
    
    xAssert(map);
    xAssert(xIsXObj(self));
    xAssert(xIsXObj(hlpRcv));
    xAssert(xIsXObj(hlpType));
    xTrace0(protdef, TR_MAJOR_EVENTS, "default virtual open enable");
    partCheck(p, "openEnable", XK_FAILURE);
    if ( defaultOpenEnable(map, hlpRcv, hlpType,
			   (VOID *)&hlpType) == XK_FAILURE ) {
	return XK_FAILURE;
    }
    xkr = XK_SUCCESS;
    sp = (Part *)xMalloc(sizeof(Part) * partLen(p));
    savePart(p, sp);
    for ( i=0; llp[i]; i++ ) {
	restorePart(p, sp);
	if ( ! xIsProtocol(llp[i]) || 
		xOpenEnable(self, hlpType, llp[i], p) == XK_FAILURE ) {
	    xTrace1(protdef, TR_SOFT_ERRORS,
		    "could not openEnable llp %s", llp[i]->name);
	    llpOpenDisable(self, hlpRcv, hlpType, sp, llp, i);
	    xkr = XK_FAILURE;
	    break;
	}
    }
    xFree((char *)sp);
    return xkr;
}


static xkern_return_t
llpOpenDisable( self, hlpRcv, hlpType, p, llp, i )
    XObj	self, hlpRcv, hlpType;
    XObj	*llp;
    Part	*p;
    int		i;
{
    Part		*sp;
    xkern_return_t	rv = XK_SUCCESS;

    sp = (Part *)xMalloc(sizeof(Part) * partLen(p));
    savePart(p, sp);
    for ( i--; i >= 0; i-- ) {
	restorePart(p, sp);
	if ( ! xIsProtocol(llp[i]) || 
		xOpenDisable(hlpRcv, hlpType, llp[i], p) == XK_FAILURE ) {
	    xTrace1(protdef, TR_SOFT_ERRORS,
		    "llpOpenDisable could not openDisable llp %s",
		    llp[i]->name);
	    rv = XK_FAILURE;
	}
    }
    xFree((char *)sp);
    return rv;
}


xkern_return_t
defaultVirtualOpenDisable( self, map, hlpRcv, hlpType, llp, p )
    Map		map;
    XObj 	self, hlpRcv, hlpType, *llp;
    Part 	*p;
{
    int	i;

    xAssert(map);
    xAssert(xIsXObj(self));
    xAssert(xIsXObj(hlpRcv));
    xAssert(xIsXObj(hlpType));
    xTrace0(protdef, TR_MAJOR_EVENTS, "default virtual open disable");
    if ( defaultOpenDisable(map, hlpRcv, hlpType,
			    (VOID *)&hlpType) == XK_FAILURE ) {
	return XK_FAILURE;
    }
    for ( i=0; llp[i]; i++ );
    return llpOpenDisable(self, hlpRcv, hlpType, p, llp, i);
}


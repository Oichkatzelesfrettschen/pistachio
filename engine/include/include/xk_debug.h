/* 
 * xk_debug.h
 *
 * x-kernel v3.2
 *
 * Copyright (c) 1993,1991,1990  Arizona Board of Regents
 *
 *
 * $Revision: 1.29 $
 * $Date: 1993/10/26 20:05:43 $
 */

#ifndef xk_debug_h
#define xk_debug_h

#include "platform.h"
#include "x_stdio.h"
#include "trace.h"

extern int
	tracebuserror,
	tracecustom,
  	traceethdrv,
#ifdef XKMACHKERNEL
 	tracelance,
#endif XKMACHKERNEL
 	traceether,
	traceevent,
	tracefixme,
	traceidle,
  	traceidmap,
	traceie,
	traceinit,  
	tracememoryinit,
	tracemsg,  
	tracenetmask,
	traceprocesscreation,
	traceprocessswitch,
	traceprotocol,
	traceptbl,
  	traceprottest,
	tracetick,
	tracetrap,
  	tracelock,
  	tracespl,
	tracesessngc,
	traceuser;

extern	char errBuf[250];


#if ! defined(XK_DEBUG)
#    define NDEBUG
#endif



void	xTraceLock( void );
void	xTraceUnlock( void );
void	xTraceInit( void );



#if defined(XK_DEBUG) || defined(lint)

#define PRETRACE(l) { int __i=l; xTraceLock();  while(__i--) putchar(' '); }
#define POSTTRACE { putchar('\n'); xTraceUnlock(); }

#define XPASTE(X,Y) X##Y
#define PASTE(X,Y) XPASTE(X,Y)


#   define xIfTrace(t, l) \
	if (PASTE(trace,t) >= l)
#   define xTrace0(t, l, f) 		\
	do {				\
	    if (PASTE(trace,t) >= l) {	\
		PRETRACE(l);		\
	    	printf(f); 		\
	    	POSTTRACE;		\
	    }				\
	} while (0)
#   define xTrace1(t, l, f, arg1) 	\
	do {				\
	    if (PASTE(trace,t) >= l) {	\
		PRETRACE(l);		\
	    	printf(f, arg1);	\
	    	POSTTRACE;		\
	    }				\
	} while (0)
#   define xTrace2(t, l, f, arg1, arg2) \
	do {				\
	    if (PASTE(trace,t) >= l) {	\
		PRETRACE(l);		\
	    	printf(f, arg1, arg2);	\
	    	POSTTRACE;		\
	    }				\
	} while (0)
#   define xTrace3(t, l, f, arg1, arg2, arg3) \
	do {				\
	    if (PASTE(trace,t) >= l) {	\
		PRETRACE(l);		\
	    	printf(f, arg1, arg2, arg3);	\
	    	POSTTRACE;		\
	    }				\
	} while (0)
#   define xTrace4(t, l, f, arg1, arg2, arg3, arg4) \
	do {				\
	    if (PASTE(trace,t) >= l) {	\
		PRETRACE(l);		\
	    	printf(f, arg1, arg2, arg3, arg4);	\
	    	POSTTRACE;		\
	    }				\
	} while (0)
#   define xTrace5(t, l, f, arg1, arg2, arg3, arg4, arg5) \
	do {				\
	    if (PASTE(trace,t) >= l) {	\
		PRETRACE(l);		\
	    	printf(f, arg1, arg2, arg3, arg4, arg5); \
	    	POSTTRACE;		\
	    }				\
	} while (0)

#   define xIfTraceP( _obj, _lev ) \
	if ( *(_obj)->traceVar >= (_lev) )

#define xTraceP0( _obj, _lev, _fmt )					\
	do {								\
          xAssert((_obj)->traceVar);					\
	  if ( *(_obj)->traceVar >= (_lev) ) {				\
	    sprintf(errBuf, "[%s]: %s", (_obj)->fullName, (_fmt));	\
    		PRETRACE(_lev);		\
	    	printf(errBuf); 		\
	    	POSTTRACE;		\
	  }								\
	} while (0)

#define xTraceP1( _obj, _lev, _fmt, _arg1 )				\
	do {								\
          xAssert((_obj)->traceVar);					\
	  if ( *(_obj)->traceVar >= (_lev) ) {				\
	    sprintf(errBuf, "[%s]: %s", (_obj)->fullName, (_fmt));	\
    		PRETRACE(_lev);		\
	    	printf(errBuf, _arg1); 		\
	    	POSTTRACE;		\
	  }								\
	} while (0)

#define xTraceP2( _obj, _lev, _fmt, _arg1, _arg2 )			\
	do {								\
          xAssert((_obj)->traceVar);					\
	  if ( *(_obj)->traceVar >= (_lev) ) {				\
	    sprintf(errBuf, "[%s]: %s", (_obj)->fullName, (_fmt));	\
    		PRETRACE(_lev);						\
	    	printf(errBuf, _arg1, _arg2); 				\
	    	POSTTRACE;						\
	  }								\
	} while (0)

#define xTraceP3( _obj, _lev, _fmt, _arg1, _arg2, _arg3 )		\
	do {								\
          xAssert((_obj)->traceVar);					\
	  if ( *(_obj)->traceVar >= (_lev) ) {				\
	    sprintf(errBuf, "[%s]: %s", (_obj)->fullName, (_fmt));	\
    		PRETRACE(_lev);						\
	    	printf(errBuf, _arg1, _arg2, _arg3);			\
	    	POSTTRACE;						\
	  }								\
	} while (0)

#define xTraceP4( _obj, _lev, _fmt, _arg1, _arg2, _arg3, _arg4 )	\
	do {								\
          xAssert((_obj)->traceVar);					\
	  if ( *(_obj)->traceVar >= (_lev) ) {				\
	    sprintf(errBuf, "[%s]: %s", (_obj)->fullName, (_fmt));	\
    		PRETRACE(_lev);						\
	    	printf(errBuf, _arg1, _arg2, _arg3, _arg4);		\
	    	POSTTRACE;						\
	  }								\
	} while (0)

#define xTraceP5( _obj, _lev, _fmt, _arg1, _arg2, _arg3, _arg4, _arg5 )	\
	do {								\
          xAssert((_obj)->traceVar);					\
	  if ( *(_obj)->traceVar >= (_lev) ) {				\
	    sprintf(errBuf, "[%s]: %s", (_obj)->fullName, (_fmt));	\
    		PRETRACE(_lev);						\
	    	printf(errBuf, _arg1, _arg2, _arg3, _arg4, _arg5);	\
	    	POSTTRACE;						\
	  }								\
	} while (0)


#else

#   define xIfTrace(t, l) if (0)
#   define xIfTraceP(t, l) if (0)
#   define xTrace0(t, l, f)
#   define xTrace1(t, l, f, arg1)
#   define xTrace2(t, l, f, arg1, arg2)
#   define xTrace3(t, l, f, arg1, arg2, arg3)
#   define xTrace4(t, l, f, arg1, arg2, arg3, arg4)
#   define xTrace5(t, l, f, arg1, arg2, arg3, arg4, arg5)
#   define xTraceP0(t, l, f)
#   define xTraceP1(t, l, f, arg1)
#   define xTraceP2(t, l, f, arg1, arg2)
#   define xTraceP3(t, l, f, arg1, arg2, arg3)
#   define xTraceP4(t, l, f, arg1, arg2, arg3, arg4)
#   define xTraceP5(t, l, f, arg1, arg2, arg3, arg4, arg5)

#endif	/* XK_DEBUG */

extern void	xError(
		       char *
		       );

#endif	/* xk_debug_h */

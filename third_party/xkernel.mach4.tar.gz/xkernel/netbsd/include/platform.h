/* 
 * platform.h
 *
 * x-kernel v3.2
 *
 * Copyright (c) 1993,1991,1990  Arizona Board of Regents
 *
 *
 * $Revision: 1.36 $
 * $Date: 1993/10/13 23:44:06 $
 */

/*
 * "sunos" (sparc simulator) version of platform.h
 */

#ifndef platform_h
#define platform_h

#include "xk_debug.h"

#if defined(__GNUC__) && ! defined(XK_DEBUG)
#  define XK_USE_INLINE
#endif

#include <sys/types.h>
#include <netinet/in.h>
#include <malloc.h>
#include <limits.h>
#include "process.h"
#include "msg_s.h"
#include "xtype.h"


#ifdef __STDC__

u_short ocsum( u_short *buf, int count );
u_short ocsum_simple( u_short *buf, int count );
u_short inCkSum( Msg *m, u_short *buf, int len );
#ifndef X_NETBSD
int	atoi( char * );
#endif
char *	inet_ntoa( struct in_addr );

extern	char *	xMalloc( unsigned );
extern	void	xFree( char * );
extern	void	xMallocInit( void );

#else

extern	char *	xMalloc( );
extern	void	xFree();
extern	void	xMallocInit();

#endif


typedef long *Unspec;
typedef unsigned long ProcessId;
typedef unsigned long ContextId;

#ifndef NULL
#define NULL	0
#endif
#define MAXUNSIGNED	((unsigned) (-1)

#define splx(x)
#define spl7() 1
#define splnet spl7
#define splclk spl7

#define INIT_STACK_SIZE 1024

#define BYTES_PER_WORD	4

#ifndef ENDIAN
/*
 * Definitions for byte order,
 * according to byte significance from low address to high.
 */
#define	LITTLE	1234		/* least-significant byte first (vax) */
#define	BIG	4321		/* most-significant byte first */
#define	PDP	3412		/* LSB first in word, MSW first in long (pdp) */
#if defined(vax) || defined(pmax) || defined(mips)
#define	ENDIAN	LITTLE
#else
#define	ENDIAN	BIG		/* byte order on mc68000, tahoe, most others */
#endif
#endif ENDIAN

#define CLICKS_PER_SEC 100	/* Clock interrupts per second */

#if !defined(ROUND4)
#define ROUND4(len)  ((len + 3) & ~3)
#endif
#define BETWEEN(A,B,C) ((A) <= (B) && (B) < (C))

typedef	char	*mapKeyPtr_t;
typedef	int	mapVal_t;

typedef char	*statePtr_t;

#ifndef	TRUE
#define	TRUE	1
#define FALSE	0
#endif

#define	SUCCESS_RET_VAL		0
#define	FAILURE_RET_VAL		(-1)

#define	LO_BYTE_OF_2(word)	 ((u_char) (0xff & (unsigned) word))
#define HI_BYTE_OF_2(word)	 ((u_char) (((unsigned) word) >> 8 ))
#define CAT_BYTES(hiByte,loByte) ((((unsigned)hiByte)<<8) + (unsigned)loByte)

/* 
 * LONG_ALIGNMENT_REQUIRED indicates that int's must be 32-bit aligned
 * on this architecture.
 */
#define LONG_ALIGNMENT_REQUIRED
#define LONG_ALIGNED( address )  (! ((int)(address) & 0x3))
#define SHORT_ALIGNED( address ) ( ! ((int)(address) & 0x1))

#define ROM_MAX_LINES 	100 	/* Up to 100 table entries */
#define ROM_MAX_FIELDS 	20

extern	char *rom[ROM_MAX_LINES + 1][ROM_MAX_FIELDS + 1];  


#endif

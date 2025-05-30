/*	$NetBSD: search.h,v 1.7 1995/04/29 06:49:13 jtc Exp $	*/

/*
 * Written by J.T. Conklin <jtc@netbsd.org>
 * Public domain.
 */

#ifndef _SEARCH_H_
#define _SEARCH_H_
#include <sys/cdefs.h>
#include <machine/ansi.h>

#ifdef	_BSD_SIZE_T_
typedef	_BSD_SIZE_T_	size_t;
#undef	_BSD_SIZE_T_
#endif

__BEGIN_DECLS
extern void	*bsearch (const void *, const void *, size_t, size_t,
			      int (*)(const void *, const void *));
extern void	*lfind (const void *, const void *, size_t *, size_t,
			      int (*)(const void *, const void *));
extern void	*lsearch (const void *, const void *, size_t *, size_t,
			      int (*)(const void *, const void *));
extern void	 insque (void *, void *);
extern void	 remque (void *);
__END_DECLS

#endif

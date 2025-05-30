/* 
 * x_util.h
 *
 * x-kernel v3.2
 *
 * Copyright (c) 1993,1991,1990  Arizona Board of Regents
 *
 *
 * $Revision: 1.3 $
 * $Date: 1993/02/01 22:40:06 $
 */

#ifndef x_util_h
#define x_util_h


extern void	Delay( int );
extern void	Kabort( char * );


/* 
 * non-ANSI compilers gripe about the _n##U usage while GCC gives
 * warning messages about the u_long cast.
 */
#   define UNSIGNED(_n)	_n##U

#endif ! x_util_h

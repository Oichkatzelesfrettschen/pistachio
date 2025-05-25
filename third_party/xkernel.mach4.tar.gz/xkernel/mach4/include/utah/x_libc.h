/*     
 * $RCSfile: x_libc.h,v $
 *
 * x-kernel v3.2
 *
 * Copyright (c) 1993,1991,1990  Arizona Board of Regents
 *
 *
 * $Revision: 1.1 $
 * $Date: 1993/08/12 21:17:55 $
 */

/*
 * C library prototypes
 *
 * This version is specific to the Utah port of mach3 for the HP Snakes.
 */

#ifndef x_libc_h
#define x_libc_h

#ifndef XKMACHKERNEL

#include <string.h>
#include <stdio.h>

extern void     bcopy( char *, char *, int );
extern int	bcmp( char *, char *, int );
extern void     bzero( char *, int );

#endif ! XKMACHKERNEL

#endif ! x_libc_h

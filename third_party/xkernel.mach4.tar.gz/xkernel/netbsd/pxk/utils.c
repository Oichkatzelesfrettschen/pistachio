/* 
 * utils.c
 *
 * x-kernel v3.2
 *
 * Copyright (c) 1993,1991,1990  Arizona Board of Regents
 *
 *
 * $Revision: 1.9 $
 * $Date: 1993/03/02 23:55:55 $
 */

#include "x_stdio.h"
#include "x_libc.h"
#include "x_util.h"
#include "upi.h"

extern char *	malloc();
extern int	abort();

void
Kabort(s)
    char *s;
{
    fprintf(stderr, "xkernel abort: %s\n", s);
    xRapture();
    abort();
}


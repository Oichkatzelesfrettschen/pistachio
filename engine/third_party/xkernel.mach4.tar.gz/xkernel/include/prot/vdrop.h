/* 
 * $RCSfile: vdrop.h,v $
 *
 * x-kernel v3.2
 *
 * Copyright (c) 1993,1991,1990  Arizona Board of Regents
 *
 *
 * $Revision: 1.3 $
 * $Date: 1993/02/13 10:00:06 $
 */

#ifndef vdrop_h
#define vdrop_h
   
#  ifdef __STDC__

void	vdrop_init( XObj );

#  endif

#define VDROP_GETINTERVAL	(VDROP_CTL * MAXOPS + 0)
#define VDROP_SETINTERVAL	(VDROP_CTL * MAXOPS + 1)


#endif  ! vcache_h

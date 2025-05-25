/* 
 * $RCSfile: vdisorder.h,v $
 *
 * x-kernel v3.2
 *
 * Copyright (c) 1993,1991,1990  Arizona Board of Regents
 *
 *
 * $Revision: 1.1 $
 * $Date: 1993/05/05 22:49:19 $
 */

#ifndef vdisorder_h
#define vdisorder_h
   
#  ifdef __STDC__

void	vdisorder_init( XObj );

#  endif

#define VDISORDER_CTL	TMP3_CTL

#define VDISORDER_SET_ARRIVAL_INTERVAL	(VDISORDER_CTL * MAXOPS + 0)
#define VDISORDER_GET_ARRIVAL_INTERVAL	(VDISORDER_CTL * MAXOPS + 1)
#define VDISORDER_SET_DISORDER_INTERVAL	(VDISORDER_CTL * MAXOPS + 2)
#define VDISORDER_GET_DISORDER_INTERVAL	(VDISORDER_CTL * MAXOPS + 3)
#define VDISORDER_SET_TIMEOUT_INTERVAL	(VDISORDER_CTL * MAXOPS + 4)
#define VDISORDER_GET_TIMEOUT_INTERVAL	(VDISORDER_CTL * MAXOPS + 5)


#endif  ! vdisorder_h

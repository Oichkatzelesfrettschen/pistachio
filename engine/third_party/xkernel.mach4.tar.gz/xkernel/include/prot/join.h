/*
 * join.h
 *
 * x-kernel v3.2
 *
 * Copyright (c) 1993  Arizona Board of Regents
 *
 *
 * $Revision: 1.2 $
 * $Date: 1993/06/24 20:49:41 $
 *
 */

#ifndef join_h
#define join_h

#define JOINMAXSEG  16

#define JOINGETNUMSEG  		(JOIN_CTL*MAXOPS + 0)
#define JOINGETORDER  		(JOIN_CTL*MAXOPS + 1)
#define JOINDONE 		(JOIN_CTL*MAXOPS + 2)
#define JOINSETCONTROL 		(JOIN_CTL*MAXOPS + 4)


#  ifdef __STDC__

void	join_init( XObj );

#  endif


#endif

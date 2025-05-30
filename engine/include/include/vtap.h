/* 
 * vtap.h 
 *
 * x-kernel v3.2
 *
 * Copyright (c) 1993  Arizona Board of Regents
 *
 *
 */

#ifndef vtap_h
#define vtap_h
   

void	vtap_init( XObj );

#define VTAP_ENABLETAP       (VTAP_CTL * MAXOPS + 0)
#define VTAP_DISABLETAP      (VTAP_CTL * MAXOPS + 1)
#define VTAP_PRINTCHARS      (VTAP_CTL * MAXOPS + 2)
#define VTAP_PRINTHEX        (VTAP_CTL * MAXOPS + 3)




#endif  

/*     
 * $RCSfile: sim.h,v $
 *
 * x-kernel v3.2
 *
 * Copyright (c) 1993,1991,1990  Arizona Board of Regents
 *
 *
 * $Log: sim.h,v $
 * Revision 1.1  1993/11/15  20:38:49  menze
 * Initial revision
 *
 */

/* 
 * Operations common to the simulator class of protocols
 */

#ifndef sim_h
#define sim_h

#define	SIM_SOCK2ADDR	( SIM_CTL * MAXOPS + 0 )

typedef struct {
    IPhost	ipHost;	  /* in */
    int		udpPort;  /* in */
    ETHhost	ethHost;  /* out */
} SimAddrBuf;

#endif /* sim_h */


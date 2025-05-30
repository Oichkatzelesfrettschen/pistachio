/* 
 * chan.h
 *
 * x-kernel v3.2
 *
 * Copyright (c) 1993,1991,1990  Arizona Board of Regents
 *
 *
 * $Revision: 1.5 $
 * $Date: 1993/07/20 19:47:20 $
 */

#ifndef chan_h
#define chan_h

enum {
    CHAN_ABORT_CALL = CHAN_CTL*MAXOPS,
    CHAN_SET_TIMEOUT,
    CHAN_GET_TIMEOUT,
    CHAN_SET_MAX_TIMEOUT,
    CHAN_GET_MAX_TIMEOUT,
    CHAN_RETRANSMIT
};

typedef struct {
    XObj transport;
    int  ticket;
    int  reliable;
    int  expensive;
    unsigned int timeout;
} chan_info_t; 
    

void	chan_init( XObj );


#endif chan_h

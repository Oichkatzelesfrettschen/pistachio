/*
 * fsm.h - {Link, IP} Control Protocol Finite State Machine definitions.
 *
 * Copyright (c) 2000 by Sun Microsystems, Inc.
 * All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, provided that the above copyright
 * notice appears in all copies.
 *
 * SUN MAKES NO REPRESENTATION OR WARRANTIES ABOUT THE SUITABILITY OF
 * THE SOFTWARE, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 * TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, OR NON-INFRINGEMENT.  SUN SHALL NOT BE LIABLE FOR
 * ANY DAMAGES SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING OR
 * DISTRIBUTING THIS SOFTWARE OR ITS DERIVATIVES
 *
 * Copyright (c) 1989 Carnegie Mellon University.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by Carnegie Mellon University.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * $Id: fsm.h,v 1.8 1999/11/15 01:51:50 paulus Exp $
 */

#pragma ident	"%Z%%M%	%I%	%E% SMI"

#ifndef __FSM_H__
#define __FSM_H__

#ifdef	__cplusplus
extern "C" {
#endif

/*
 * Packet header = Code, id, length.
 */
#define HEADERLEN	4

/*
 *  Control Protocol (LCP, IPCP, etc.) message code numbers.
 *  (More in lcp.h and net/ppp-comp.h.)
 */
#define CODE_CONFREQ		1	/* Configuration Request */
#define CODE_CONFACK		2	/* Configuration Ack */
#define CODE_CONFNAK		3	/* Configuration Nak */
#define CODE_CONFREJ		4	/* Configuration Reject */
#define CODE_TERMREQ		5	/* Termination Request */
#define CODE_TERMACK		6	/* Termination Ack */
#define CODE_CODEREJ		7	/* Code Reject */


/*
 * Each FSM is described by an fsm structure and fsm callbacks.
 */
typedef struct fsm {
    int unit;			/* Interface unit number */
    int protocol;		/* Data Link Layer Protocol field value */
    int state;			/* State */
    int flags;			/* Contains option bits */
    u_char id;			/* Current id */
    u_char reqid;		/* Current request id */
    u_char seen_ack;		/* Have received valid Ack/Nak/Rej to Req */
    int timeouttime;		/* Timeout time in milliseconds */
    int maxconfreqtransmits;	/* Maximum Configure-Request transmissions */
    int retransmits;		/* Number of retransmissions left */
    int maxtermtransmits;	/* Maximum Terminate-Request transmissions */
    int nakloops;		/* Number of nak loops since last ack */
    int maxnakloops;		/* Maximum number of nak loops tolerated */
    struct fsm_callbacks *callbacks;	/* Callback routines */
    char *term_reason;		/* Reason for closing protocol */
    int term_reason_len;	/* Length of term_reason */
    u_int32_t codemask[8];	/* Codes rejected by peer */
} fsm;

/*
 *	Function	RFC 1661
 *	-----		------------------------------
 *	ackci		RCA
 *	nakci		RCN
 *	rejci		RCN
 *	reqci		RCR+ or RCR- (by return value)
 *	up		tlu (this-layer-up)
 *	down		tld (this-layer-down)
 *	starting	tls (this-layer-starting)
 *	finished	tlf (this-layer-finished)
 *	codereject	RXJ+ or RXJ- (by return value)
 *
 * Note that this-layer-down means "stop transmitting."
 * This-layer-finished means "stop everything."
 */

typedef struct fsm_callbacks {
    void (*resetci)		/* Reset our Configuration Information */
		(fsm *);
    int  (*cilen)		/* Length of our Configuration Information */
		(fsm *);
    void (*addci)		/* Add our Configuration Information */
		(fsm *, u_char *, int *);
    int  (*ackci)		/* ACK our Configuration Information */
		(fsm *, u_char *, int);
    int  (*nakci)		/* NAK our Configuration Information */
		(fsm *, u_char *, int);
    int  (*rejci)		/* Reject our Configuration Information */
		(fsm *, u_char *, int);
    int  (*reqci)		/* Request peer's Configuration Information */
		(fsm *, u_char *, int *, int);
    void (*up)			/* Called when fsm reaches OPENED state */
		(fsm *);
    void (*down)		/* Called when fsm leaves OPENED state */
		(fsm *);
    void (*starting)		/* Called when we want the lower layer */
		(fsm *);
    void (*finished)		/* Called when we don't want the lower layer */
		(fsm *);
    void (*retransmit)		/* Retransmission is necessary */
		(fsm *);
    int  (*extcode)		/* Called when unknown code received */
		(fsm *, int, int, u_char *, int);
    char *proto_name;		/* String name for protocol (for messages) */
    int (*codereject)		/* Called when Code-Reject received */
		(fsm *p, int code, int id, u_char *inp, int len);
} fsm_callbacks;


/*
 * RFC 1661 NCP states.
 */
#define INITIAL		0	/* Down, hasn't been opened */
#define STARTING	1	/* Down, been opened */
#define CLOSED		2	/* Up, hasn't been opened */
#define STOPPED		3	/* Open, waiting for down event */
#define CLOSING		4	/* Terminating the connection, not open */
#define STOPPING	5	/* Terminating, but open */
#define REQSENT		6	/* We've sent a Config Request */
#define ACKRCVD		7	/* We've received a Config Ack */
#define ACKSENT		8	/* We've sent a Config Ack */
#define OPENED		9	/* Connection available */

#define FSM__STATES \
	"Initial", "Starting", "Closed", "Stopped", "Closing", "Stopping", \
	"ReqSent", "AckRcvd", "AckSent", "Opened"

/*
 * Flags - indicate options controlling FSM operation
 */
#define OPT_PASSIVE	1	/* Don't die if we don't get a response */
#define OPT_RESTART	2	/* Treat 2nd OPEN as DOWN, UP */
#define OPT_SILENT	4	/* Wait for peer to speak first */


/*
 * Timeouts.
 */
#define DEFTIMEOUT	3	/* Timeout time in seconds */
#define DEFMAXTERMREQS	2	/* Maximum Terminate-Request transmissions */
#define DEFMAXCONFREQS	10	/* Maximum Configure-Request transmissions */
#define DEFMAXNAKLOOPS	5	/* Maximum number of nak loops */


/*
 * Prototypes
 */
void fsm_init (fsm *);
void fsm_lowerup (fsm *);
void fsm_lowerdown (fsm *);
void fsm_open (fsm *);
void fsm_close (fsm *, char *);
void fsm_input (fsm *, u_char *, int);
void fsm_protreject (fsm *);
void fsm_sdata (fsm *, int, int, u_char *, int);
void fsm_setpeermru (int, int);

const char *fsm_state (int statenum);

#ifdef	__cplusplus
}
#endif

#endif /* __FSM_H__ */

/* 
 * sunrpc.h
 *
 * x-kernel v3.2
 *
 * Copyright (c) 1993,1991,1990  Arizona Board of Regents
 *
 *
 * $Revision: 1.8 $
 * $Date: 1993/02/01 22:40:34 $
 */

#ifndef sun_rpc_h
#define sun_rpc_h

#include "udp.h"


/* Sun RPC control ops; those beginning with CLNT
 * are available only to client, and those beginning
 * with SVC are available only to server
 */

#define SUNRPC_SVCGETCALLER	(SUNRPC_CTL*MAXOPS+0)
#define SUNRPC_CLNTSETTOUT	(SUNRPC_CTL*MAXOPS+1)
#define SUNRPC_CLNTGETTOUT	(SUNRPC_CTL*MAXOPS+2)
#define SUNRPC_CLNTSETWAIT	(SUNRPC_CTL*MAXOPS+3)
#define SUNRPC_CLNTGETWAIT	(SUNRPC_CTL*MAXOPS+4)
#define SUNRPC_CLNTGETERROR	(SUNRPC_CTL*MAXOPS+5)
#define SUNRPC_SVCGETARGLEN	(SUNRPC_CTL*MAXOPS+6)
#define SUNRPC_SVCGETARGS	(SUNRPC_CTL*MAXOPS+7)
#define SUNRPC_GETCRED		(SUNRPC_CTL*MAXOPS+8)
#define SUNRPC_SETCRED		(SUNRPC_CTL*MAXOPS+9)
#define SUNRPC_GETVERF		(SUNRPC_CTL*MAXOPS+10)
#define SUNRPC_SETVERF		(SUNRPC_CTL*MAXOPS+11)
#define SUNRPC_SVCAUTHERR	(SUNRPC_CTL*MAXOPS+12)
#define SUNRPC_SVCGARBAGEARGS	(SUNRPC_CTL*MAXOPS+13)
#define SUNRPC_SVCSYSTEMERR	(SUNRPC_CTL*MAXOPS+14)
#define SUNRPC_GETCREDTYPE	(SUNRPC_CTL*MAXOPS+15)
#define SUNRPC_SETCREDTYPE	(SUNRPC_CTL*MAXOPS+16)
#define SUNRPC_GETVERFTYPE	(SUNRPC_CTL*MAXOPS+17)
#define SUNRPC_SETVERFTYPE	(SUNRPC_CTL*MAXOPS+18)
#define SUNRPC_SVCGETHLP	(SUNRPC_CTL*MAXOPS+19)
#define SUNRPC_SVCGETSERVER	(SUNRPC_CTL*MAXOPS+20)
#define SUNRPC_GETPORT		(SUNRPC_CTL*MAXOPS+21)



void	sunrpc_init( XObj );


#endif


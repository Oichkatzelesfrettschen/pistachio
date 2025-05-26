/*
 * km.h
 *
 * x-kernel v3.2
 *
 * Copyright (c) 1993  Arizona Board of Regents
 *
 * $Revision: 1.1 $
 * $Date: 1993/07/06 23:28:25 $
 *
 */

/* Data structures & interface for Key Manager */

#ifndef BIND_H
#define BIND_H

#include "upi.h"


/*
 * BIND control opcodes
 */
#define BIND_RESOLVE	(BIND_CTL*MAXOPS+0)
#define BIND_SET	(BIND_CTL*MAXOPS+1)
#define BIND_INVALIDATE	(BIND_CTL*MAXOPS+2)
#define BIND_ISEMPTY	(BIND_CTL*MAXOPS+3)
#define BIND_ISVALID	(BIND_CTL*MAXOPS+4)
#define BIND_ISINVALID	(BIND_CTL*MAXOPS+5)
#define BIND_KEYSIZE	(BIND_CTL*MAXOPS+6)
#define BIND_HOSTSIZE	(BIND_CTL*MAXOPS+7)


#endif BIND_H

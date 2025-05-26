/*
 * km.h
 *
 * x-kernel v3.2
 *
 * Copyright (c) 1993  Arizona Board of Regents
 *
 * $Revision: 1.4 $
 * $Date: 1993/06/04 23:02:42 $
 *
 */

/* Data structures & interface for Key Manager */

#ifndef KM_H
#define KM_H

#include "upi.h"

/*
 * KM control opcodes
 */
#define KM_RESOLVE	(KM_CTL*MAXOPS+0)
#define KM_SET		(KM_CTL*MAXOPS+1)
#define KM_INVALIDATE	(KM_CTL*MAXOPS+2)
#define KM_ISEMPTY	(KM_CTL*MAXOPS+3)
#define KM_ISVALID	(KM_CTL*MAXOPS+4)
#define KM_ISINVALID	(KM_CTL*MAXOPS+5)
#define KM_KEYSIZE	(KM_CTL*MAXOPS+6)
#define KM_HOSTSIZE	(KM_CTL*MAXOPS+7)


#endif KM_H

/*
 * chap_ms.h - Microsoft CHAP definitions.
 *
 * Copyright (c) 2000 by Sun Microsystems, Inc.
 * All rights reserved.
 *
 * Copyright (c) 1995 Eric Rosenquist, Strata Software Limited.
 * http://www.strataware.com/
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by Eric Rosenquist.  The name of the author may not be used to
 * endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * $Id: chap_ms.h,v 1.2 1997/11/27 06:08:10 paulus Exp $
 */

#pragma ident	"%Z%%M%	%I%	%E% SMI"

#ifndef __CHAPMS_INCLUDE__

#define MD4_SIGNATURE_SIZE	16	/* 16 bytes in a MD4 message digest */
#define MAX_NT_PASSWORD	256	/* Maximum number of (Unicode) chars in an NT password */

void ChapMS(chap_state *, u_char *, int, char *, int);
void ChapMSv2(chap_state *, u_char *, int, char *, int);
int ChapMSValidate(chap_state *cstate, u_char *response, int response_len,
    char *secret, int secret_len);
int ChapMSv2Validate(chap_state *cstate, char *rhostname,
    u_char *response, int response_len, char *secret, int secret_len);

#define __CHAPMS_INCLUDE__
#endif /* __CHAPMS_INCLUDE__ */

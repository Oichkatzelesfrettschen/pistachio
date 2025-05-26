/* 
 * Mach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 * 
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they make and grant Carnegie Mellon 
 * the rights to redistribute these changes.
 */
/*
 * HISTORY
 * $Log:	uipc_domain.c,v $
 * Revision 2.1  92/04/21  17:12:40  rwd
 * BSDSS
 * 
 *
 */

/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)uipc_domain.c	7.9 (Berkeley) 3/4/91
 */

#include <ns.h>
#include <iso.h>
#include <rmp.h>
#include <ccitt.h>
#include <inet.h>

#include <sys/cdefs.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/protosw.h>
#include <sys/domain.h>
#include <sys/mbuf.h>
#include <sys/time.h>
#include <sys/kernel.h>

#define	ADDDOMAIN(x)	{ \
	extern struct domain __CONCAT(x,domain); \
	__CONCAT(x,domain.dom_next) = domains; \
	domains = &__CONCAT(x,domain); \
}

domaininit()
{
	register struct domain *dp;
	register struct protosw *pr;

#undef unix
#ifndef lint
	ADDDOMAIN(unix);
	ADDDOMAIN(route);
#if INET
	ADDDOMAIN(inet);
#endif
#if NS
	ADDDOMAIN(ns);
#endif
#if ISO
	ADDDOMAIN(iso);
#endif
#if RMP
	ADDDOMAIN(rmp);
#endif
#if CCITT
	ADDDOMAIN(ccitt);
#endif
#include <imp.h>
#if NIMP > 0
	ADDDOMAIN(imp);
#endif
#endif

	for (dp = domains; dp; dp = dp->dom_next) {
		if (dp->dom_init)
			(*dp->dom_init)();
		for (pr = dp->dom_protosw; pr < dp->dom_protoswNPROTOSW; pr++)
			if (pr->pr_init)
				(*pr->pr_init)();
	}

if (max_linkhdr < 16)		/* XXX */
max_linkhdr = 16;
	max_hdr = max_linkhdr + max_protohdr;
	max_datalen = MHLEN - max_hdr;
	pffasttimo();
	pfslowtimo();
}

struct protosw *
pffindtype(family, type)
	int family, type;
{
	register struct domain *dp;
	register struct protosw *pr;

	for (dp = domains; dp; dp = dp->dom_next)
		if (dp->dom_family == family)
			goto found;
	return (0);
found:
	for (pr = dp->dom_protosw; pr < dp->dom_protoswNPROTOSW; pr++)
		if (pr->pr_type && pr->pr_type == type)
			return (pr);
	return (0);
}

struct protosw *
pffindproto(family, protocol, type)
	int family, protocol, type;
{
	register struct domain *dp;
	register struct protosw *pr;
	struct protosw *maybe = 0;

	if (family == 0)
		return (0);
	for (dp = domains; dp; dp = dp->dom_next)
		if (dp->dom_family == family)
			goto found;
	return (0);
found:
	for (pr = dp->dom_protosw; pr < dp->dom_protoswNPROTOSW; pr++) {
		if ((pr->pr_protocol == protocol) && (pr->pr_type == type))
			return (pr);

		if (type == SOCK_RAW && pr->pr_type == SOCK_RAW &&
		    pr->pr_protocol == 0 && maybe == (struct protosw *)0)
			maybe = pr;
	}
	return (maybe);
}

pfctlinput(cmd, sa)
	int cmd;
	struct sockaddr *sa;
{
	register struct domain *dp;
	register struct protosw *pr;

	for (dp = domains; dp; dp = dp->dom_next)
		for (pr = dp->dom_protosw; pr < dp->dom_protoswNPROTOSW; pr++)
			if (pr->pr_ctlinput)
				(*pr->pr_ctlinput)(cmd, sa, (caddr_t) 0);
}

pfslowtimo()
{
	register struct domain *dp;
	register struct protosw *pr;

	for (dp = domains; dp; dp = dp->dom_next)
		for (pr = dp->dom_protosw; pr < dp->dom_protoswNPROTOSW; pr++)
			if (pr->pr_slowtimo)
				(*pr->pr_slowtimo)();
	timeout(pfslowtimo, (caddr_t)0, hz/2);
}

pffasttimo()
{
	register struct domain *dp;
	register struct protosw *pr;

	for (dp = domains; dp; dp = dp->dom_next)
		for (pr = dp->dom_protosw; pr < dp->dom_protoswNPROTOSW; pr++)
			if (pr->pr_fasttimo)
				(*pr->pr_fasttimo)();
	timeout(pffasttimo, (caddr_t)0, hz/5);
}

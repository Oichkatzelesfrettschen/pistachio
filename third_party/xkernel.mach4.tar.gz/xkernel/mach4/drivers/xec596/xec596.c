/*
 * Support for Intel 82596 device (run in 586 compatibility mode)
 * on the HP 700 series machines.
 *
 * Derived from the Mach Intel 82586 driver for the PC.
 *
 * TODO:
 *	Go native
 */
/*
 * HISTORY
 * $Log: xec596.c,v $
 * Revision 1.15  1993/03/10  19:13:15  davidm
 * *** empty log message ***
 *
 * Revision 1.14  1993/03/09  20:01:33  davidm
 * ior->io_count was set incorrectly.
 *
 * Revision 1.13  1993/03/06  01:24:15  davidm
 * Cleanup.
 *
 * Revision 1.12  1993/03/02  21:35:10  davidm
 * Call to io_req_free() in xmt_done() deleted; the io request is deleted
 * by the calling function already (as long as xmt_done() returns TRUE).
 *
 * Revision 1.11  1993/03/02  03:26:54  davidm
 * Some fixes...
 *
 * Revision 1.10  1993/02/26  04:15:18  davidm
 * Added lost & operator.
 *
 * Revision 1.9  1993/02/26  04:04:46  davidm
 * Cleaned-up code a little bit.
 *
 * Revision 1.8  1993/02/26  00:50:21  davidm
 * Disabled debugging code.
 *
 * Revision 1.7  1993/02/26  00:49:30  davidm
 * Have to pad packet to ETHERMIN length explicitly.  Chip does *NOT*
 * do this automatically!
 *
 * Revision 1.6  1993/02/12  23:39:55  davidm
 * Updated msgSetAttr() calls.
 *
 * Revision 1.5  1993/02/11  18:26:00  davidm
 * This is a working version of the driver.
 *
 * Revision 1.4  1992/10/31  04:11:13  davidm
 * Previous checkin failed...
 *
 * Revision 1.3  1992/10/31  04:08:32  davidm
 * This is an almost working ethernet driver now.  The remaining problem
 * is that msgDestroy() sometimes gets invoked twice...
 *
 * Revision 1.2  1992/10/14  04:22:34  davidm
 * Added RCS log.
 *
 */
/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
 * Copyright (c) 1991,1992 The University of Utah and
 * the Center for Software Science (CSS).
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON, THE UNIVERSITY OF UTAH AND CSS ALLOW FREE USE OF
 * THIS SOFTWARE IN ITS "AS IS" CONDITION, AND DISCLAIM ANY LIABILITY
 * OF ANY KIND FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF
 * THIS SOFTWARE.
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
 *
 * CSS requests users of this software to return to css-dist@cs.utah.edu any
 * improvements that they make and grant CSS redistribution rights.
 *
 * 	Utah $Hdr: if_i596.c 3.18 92/05/27$
 */
/* 
 *	Olivetti PC586 Mach Ethernet driver v1.0
 *	Copyright Ing. C. Olivetti & C. S.p.A. 1988, 1989
 *	All rights reserved.
 *
 */ 
/*
  Copyright 1988, 1989 by Olivetti Advanced Technology Center, Inc.,
Cupertino, California.

		All Rights Reserved

  Permission to use, copy, modify, and distribute this software and
its documentation for any purpose and without fee is hereby
granted, provided that the above copyright notice appears in all
copies and that both the copyright notice and this permission notice
appear in supporting documentation, and that the name of Olivetti
not be used in advertising or publicity pertaining to distribution
of the software without specific, written prior permission.

  OLIVETTI DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
IN NO EVENT SHALL OLIVETTI BE LIABLE FOR ANY SPECIAL, INDIRECT, OR
CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT,
NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUR OF OR IN CONNECTION
WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

/*
  Copyright 1988, 1989 by Intel Corporation, Santa Clara, California.

		All Rights Reserved

Permission to use, copy, modify, and distribute this software and
its documentation for any purpose and without fee is hereby
granted, provided that the above copyright notice appears in all
copies and that both the copyright notice and this permission notice
appear in supporting documentation, and that the name of Intel
not be used in advertising or publicity pertaining to distribution
of the software without specific, written prior permission.

INTEL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
IN NO EVENT SHALL INTEL BE LIABLE FOR ANY SPECIAL, INDIRECT, OR
CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT,
NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

/*
 * NOTE:
 *		by rvb:
 *  1.	The best book on the 82586 is:
 *		LAN Components User's Manual by Intel
 *	The copy I found was dated 1984.  This really tells you
 *	what the state machines are doing
 *  2.	In the current design, we only do one write at a time,
 *	though the hardware is capable of chaining and possibly
 *	even batching.  The problem is that we only make one
 *	transmit buffer available in sram space.
 *  3.	
 *  n.	Board Memory Map (require 32-byte cache alignment)
	RFA/FD	0x0000 - 0x02df	0x2e0 bytes
		 0x2e0 = 0x17 * 0x20 bytes
	RBD	0x02e0 - 0x369f	0x33c0 bytes
		0x33c0 = 0x17 * 0x240 bytes
				== 0x20 bytes (bd) + 0x220 bytes
	CU	0x36a0 - 0x379f	0x100 bytes
	TBD	0x37a0 - 0x395f	0x1c0 bytes
		 0x1c0 = No 14 * 0x20 bytes
	TBUF	0x3960 - 0x3f9f	0x640 bytes (> 1594(10))
	SCB	0x3fa0 - 0x3fbf	0x20 bytes
	ISCP	0x3fc0 - 0x3fdf	0x20 bytes
	SCP	0x3fe0 - 0x3fff	0x20 bytes
 *		
 */

/*
 * NOTE:
 *
 *	Currently this driver doesn't support trailer protocols for
 *	packets.  Once that is added, please remove this comment.
 */

#define XMTWATCHDOG

#include "xlan.h"
#if NXLAN > 0

#include	<hpsgc/ostype.h>	/* XXX */

#include	<sys/types.h>
#include	<machine/spl.h>
#include	<machine/Jasp.h>	/* XXX */
#include	<mach/machine/vm_types.h>

#include	<kern/time_out.h>
#include	<device/device_types.h>
#include	<device/errno.h>
#include	<device/io_req.h>
#include	<device/if_hdr.h>
#include	<device/if_ether.h>
#include	<device/net_status.h>
#include	<device/net_io.h>

#include	<hpdev/device.h>
#include	<hpsgc/mach_glue.h>
#include	<hpsgc/if_i596var.h>

#define LAN_BUFF_SIZE	(0x4000 + 16)

#define pc586chatt(unit)  pc_softc[unit].hwaddr->attn = 0
#define pc586inton(unit)  aspitab(INT_LAN, SPLLAN, pc586intr, unit, 0)
#define pc586intoff(unit) aspitab(INT_LAN, SPLLAN, NULL, unit, 0)

int	pc586probe(), pc586attach();
int	pc586intr(), pc586init(), pc586output(), pc586ioctl(), pc586reset();
#ifdef WATCHDOG
int	pc586watch();
#endif
#ifdef XMTWATCHDOG
int	pc586xmtwatch();
#endif
int	pc586rcv(), pc586bldcu();
int	pc586diag(), pc586config();
char	*pc586bldru();
volatile char *ram_to_ptr();
int	ptr_to_ram();

char	pc586sram[LAN_BUFF_SIZE+32];	/* equiv mapped to make vtop simple */
char	t_packet[ETHERMTU + sizeof(struct ether_header) + sizeof(long)];
int	xmt_watch = 0;

typedef struct { 
	struct	ifnet	ds_if;		/* generic interface header */
	u_char	ds_addr[6];		/* Ethernet hardware address */
	int	flags;
        int     seated;
#ifdef WATCHDOG
        int     timer;
#endif
#ifdef XMTWATCHDOG
	int	xmttimer;
#endif
        int     open;
        fd_t    *begin_fd;
	volatile fd_t *end_fd;
	rbd_t   *end_rbd;
	volatile char	*prom;
	volatile char	*sram;
	int     tbusy;
	volatile struct	core_lan *hwaddr;
	short	mode;
} pc_softc_t;
pc_softc_t	pc_softc[NXLAN];

#ifdef	DEBUG
int	pc586debug = 0;
#endif

#ifdef	WATCHDOG
int	pc586timo = 5;
#endif
#ifdef	XMTWATCHDOG
int	pc586xmttimo = 3;
#endif

#ifdef XKMACHKERNEL

#include "xkernel.h"
#include "eth.h"
#include "eth_i.h"
#include "msg.h"
#include "msg_internal.h"
#include "trace.h"
#include "list.h"
#include "kern_process_msg.h"
#include "process.h"

extern list_t ThreadQ;

/*
 * The unit number is stored in the protocol's state field:
 */
#define XEC596_UNIT(s)	((int)(s)->state)

/*
 * Maximum Transmission Unit:
 */
#define MTU	1500

/*
 * Keep one protocol pointer per interface:
 */
XObj self_protocol[NXLAN];

int tracexec596p = 0;

static void		xec596_xmt (int unit, io_req_t ior);
static int		xec596_read (int unit, volatile fd_t *fd_p);
static int		xec596_control (XObj self, int op, char *buf, int len);
static xkern_return_t	xec596_openenable (XObj self, XObj hlp, XObj hlpType,
					   Part *part);
static xmsg_handle_t	xec596_push (XObj s, Msg *downMsg);

xkern_return_t		xec596_init (XObj self);

#endif /* XKMACHKERNEL */


/*
 * pc586probe:
 *
 *	This function "probes" or checks for the pc586 board on the bus to see
 *	if it is there.  As far as I can tell, the best break between this
 *	routine and the attach code is to simply determine whether the board
 *	is configured in properly.  Currently my approach to this is to write
 *	and read a word from the SRAM on the board being probed.  If the word
 *	comes back properly then we assume the board is there.  The config
 *	code expects to see a successful return from the probe routine before
 *	attach will be called.
 *
 * input	: address device is mapped to, and unit # being checked
 * output	: a '1' is returned if the board exists, and a 0 otherwise
 *
 */
int
pc586probe(dev)
     struct hp_device	*dev;
{
	caddr_t		addr = dev->hp_addr;
	int		unit = dev->hp_unit;
	volatile char	*b_prom;
	volatile char	*b_sram;
	pc_softc_t	*sp = &pc_softc[unit];

	if (unit < 0 || unit >= NXLAN) {
		printf("pc%d: board out of range [0..%d]\n", unit,
		       NXLAN-1);
		return 0;
	}
	if ((int)addr < 0xF0000000)
		return 0;
	b_prom = (volatile char *) LAN_PROM_ADDR;
	b_sram = (volatile char *) (((int)pc586sram + 31) & ~31);
#ifdef DEBUG
	if (pc586debug)
		printf("pc%d: probe: addr=0x%x pc586sram=0x%x b_sram=0x%x\n",
		       unit, dev->hp_addr, pc586sram, b_sram);
#endif
	sp->seated = TRUE;
	sp->prom = b_prom;
	sp->sram = b_sram;
	return 1;
}

/*
 * pc586attach:
 *
 *	This function attaches a PC586 board to the "system".  The rest of
 *	runtime structures are initialized here (this routine is called after
 *	a successful probe of the board).  Once the ethernet address is read
 *	and stored, the board's ifnet structure is attached and readied.
 *
 * input	: hp_device structure setup in autoconfig
 * output	: board structs and ifnet is setup
 *
 */
int
pc586attach(dev)
	struct hp_device *dev;
{
	struct	ifnet	*ifp;
	u_char		*addr_p;
	u_char		*b_addr;
	u_char		unit = (u_char)dev->hp_unit;	
	pc_softc_t	*sp = &pc_softc[unit];
	volatile scb_t	*scb_p;

#ifdef WATCHDOG
	sp->timer = -1;
	sp->xmttimer = -1;
#endif
	sp->mode = 0;
	sp->flags = 0;
	sp->open = 0;
	sp->hwaddr = (volatile struct core_lan *) dev->hp_addr;
	sp->hwaddr->reset = 0;
	b_addr = (u_char *)(sp->prom + OFFSET_PROM);
	addr_p = (u_char *)sp->ds_addr;
	addr_p[0] = b_addr[0];
	addr_p[1] = b_addr[1];
	addr_p[2] = b_addr[2];
	addr_p[3] = b_addr[3];
	addr_p[4] = b_addr[4];
	addr_p[5] = b_addr[5];
	printf("ethernet id [%x:%x:%x:%x:%x:%x]\n",
		addr_p[0], addr_p[1], addr_p[2],
		addr_p[3], addr_p[4], addr_p[5]);

	scb_p = (volatile scb_t *)(sp->sram + OFFSET_SCB);
	scb_p->scb_crcerrs = 0;			/* initialize counters */
	scb_p->scb_alnerrs = 0;
	scb_p->scb_rscerrs = 0;
	scb_p->scb_ovrnerrs = 0;
	flushscball(scb_p);

	ifp = &(sp->ds_if);
	ifp->if_unit = unit;
	ifp->if_mtu = ETHERMTU;
	ifp->if_flags = IFF_BROADCAST;
	ifp->if_header_size = sizeof(struct ether_header);
	ifp->if_header_format = HDR_ETHERNET;
	ifp->if_address_size = 6;
	ifp->if_address = (char *)&sp->ds_addr[0];
	if_init_queues(ifp);
	return 1;
}

/*
 * pc586reset:
 *
 *	This routine is in part an entry point for the "if" code.  Since most 
 *	of the actual initialization has already (we hope already) been done
 *	by calling pc586attach().
 *
 * input	: unit number or board number to reset
 * output	: board is reset
 *
 */
int
pc586reset(unit)
int	unit;
{
	pc_softc[unit].ds_if.if_flags &= ~IFF_RUNNING;
	pc_softc[unit].flags &= ~(DSF_LOCK|DSF_RUNNING);
	return pc586init(unit);

}

/*
 * pc586init:
 *
 *	Another routine that interfaces the "if" layer to this driver.  
 *	Simply resets the structures that are used by "upper layers".  
 *	As well as calling pc586hwrst that does reset the pc586 board.
 *
 * input	: board number
 * output	: structures (if structs) and board are reset
 *
 */	
int
pc586init(unit)
int	unit;
{
	struct	ifnet	*ifp;
	int		stat;
	unsigned long	oldpri;

	ifp = &(pc_softc[unit].ds_if);
	oldpri = splimp();
	if ((stat = pc586hwrst(unit)) == TRUE) {
#ifdef WATCHDOG
		timeout(pc586watch, &(ifp->if_unit), pc586timo*hz);
		pc_softc[unit].timer = pc586timo;
#endif
#ifdef XMTWATCHDOG
		timeout(pc586xmtwatch, ifp->if_unit, pc586xmttimo*hz);
		pc_softc[unit].xmttimer = -1;
#endif
		pc_softc[unit].ds_if.if_flags |= IFF_RUNNING;
		pc_softc[unit].flags |= DSF_RUNNING;
		pc_softc[unit].tbusy = 0;
		pc586start(unit);
	} else
		printf("pc%d init(): trouble resetting board.\n", unit);
	splx(oldpri);
	return stat;
}


/*ARGSUSED*/
int
pc586open(dev, flag)
	dev_t	dev;
	int	flag;
{
	register int	unit;

	unit = minor(dev);	/* XXX */
	if (unit < 0 || unit >= NXLAN || !pc_softc[unit].seated)
	    return ENXIO;

	pc_softc[unit].ds_if.if_flags |= IFF_UP;
	pc586init(unit);
	return 0;
}


/*
 * pc586start:
 *
 *	This is yet another interface routine that simply tries to output a
 *	in an mbuf after a reset.
 *
 * input	: board number
 * output	: stuff sent to board if any there
 *
 */
int
pc586start(unit)
int	unit;
{
	io_req_t		m;
	struct	ifnet		*ifp;
	register pc_softc_t	*is = &pc_softc[unit];

#ifdef DEBUG
	if (pc586debug)
		printf("pc%d: start() busy %d\n", unit, is->tbusy);
#endif

	if (is->tbusy) {
		return 0;
	}

	ifp = &(pc_softc[unit].ds_if);
	IF_DEQUEUE(&ifp->if_snd, m);
	if (m) {
		is->tbusy++;
		xec596_xmt(unit, m);
	}
	return 1;
}

/*
 * Send a packet composed of an mbuf chain to the higher levels
 *
 */
int
pc586output(dev, ior)
	dev_t	dev;
	io_req_t ior;
{
	register int	unit;

	unit = minor(dev);	/* XXX */
	if (unit < 0 || unit >= NXLAN || !pc_softc[unit].seated)
	    return ENXIO;

	return net_write(&pc_softc[unit].ds_if, pc586start, ior);
}


int
pc586setinput(dev, receive_port, priority, filter, filter_count)
	dev_t		dev;
	mach_port_t	receive_port;
	int		priority;
	filter_t	filter[];
	unsigned int	filter_count;
{
	register int unit = minor(dev);
	if (unit < 0 || unit >= NXLAN || !pc_softc[unit].seated)
	    return ENXIO;

	return net_set_filter(&pc_softc[unit].ds_if,
			      receive_port, priority,
			      filter, filter_count);
}


int
pc586getstat(dev, flavor, status, count)
	dev_t	dev;
	int	flavor;
	dev_status_t	status;		/* pointer to OUT array */
	unsigned int	*count;		/* out */
{
	register int	unit = minor(dev);
	register pc_softc_t	*sp;

	if (unit < 0 || unit >= NXLAN || !pc_softc[unit].seated)
	    return ENXIO;

	sp = &pc_softc[unit];
	return net_getstat(&sp->ds_if, flavor, status, count);
}


int
pc586setstat(dev, flavor, status, count)
	dev_t	dev;
	int	flavor;
	dev_status_t	status;
	unsigned int	count;
{
	register int	unit = minor(dev);
	register pc_softc_t	*sp;

	if (unit < 0 || unit >= NXLAN || !pc_softc[unit].seated)
	    return ENXIO;

	sp = &pc_softc[unit];

	switch (flavor) {
	    case NET_STATUS:
	    {
		/*
		 * All we can change are flags, and not many of those.
		 */
		register struct net_status *ns = (struct net_status *)status;
		int	mode = 0;

		if (count < NET_STATUS_COUNT)
		    return D_INVALID_OPERATION;

		if (ns->flags & IFF_ALLMULTI)
		    mode |= MOD_ENAL;
		if (ns->flags & IFF_PROMISC)
		    mode |= MOD_PROM;

		/*
		 * Force a complete reset if the receive mode changes
		 * so that these take effect immediately.
		 */
		if (sp->mode != mode) {
		    sp->mode = mode;
		    if (sp->flags & DSF_RUNNING) {
			sp->flags &= ~(DSF_LOCK|DSF_RUNNING);
			pc586init(unit);
		    }
		}
		break;
	    }

	    default:
		return D_INVALID_OPERATION;
	}
	return D_SUCCESS;

}

/*
 * pc586hwrst:
 *
 *	This routine resets the pc586 board that corresponds to the 
 *	board number passed in.
 *
 * input	: board number to do a hardware reset
 * output	: board is reset
 *
 */
int
pc586hwrst(unit)
int unit;
{
	u_long data;
	volatile int *sram;
	volatile register struct core_lan *lanp;

	lanp = pc_softc[unit].hwaddr;
	/* turn off interrupt */
	aspitab(INT_LAN, SPLLAN, NULL, unit, 0);

	sram = (int *) (&pc_softc[unit].sram[OFFSET_SCP]);
	/* translate virtual address to physical address: */
	data = (unsigned) ltor(KERNSPC, sram) | 1;
#ifdef DEBUG
	if (pc586debug)
		printf("pc586hwrst: addr=0x%x, sram=%x(%x)\n",
		       lanp, sram, data);
#endif

	/* self test */
	sram[0] = -1;
	sram[1] = -1;
	fcacheline(KERNSPC, (caddr_t)sram);

	lanp->port = data >> 16;
	DELAY(1000);		/* XXX way too big? */
	lanp->port = data;
	DELAY(1000);		/* XXX way too big? */
	for (data = 1000000; sram[0] != -1 && data > 0; data--)
		pcacheline(KERNSPC, (caddr_t)sram);
#ifdef DEBUG
	if (pc586debug)
		printf("pc%d: self test = 0x%x:0x%x\n",
		       unit, sram[0], sram[1]);
#endif
	if (sram[1] != 0)
		return FALSE;
	if (pc586bldcu(unit) == FALSE)
		return FALSE;
#ifdef XMTWATCHDOG
	pc_softc[unit].xmttimer = -1;
#endif

	if (pc586diag(unit) == FALSE)
		return FALSE;

	if (pc586config(unit) == FALSE)
		return FALSE;
	/* 
	 * insert code for loopback test here
	 */
	pc586rustrt(unit);
	pc586inton(unit);
	return TRUE;
}

#ifdef WATCHDOG
/*
 * pc586watch():
 *
 *	This routine is the watchdog timer routine for the pc586 chip.  If
 *	chip wedges, this routine will fire and cause a board reset and
 *	begin again.
 *
 * input	: which board is timing out
 * output	: potential board reset if wedged
 *
 */
int watch_dead = 0;
pc586watch(b_ptr)
caddr_t	b_ptr;
{
	int	opri,
		unit = *b_ptr;

	if ((pc_softc[unit].ds_if.if_flags & IFF_UP) == 0)  {
		return;
	}

	opri = splx(SPLLAN);
	if (pc_softc[unit].timer == -1) {
		timeout(pc586watch, b_ptr, pc586timo*hz);
		splx(opri);
		return;
	}
	if (--pc_softc[unit].timer != -1) {
		timeout(pc586watch, b_ptr, 1*hz);
		splx(opri);
		return;
	}

#ifdef	notdef
	printf("pc%d watch(): %dsec timeout no %d\n",
	       unit, pc586timo, ++watch_dead);
#endif
	if (pc586hwrst(unit) != TRUE) {
		printf("pc%d watch(): hwrst trouble.\n", unit);
		pc_softc[unit].timer = 0;
	} else {
		timeout(pc586watch, b_ptr, 1*hz);
		pc_softc[unit].timer = pc586timo;
	}
	splx(opri);
	printf("pc%d: watchdog: %d second timeout #%d\n",
	       unit, pc586timo, ++watch_dead);
}
#endif

#ifdef XMTWATCHDOG
/*
 * pc586xmtwatch():
 *
 *	This routine is the watchdog timer routine for the transmitter
 *	on the 586 chip.  If the transmitter wedges, this routine will
 *	fire and restart it.
 */
int xmtwatch_dead = 0;

int
pc586xmtwatch(unit)
     int unit;
{
    register pc_softc_t *is = &pc_softc[unit];
    volatile scb_t *scb_p;
    int opri = 0;

    if ((is->ds_if.if_flags & IFF_UP) == 0)
      return 0;

    opri = splx(SPLLAN);
    if (is->xmttimer == -1)
      timeout(pc586xmtwatch, unit, pc586xmttimo*hz);
    else if (--is->xmttimer != -1)
      timeout(pc586xmtwatch, unit, 1*hz);
    else {
	scb_p = (volatile scb_t *)(is->sram + OFFSET_SCB);
	timeout(pc586xmtwatch, unit, 1*hz);
	purgescbstat(scb_p);
	if (scb_p->scb_status & 0x0700) {
	    printf("pc%d: XMT watchdog: %d second timeout #%d\n",
		   unit, pc586xmttimo, ++xmtwatch_dead);
	    /* reset HW? */
	}
	is->tbusy = 0;
	pc586start(unit);
    }
    splx(opri);
    return 1;
}
#endif

/*
 * pc586intr:
 *
 *	This function is the interrupt handler for the pc586 ethernet
 *	board.  This routine will be called whenever either a packet
 *	is received, or a packet has successfully been transfered and
 *	the unit is ready to transmit another packet.
 *
 * input	: board number that interrupted
 * output	: either a packet is received, or a packet is transfered
 *
 */
int
pc586intr(unit)
     int unit;
{
    register pc_softc_t *is = &pc_softc[unit];
    volatile scb_t	*scb_p = (volatile scb_t *)(is->sram + OFFSET_SCB);
    volatile ac_t	*cb_p  = (volatile ac_t *)(is->sram + OFFSET_CU);
    int			int_type;
    
#ifdef XEC596_DEBUG
    printf("pc%d intr()\n", unit);
#endif /* XEC596_DEBUG */
    
    if (is->seated == FALSE) { 
	printf("pc%d intr(): board not seated\n", unit);
	return -1;
    }
    
    purgescbstat(scb_p);
    while ((int_type = (scb_p->scb_status & SCB_SW_INT)) != 0) {
	pc586ack(unit);
	if (int_type & SCB_SW_FR) {
	    pc586rcv(unit);
	    ledcontrol(0, 0, LED_LANRCV);
	}
	if (int_type & SCB_SW_RNR) {
	    printf("pc%d intr(): receiver overrun! begin_fd = %x\n",
		   unit, is->begin_fd);
	    pc586rustrt(unit);
	}
	if (int_type & SCB_SW_CNA) {
	    /*
	     * At present, we don't care about CNA's.  We
	     * believe they are a side effect of XMT.
	     */
	}
	if (int_type & SCB_SW_CX) {
	    /*
	     * At present, we only request Interrupt for
	     * XMT.
	     */
	    pdcache(KERNSPC, cb_p, sizeof(*cb_p));
#ifdef	DEBUG
	    if (pc586debug)
	      printf("pc%d: intr type: %x status %x\n",
		     unit, int_type, cb_p->ac_status);
#endif
	    if ((!(cb_p->ac_status & AC_SW_OK)) ||
		(cb_p->ac_status & (0xfff^TC_SQE))) {
		if (cb_p->ac_status & TC_DEFER) {
		    if (xmt_watch) printf("DF");
		} else if (cb_p->ac_status & (TC_COLLISION|0xf)) {
		    if (xmt_watch) printf("%x",cb_p->ac_status & 0xf);
		} else {
		    if (xmt_watch) 
		      printf("pc%d XMT: %x %x\n",
			     unit, cb_p->ac_status,
			     cb_p->ac_command);
		    /*
		     * If transmitter claims to be busy,
		     * believe it.  This is probably a
		     * left-over interrupt as a result of
		     * the race condition in pc586start.
		     * DON'T restart the transmitter!
		     */
		    if (cb_p->ac_status == AC_SW_B) {
			goto xdone;
		    }
		    log(LOG_WARNING,
			"pc%d: XMT: bad status %x\n",
			unit, cb_p->ac_status);
		}
	    } else {
		ledcontrol(0, 0, LED_LANXMT);
	    }
#ifdef XMTWATCHDOG
	    is->xmttimer = -1;
#endif
	    is->tbusy = 0;
	    pc586start(unit);
	}
      xdone:
#ifdef WATCHDOG
	is->timer = pc586timo;
	watch_dead = 0;
#endif
	purgescbstat(scb_p);
    }
    return 0;
}


/*
 * pc586rcv:
 *
 *	This routine is called by the interrupt handler to initiate a
 *	packet transfer from the board to the "if" layer above this
 *	driver.  This routine checks if a buffer has been successfully
 *	received by the pc586.  If so, the routine xec596_read is called
 *	to do the actual transfer of the board data (including the
 *	ethernet header) into a packet (consisting of an mbuf chain).
 *
 * input	: number of the board to check
 * output	: if a packet is available, it is "sent up"
 *
 */
int
pc586rcv(unit)
     int unit;
{
    volatile fd_t *fd_p;
    register pc_softc_t *is = &pc_softc[unit];

    for (fd_p = is->begin_fd; fd_p != (fd_t *)NULL; fd_p = is->begin_fd) {
	pcacheline(KERNSPC, (caddr_t)fd_p);
	if (fd_p->status == 0xffff) {
	    if (pc586hwrst(unit) != TRUE) {
		printf("pc%d rcv(): hwrst ffff trouble.\n", unit);
	    } /* if */
	    return 0;
	} else if (fd_p->status & AC_SW_C) {
	    is->begin_fd = (fd_t *)ram_to_ptr(fd_p->link_offset, unit);

	    if (fd_p->status == (RFD_DONE|RFD_RSC)) {
		printf("pc%d RCV: lost packet %x\n", unit, fd_p->status);
	    } else if (!(fd_p->status & RFD_OK)) {
		printf("pc%d RCV: %x\n", unit, fd_p->status);
	    } else if (fd_p->status & 0xfff) {
		printf("pc%d RCV: %x\n", unit, fd_p->status);
	    } else if (!xec596_read(unit, fd_p)) {
		return 0;
	    } /* if */
	    if (!pc586requeue(unit, fd_p)) {	/* abort on chain error */
		if (pc586hwrst(unit) != TRUE) {
		    printf("pc%d rcv(): hwrst trouble.\n", unit);
		} /* if */
		return 0;
	    }
	} else {
	    break;
	} /* if */
    } /* for */
    return 1;
} /* pc586rcv */

/*
 * pc586requeue:
 *
 *	This routine puts rbd's used in the last receive back onto the
 *	free list for the next receive.
 *
 */
int
pc586requeue(unit, fd_p)
     int unit;
     volatile fd_t *fd_p;
{
    rbd_t	*l_rbdp;
    rbd_t	*f_rbdp;

#ifdef REQUEUE_DBG
    if (bad_rbd_chain(fd_p->rbd_offset, unit))
      return 0;
#endif
    f_rbdp = (rbd_t *)ram_to_ptr(fd_p->rbd_offset, unit);
    if (f_rbdp != NULL) {
	l_rbdp = f_rbdp;
	pcacheline(KERNSPC, (caddr_t)l_rbdp);
	while (!(l_rbdp->status & RBD_SW_EOF) &&
	       l_rbdp->next_rbd_offset != 0xffff)
	{
	    l_rbdp->status = 0;
	    fcacheline(KERNSPC, (caddr_t)l_rbdp);
	    l_rbdp = (rbd_t *)ram_to_ptr(l_rbdp->next_rbd_offset,
					 unit);
	    pcacheline(KERNSPC, (caddr_t)l_rbdp);
	}
	l_rbdp->next_rbd_offset = PC586NULL;
	l_rbdp->status = 0;
	l_rbdp->size |= AC_CW_EL;
	fcacheline(KERNSPC, (caddr_t)l_rbdp);
	pc_softc[unit].end_rbd->next_rbd_offset = 
	  ptr_to_ram((char *)f_rbdp, unit);
	pc_softc[unit].end_rbd->size &= ~AC_CW_EL;
	fcacheline(KERNSPC, (caddr_t)pc_softc[unit].end_rbd);
	pc_softc[unit].end_rbd= l_rbdp;
    }

    fd_p->status = 0;
    fd_p->command = AC_CW_EL;
    fd_p->link_offset = PC586NULL;
    fd_p->rbd_offset = PC586NULL;
    fcacheline(KERNSPC, (caddr_t)fd_p);

    pc_softc[unit].end_fd->link_offset = ptr_to_ram((char *)fd_p, unit);
    pc_softc[unit].end_fd->command = 0;
    fcacheline(KERNSPC, (caddr_t)pc_softc[unit].end_fd);
    pc_softc[unit].end_fd = fd_p;

    return 1;
}

/*
 * pc586bldcu:
 *
 *	This function builds up the command unit structures.  It inits
 *	the scp, iscp, scb, cb, tbd, and tbuf.
 *
 */
int
pc586bldcu(unit)
{
    volatile char *sram = pc_softc[unit].sram;
    scp_t		*scp_p = (scp_t *)(sram + OFFSET_SCP);
    iscp_t		*iscp_p = (iscp_t *)(sram + OFFSET_ISCP);
    volatile scb_t	*scb_p = (volatile scb_t *)(sram + OFFSET_SCB);
    volatile ac_t	*cb_p = (volatile ac_t *)(sram + OFFSET_CU);
    tbd_t		*tbd_p = (tbd_t *)(sram + OFFSET_TBD);
    int		i;
    
    scp_p->scp_sysbus = 0x68;
    i = (int)ltor(KERNSPC, iscp_p);
    scp_p->scp_iscp_lo = i;
    scp_p->scp_iscp_hi = i >> 16;
    scp_p->scp_unused1 = 0;
    scp_p->scp_unused2 = 0;
    
    iscp_p->iscp_busy = 1;
    iscp_p->iscp_scb_offset = OFFSET_SCB;
    i = (int)ltor(KERNSPC, sram);
    iscp_p->iscp_scb_lo = i;
    iscp_p->iscp_scb_hi = i >> 16;
    
    purgescball(scb_p);
    scb_p->scb_status = 0;
    scb_p->scb_command = 0;
    scb_p->scb_cbl_offset = OFFSET_CU;
    scb_p->scb_rfa_offset = OFFSET_RU;
    scb_p->scb_crcerrs = 0;
    scb_p->scb_alnerrs = 0;
    scb_p->scb_rscerrs = 0;
    scb_p->scb_ovrnerrs = 0;
    
    cb_p->ac_status = 0;
    cb_p->ac_command = (AC_CW_EL|AC_NOP);
    cb_p->ac_link_offset = OFFSET_CU;
    
    tbd_p->act_count = 0;
    tbd_p->next_tbd_offset = PC586NULL;
    tbd_p->buffer_addr = 0;
    tbd_p->buffer_base = 0;
    
    i = ltor(KERNSPC, scp_p) | 0x2;
    fdcache(KERNSPC, pc_softc[unit].sram, LAN_BUFF_SIZE);
    pc_softc[unit].hwaddr->port = (u_long)i >> 16;
    DELAY(1000);
    pc_softc[unit].hwaddr->port = (u_long)i;
    DELAY(1000);
    pc586chatt(unit);
    for (i = 1000000; iscp_p->iscp_busy && i > 0; i--)
      pcacheline(KERNSPC, (caddr_t)iscp_p);
#ifdef DEBUG
    if (pc586debug)
      printf("pc%d: iscp status = %x\n", unit, iscp_p->iscp_busy);
#endif
    if (!i) {
	printf("pc%d bldcu(): iscp_busy timeout.\n", unit);
	return FALSE;
    }
    for (i = STATUS_TRIES; i > 0; i--) {
	if (scb_p->scb_status == (SCB_SW_CX|SCB_SW_CNA)) 
	  break;
	purgescbstat(scb_p);
    }
#ifdef DEBUG
    if (pc586debug)
      printf("pc%d: scb status = %x:%x %d\n",
	     unit, scb_p->scb_status, scb_p->scb_command, i);
#endif
    if (!i) {
	printf("pc%d bldcu(): not ready after reset.\n", unit);
	return FALSE;
    }
    pc586ack(unit);
    return TRUE;
}

/*
 * pc586bldru:
 *
 *	This function builds the linear linked lists of fd's and
 *	rbd's.  Based on page 4-32 of 1986 Intel microcom handbook.
 *
 */
char *
pc586bldru(unit)
int unit;
{
	fd_t	*fd_p = (fd_t *)(pc_softc[unit].sram + OFFSET_RU);
	ru_t	*rbd_p = (ru_t *)(pc_softc[unit].sram + OFFSET_RBD);
	int 	i;

	pc_softc[unit].begin_fd = fd_p;
	for(i = 0; i < N_FD; i++, fd_p++) {
		fd_p->status = 0;
		fd_p->command	= 0;
		fd_p->link_offset = ptr_to_ram((char *)(fd_p + 1), unit);
		fd_p->rbd_offset = PC586NULL;
	}
	pc_softc[unit].end_fd = --fd_p;
	fd_p->link_offset = PC586NULL;
	fd_p->command = AC_CW_EL;
	fd_p = (fd_t *)(pc_softc[unit].sram + OFFSET_RU);

	fd_p->rbd_offset = ptr_to_ram((char *)rbd_p, unit);
	for(i = 0; i < N_RBD; i++, rbd_p++) {
		rbd_p->r.status = 0;
		rbd_p->r.buffer_addr = (u_short)((int)rbd_p->rbuffer & 0xffff);
		rbd_p->r.buffer_base = (int)rbd_p->rbuffer >> 16;
#ifdef DEBUG
		if (pc586debug)
			printf("pc%d: create rbuf #4 base=%x addr=%x\n",
			       unit, rbd_p->r.buffer_base,
			       rbd_p->r.buffer_addr);
#endif
		rbd_p->r.size = RCVBUFSIZE;
		if (i != N_RBD-1) {
			rbd_p->r.next_rbd_offset =
				ptr_to_ram((char *)(rbd_p + 1), unit);
		} else {
			rbd_p->r.next_rbd_offset = PC586NULL;
			rbd_p->r.size |= AC_CW_EL;
			pc_softc[unit].end_rbd = (rbd_t *)rbd_p;
		}
	}
	return (char *)pc_softc[unit].begin_fd;
}

/*
 * pc586rustrt:
 *
 *	This routine starts the receive unit running.  First checks if the
 *	board is actually ready, then the board is instructed to receive
 *	packets again.
 *
 */
int
pc586rustrt(unit)
int unit;
{
    volatile scb_t *scb_p =
      (volatile scb_t *) (pc_softc[unit].sram + OFFSET_SCB);
    char *strt;
    
    purgescball(scb_p);
    if ((scb_p->scb_status & SCB_RUS_READY) == SCB_RUS_READY)
      return 1;
    
    strt = pc586bldru(unit);
    scb_p->scb_command = SCB_RU_STRT;
    scb_p->scb_rfa_offset = ptr_to_ram(strt, unit);
    fdcache(KERNSPC, pc_softc[unit].sram, LAN_BUFF_SIZE);
    pc586chatt(unit);
    return 1;
}

/*
 * pc586diag:
 *
 *	This routine does a 586 op-code number 7, and obtains the
 *	diagnose status for the pc586.
 *
 */
int
pc586diag(unit)
int unit;
{
    volatile scb_t *scb_p =
	  (volatile scb_t *)(pc_softc[unit].sram + OFFSET_SCB);
    volatile ac_t *cb_p  = (volatile ac_t *)(pc_softc[unit].sram + OFFSET_CU);
    int i;
    
    purgescball(scb_p);
    if (scb_p->scb_status & SCB_SW_INT) {
	printf("pc%d diag(): bad initial state %\n",
	       unit, scb_p->scb_status);
	pc586ack(unit);
    }
    cb_p->ac_status	= 0;
    cb_p->ac_command = (AC_DIAGNOSE|AC_CW_EL);
    fdcache(KERNSPC, cb_p, sizeof *cb_p);
    scb_p->scb_command = SCB_CU_STRT;
    flushscbcmd(scb_p);
    pc586chatt(unit);
    for (i = 0; i < 0xffff; i++) {
	if ((cb_p->ac_status & AC_SW_C))
	  break;
	pdcache(KERNSPC, cb_p, sizeof *cb_p);
    }
    if (i == 0xffff || !(cb_p->ac_status & AC_SW_OK)) {
	printf("pc%d: diag failed; status = %x, command = %x\n",
	       unit, cb_p->ac_status, cb_p->ac_command);
	return FALSE;
    }
    purgescbstat(scb_p);
    if ((scb_p->scb_status & SCB_SW_INT) && (scb_p->scb_status != SCB_SW_CNA))
    {
	printf("pc%d diag(): bad final state %x\n",
	       unit, scb_p->scb_status);
	pc586ack(unit);
    }
    return TRUE;
}

/*
 * pc586config:
 *
 *	This routine does a standard config of the pc586 board.
 *
 */
int
pc586config(unit)
int unit;
{
    volatile scb_t *scb_p =
      (volatile scb_t *)(pc_softc[unit].sram + OFFSET_SCB);
    volatile ac_t *cb_p	=
      (volatile ac_t *)(pc_softc[unit].sram + OFFSET_CU);
    int i;
    
    
    purgescball(scb_p);
    if ((scb_p->scb_status != SCB_SW_CNA) && (scb_p->scb_status & SCB_SW_INT))
    {
	printf("pc%d config(): unexpected initial state %x\n",
	       unit, scb_p->scb_status);
    }
    pc586ack(unit);
    
#if 0
    cb_p->ac_status	= 0;
    cb_p->ac_command = (AC_CONFIGURE|AC_CW_EL);
    
    /*
     * below is the default board configuration from p2-28 from 586 book
     */
    cb_p->cmd.configure.fifolim_bytecnt 	= 0x080c;
    cb_p->cmd.configure.addrlen_mode  	= 0x2600;
    cb_p->cmd.configure.linprio_interframe	= 0x6000;
    cb_p->cmd.configure.slot_time      	= 0xf200;
    cb_p->cmd.configure.hardware     	= 0x0000;
    cb_p->cmd.configure.min_frame_len   	= 0x0040;
    fdcache(KERNSPC, cb_p, sizeof(*cb_p));
    
    scb_p->scb_command = SCB_CU_STRT;
    flushscbcmd(scb_p);
    pc586chatt(unit);
    
    for (i = 0; i < 0xffff; i++) {
	if ((cb_p->ac_status & AC_SW_C))
	  break;
	pdcache(KERNSPC, cb_p, sizeof(*cb_p));
    }
    if (i == 0xffff || !(cb_p->ac_status & AC_SW_OK)) {
	printf("pc%d: config-configure failed; status = %x\n",
	       unit, cb_p->ac_status);
	return FALSE;
    }
    purgescbstat(scb_p);
    if (scb_p->scb_status & SCB_SW_INT) {
	printf("pc%d configure(): bad configure state %x\n",
	       unit, scb_p->scb_status);
	pc586ack(unit);
    }
#endif
    cb_p->ac_status = 0;
    cb_p->ac_command = (AC_IASETUP|AC_CW_EL);
    
    bcopy(pc_softc[unit].ds_addr, cb_p->cmd.iasetup, ETHER_ADD_SIZE);
    fdcache(KERNSPC, cb_p, sizeof(*cb_p));
    
    scb_p->scb_command = SCB_CU_STRT;
    flushscbcmd(scb_p);
    pc586chatt(unit);
    
    for (i = 0; i < 0xffff; i++) {
	if ((cb_p->ac_status & AC_SW_C))
	  break;
	pdcache(KERNSPC, cb_p, sizeof *cb_p);
    }
    if (i == 0xffff || !(cb_p->ac_status & AC_SW_OK)) {
	printf("pc%d: config-address failed; status = %x\n",
	       unit, cb_p->ac_status);
	return FALSE;
    }
    
    purgescbstat(scb_p);
    if ((scb_p->scb_status & SCB_SW_INT) != SCB_SW_CNA) {
	printf("pc%d configure(): unexpected final state %x\n",
	       unit, scb_p->scb_status);
    }
    pc586ack(unit);
    
    return TRUE;
}

/*
 * pc586ack:
 */
int
pc586ack(unit)
{
    volatile scb_t *scb_p =
      (volatile scb_t *)(pc_softc[unit].sram + OFFSET_SCB);
    int i;

    purgescball(scb_p);
    scb_p->scb_command = (scb_p->scb_status & SCB_SW_INT);
    flushscbcmd(scb_p);
    if (!scb_p->scb_command) {
	return 0;
    } /* if */
    pc586chatt(unit);
    i = 1000000;
    do {
	purgescbcmd(scb_p);
    } while (scb_p->scb_command && i-- > 0);
    if (!i) {
	printf("pc%d pc586ack(): board not accepting command.\n", unit);
    } /* if */
    return 1;
}


volatile char *
ram_to_ptr(int offset, int unit)
{
    if (offset == PC586NULL)
      return NULL;
    if (offset > 0x3fff) {
	printf("pc%d: ram_to_ptr(%x, %d)\n", unit, offset, unit);
	panic("range");
	return NULL;
    }
    return pc_softc[unit].sram + offset;
}


#ifdef REQUEUE_DBG
int
bad_rbd_chain(offset, unit)
{
    rbd_t *rbdp;
    volatile char *sram = pc_softc[unit].sram;

    for (;;) {
	if (offset == PC586NULL)
	  return 0;
	if (offset > 0x3fff) {
	    printf("pc%d: bad_rbd_chain offset = %x\n",
		   unit, offset);
	    return 1;
	}

	rbdp = (rbd_t *)(sram + offset);
	pcacheline(KERNSPC, (caddr_t)rbdp);
	offset = rbdp->next_rbd_offset;
    }
    return 0;
}
#endif

int
ptr_to_ram(k_va, unit)
char *k_va;
int unit;
{
    return k_va - pc_softc[unit].sram;
}


#ifdef DEBUG
int
pc586scb(unit)
{
    volatile scb_t *scb = (volatile scb_t *)(pc_softc[unit].sram + OFFSET_SCB);
    u_short i;
    
    purgescball(scb);
    i = scb->scb_status;
    printf("stat: stat %x, cus %x, rus %x //",
	   (i&0xf000)>>12, (i&0x0700)>>8, (i&0x0070)>>4);
    i = scb->scb_command;
    printf(" cmd: ack %x, cuc %x, ruc %x\n",
	   (i&0xf000)>>12, (i&0x0700)>>8, (i&0x0070)>>4);
    
    printf("cbl %x, rfa %x //", scb->scb_cbl_offset, scb->scb_rfa_offset);
}


int
pc586tbd(unit)
{
	pc_softc_t	*is = &pc_softc[unit];
	tbd_t		*tbd_p = (tbd_t *)(is->sram + OFFSET_TBD);
	int 		i = 0;
	int		sum = 0;

	do {
		pcacheline(KERNSPC, (caddr_t)tbd_p);
		sum += (tbd_p->act_count & ~TBD_SW_EOF);
		printf("pc%d: %d: addr=%x count=%d(%d) next=%x base=%x\n",
		       unit, i++, tbd_p->buffer_addr,
		       (tbd_p->act_count & ~TBD_SW_EOF), sum,
		       tbd_p->next_tbd_offset,
		       tbd_p->buffer_base);
		if (tbd_p->act_count & TBD_SW_EOF)
			break;
		tbd_p = (tbd_t *)(is->sram + tbd_p->next_tbd_offset);
	} while (1);
	dumpbytes((u_char *)(is->sram + OFFSET_TBUF), sum);
}


int
dumpehdr(eh)
	register struct ether_header *eh;
{
	printf("type=0x%x", eh->ether_type);
	printf(" daddr=%s", ether_sprintf(eh->ether_dhost));
	printf(" saddr=%s", ether_sprintf(eh->ether_shost));
}


int
dumpbytes(dp, len)
	register u_char *dp;
	register int len;
{
	static char digits[] = "0123456789ABCDEF";
	char cbuf[3];
	register int i;

	cbuf[2] = 0;
	for (i = 0; i < len; i++) {
		cbuf[0] = digits[*dp >> 4];
		cbuf[1] = digits[*dp++ & 0xF];
		printf("%s%c", cbuf, (i%32)==31 ? '\n' : ' ');
	}
	if (i%32)
		printf("\n");
}
#endif	/* DEBUG */

/*
 * x-kernel interface for Ethernet driver:
 */

/****************** utility operations ******************/

static bool
copyout(src, len, arg)
     char *src;
     long len;
     void *arg;
{
    void **dst = (void**)arg;

#ifdef XEC596_DEBUG
    printf("xec596: copyout(src=%x,dst=%x len=%d)\n", src, *dst, len);
#endif /* XEC596_DEBUG */
    bcopy(src, *dst, len);
    *dst += len;
    return TRUE;
} /* copyout */


/*
 * This routine fills in the appropriate registers and memory
 * locations on the i82596 chip and starts the it off on the
 * transmit.
 *
 * ATTENTION: This routine can be called from the interrupt handler
 *	      so be careful when using x-kernel operations.  To clean
 *	      up things, you can use xmt_done() which is called by
 *	      a normal kernel thread, so blocking is ok there.
 */
static void
xec596_xmt(int unit, io_req_t ior)
{
    pc_softc_t *is = &pc_softc[unit];
    u_char *xmtdata_p = (u_char *)(is->sram + OFFSET_TBUF);
    register Msg *msg = (Msg*) ior->io_data;
    ETHhdr  *ehp = (ETHhdr *)msgGetAttr(msg, 0);
    volatile scb_t *scb_p = (volatile scb_t *)(is->sram + OFFSET_SCB);
    volatile ac_t *cb_p = (volatile ac_t *)(is->sram + OFFSET_CU);
    tbd_t *tbd_p = (tbd_t *)(is->sram + OFFSET_TBD);
    int len;

    cb_p->ac_status = 0;
    cb_p->ac_command = (AC_CW_EL|AC_TRANSMIT|AC_CW_I);
    cb_p->ac_link_offset = PC586NULL;
    cb_p->cmd.transmit.tbd_offset = OFFSET_TBD;

#ifdef XEC596_DEBUG
    printf("xec596_xmt: src=[%04x:%04x:%04x], dst=[%04x:%04x:%04x]\n",
	   ehp->src.high, ehp->src.mid, ehp->src.low,
	   ehp->dst.high, ehp->dst.mid, ehp->dst.low);
#endif /* XEC596_DEBUG */

    bcopy(&ehp->dst, cb_p->cmd.transmit.dest_addr, ETHER_ADD_SIZE);
    cb_p->cmd.transmit.length = (u_short)(ehp->type);
    fdcache(KERNSPC, (caddr_t)cb_p, sizeof *cb_p);

    tbd_p->act_count = 0;
    tbd_p->buffer_addr = (u_short)((int)xmtdata_p & 0xffff);
    tbd_p->buffer_base = (int)xmtdata_p >> 16;
    /*
     *		Linearize message.
     * WARNING: xmtdata_p is modified in copyout:
     */
    msgForEach(msg, copyout, &xmtdata_p);
    tbd_p->act_count = len = msgLen(msg);
#ifdef XEC596_DEBUG
    printf("xec596_xmt: total msg len=%d\n", len);
#endif /* XEC596_DEBUG */

    /* if packet is shorter than ETHERMIN pad with 0s: */
    if (len < ETHERMIN) {
	bzero(xmtdata_p, ETHERMIN - len);
	tbd_p->act_count = ETHERMIN;
    } /* if */
    tbd_p->act_count |= TBD_SW_EOF;
    tbd_p->next_tbd_offset = PC586NULL;
    fcacheline(KERNSPC, (caddr_t)tbd_p);

    fdcache(KERNSPC, (caddr_t)(is->sram + OFFSET_TBUF), len);

    do {
	purgescbcmd(scb_p);
    } while (scb_p->scb_command);
    scb_p->scb_command = SCB_CU_STRT;
    flushscbcmd(scb_p);

    pc586chatt(unit);
#ifdef XMTWATCHDOG
    is->xmttimer = pc586xmttimo;
    xmtwatch_dead = 0;
#endif
    iodone(ior);
} /* xec596_xmt */


/*
 * Copy a packet from the i82596 chip.  Returns 1 on success, 0 on failure.
 */
static int
copy_from_596(int unit, volatile fd_t *fd_p, ETHhdr *ehp, char *dp)
{
    volatile rbd_t *rbd_p;
    volatile u_char *buffer_p;
    int bytes_in_msg;
    int len;

#ifdef XEC596_DEBUG
    printf("copy_from_596()\n");
#endif /* XEC596_DEBUG */

    /* get ether header: */
    ehp->type = fd_p->length;
    bcopy(fd_p->source, &ehp->src, ETHER_ADD_SIZE);
    bcopy(fd_p->destination, &ehp->dst, ETHER_ADD_SIZE);

#ifdef XEC596_DEBUG
    printf("copy_from_596: src=[%04x:%04x:%04x], dst=[%04x:%04x:%04x]\n",
	   ehp->src.high, ehp->src.mid, ehp->src.low,
	   ehp->dst.high, ehp->dst.mid, ehp->dst.low);
#endif /* XEC596_DEBUG */

    /* get packet body: */
    rbd_p = (volatile rbd_t *)ram_to_ptr(fd_p->rbd_offset, unit);
    if (!rbd_p) {
	printf("xec%d read(): Invalid buffer\n", unit);
	if (pc586hwrst(unit) != TRUE) {
	    printf("xec%d read(): hwrst trouble.\n", unit);
	} /* if */
	return 0;
    } /* if */

    len = 0;
    do {
	/* copy in one receive buffer: */
	pcacheline(KERNSPC, (caddr_t)rbd_p);
	buffer_p = (volatile u_char*) (rbd_p->buffer_base << 16) +
	  rbd_p->buffer_addr;
	bytes_in_msg = rbd_p->status & RBD_SW_COUNT;
	pdcache(KERNSPC, (caddr_t)buffer_p, bytes_in_msg);

#ifdef XEC596_DEBUG
	printf("copy_from_596: from=0x%08x, to=0x%08x, len=%d\n",
	       rbd_p, dp, bytes_in_msg);
#endif /* XEC596_DEBUG */

	bcopy(buffer_p, dp, bytes_in_msg);

	len += bytes_in_msg;
	dp += bytes_in_msg;
	if (rbd_p->status & RBD_SW_EOF) {
	    break;
	} /* if */
	/* follow receive buffer chain: */
	rbd_p = (volatile rbd_t*) ram_to_ptr(rbd_p->next_rbd_offset, unit);
    } while ((int) rbd_p);

#ifdef XEC596_DEBUG
    printf("copy_from_596: returning successfully\n");
#endif /* XEC596_DEBUG */

    return len;
} /* copy_from_596 */


/*
 * This routine does the actual copy of data (including ethernet header
 * structure) from the pc586 to an mbuf chain that will be passed up
 * to the "if" (network interface) layer.  NOTE:  we currently
 * don't handle trailer protocols, so if that is needed, it will
 * (at least in part) be added here.  For simplicities sake, this
 * routine copies the receive buffers from the board into a local (stack)
 * buffer until the frame has been copied from the board.  Once in
 * the local buffer, the contents are copied to an mbuf chain that
 * is then enqueued onto the appropriate "if" queue.
 */
static int
xec596_read(int unit, volatile fd_t *fd_p)
{
    register pc_softc_t *is = &pc_softc[unit];
    register struct ifnet *ifp = &is->ds_if;
    struct threadIBlock *ti;
    struct input_buffer *buf;
    XObj self = self_protocol[unit];
    int len;

    if ((ifp->if_flags & (IFF_UP|IFF_RUNNING)) != (IFF_UP|IFF_RUNNING)) {
	xError("xec596_read(): board is not running");
	pc586intoff(ifp->if_unit);
    } /* if */

    /* try to find a free shepherd thread: */
    ti = (struct threadIBlock*) delist_head(&xkThreadQ);
    if (ti) {
	ti->buf = buf = (struct input_buffer*) delist_head(&xkBufferPool);
	if (buf) {
	    buf->driverProtl = self;
	    len = copy_from_596(unit, fd_p, (ETHhdr*) &buf->hdr, buf->data);
	    if (len > 0) {
		msgSetAttr(&buf->upmsg, 0, &buf->hdr, sizeof(ETHhdr));
		msgTruncate(&buf->upmsg, len);
	    } else {
		xError("xec596: short frame received");
		/* make sure thread will ignore frame: */
		ti->buf = NULL;
	    } /* if */
	} else {
	    xError("xec596: dropped incoming msg (got thread but no buffer)");
	    /* 
	     * We'll restart the thread anyway.  It will find no
	     * buffer attached to its structure and put itself back
	     * on the queue. 
	     */
	} /* if */
	/* awake shepherd: */
	clear_wait(ti->thread, THREAD_AWAKENED, FALSE);
    } else {
	/*
	 * No thread available.  Try to put the message on the
	 * incomingData queue:
	 */
	if (xkIncomingData_lock) {
	    xError("xec596: dropped incoming msg (incomingData queue locked)");
	} else {
	    buf = (struct input_buffer*) delist_head(&xkBufferPool);
	    if (buf) {
		buf->driverProtl = self;
		len = copy_from_596(unit, fd_p, (ETHhdr*) &buf->hdr,
				    buf->data);
		if (len > 0) {
		    msgSetAttr(&buf->upmsg, 0, &buf->hdr, sizeof(ETHhdr));
		    msgTruncate(&buf->upmsg, len);
		    enlist_tail((list_t) &xkIncomingData, (list_entry_t) buf);
		} else {
		    /* cannot put back buffer... */
		    panic("xec596: short frame received");
		} /* if */
	    } else {
		xError("xec596: dropped incoming msg (no thread, no buffer)");
	    } /* if */
	} /* if */
    } /* if */
    return 1;
} /* xec596_read */


/*
 * Enqueue I/O request "ior" on interface "ifp".  Invoked "start" after
 * enqeueing message to get things started.
 */
static io_return_t
xec596_net_write(ifp, start, ior)
     register struct ifnet *ifp;
     int (*start)();
     io_req_t ior;
{
    int s;

    /*
     * Reject the write if the interface is down.
     */
    if ((ifp->if_flags & (IFF_UP|IFF_RUNNING)) != (IFF_UP|IFF_RUNNING)) {
	return D_DEVICE_DOWN;
    } /* if */

    /*
     * Reject the write if the packet is too large or too small.
     */
    if (ior->io_count < ifp->if_header_size ||
	ior->io_count > ifp->if_header_size + ifp->if_mtu)
    {
	return D_INVALID_SIZE;
    } /* if */

    /*
     * Queue the packet on the output queue, and
     * start the device.
     */
    xTrace0(xec596p, TR_DETAILED, "xec596: write packet");

    s = splimp();
    IF_ENQUEUE(&ifp->if_snd, ior);
    (*start)(ifp->if_unit);
    splx(s);

    return D_IO_QUEUED;
} /* xec596_net_write */


/*
 * This function is invoked by the io_done_thread (see device/ds_routines.c).
 * Therefore it is safe to block while trying to acquire the x-kernel
 * master lock.
 */
static int
xmt_done(ior)
     io_req_t ior;
{
    Msg *msg = (Msg*) ior->io_data;
    char *msg_attr;

    /* destroy msg & free allocated space: */
    xk_master_lock();
    msg_attr = msgGetAttr(msg, 0);
    xFree(msg_attr);
    msgDestroy(msg);
    xFree((char*) msg);
    xk_master_unlock();
    return TRUE;
} /* xmt_done */


static void
get_local_eth_addr(ETHhost *host, int unit)
{
    pc_softc_t *sp = &pc_softc[unit];

    bcopy(sp->ds_addr, (char*) host, sizeof(ETHhost));

    xTrace4(xec596p, TR_EVENTS,
	    "get_local_eth_addr: xec596 unit %d host addr = [%02x:%02x:%02x]",
	    unit, host->high, host->mid, host->low);
} /* get_local_eth_addr */

/****************** protocol operations ******************/

/*
 * Initialize driver:
 */
xkern_return_t
xec596_init(XObj self)
{
    extern kalloc_max;
    int unit;

    xTrace0(xec596p, TR_FUNCTIONAL_TRACE, "xec596_init()");

    /*
     * Don't want to try to get pages beyond what kalloc can
     * conveniently work with; larger pages get wired down memory:
     */
    if (PageSize > kalloc_max) {
	panic("xec596: xkernel msg pages too big");
    } /* if */

    if (PageSize < MTU) {
	panic("xec596: xkernel msg pages too small for network MTU");
    } /* if */

    unit = self->instName[strlen(self->instName)-1] - '0';
    if (unit < 0 || unit >= NXLAN) {
	unit = 0;
	printf("xec596: unit number out of range, assuming unit 0 for %s\n",
	       self->instName);
    } /* if */

    self_protocol[unit] = self;
    XEC596_UNIT(self) = unit;

    xTrace1(xec596p, TR_EVENTS, "xec596_init: opening unit %d", unit);

    /* invoke open operation: */
    pc586open((dev_t)unit, 0);

    /* setup protocol function pointers: */
    self->openenable = xec596_openenable;
    self->push = xec596_push;
    self->control = xec596_control;

    xTrace0(xec596p, TR_FUNCTIONAL_TRACE, "xec596_init: returning XK_SUCCESS");

    return XK_SUCCESS;
} /* xec596_init */


/*
 * Control operations:
 */
static int
xec596_control(XObj self, int op, char *buf, int len)
{
    int unit;
    int retval = -1;
    int *ival;

    xTrace1(xec596p, TR_FUNCTIONAL_TRACE, "xec596_control(op=%x)", op);

    unit = XEC596_UNIT(self);
    if (unit < 0 || unit >= NXLAN) {
	printf(
	       "xec596: unit number out of range, assuming unit 0 for %s\n",
	       self->instName);
	unit = 0;
    } /* if */

    switch (op) {
      case GETMYHOST:
        if (len >= sizeof(ETHhost)) {
	    get_local_eth_addr((ETHhost*)buf, unit);
	    retval = sizeof(ETHhost);
	} /* if */
	break;

      case GETMAXPACKET:
	if (len >= sizeof(int)) {
	    ival = (int*)buf;
	    *ival = MTU;
	    retval = sizeof(int);
	} /* if */
	break;

      default:
	break;
    } /* switch */

    return retval;
} /* xec596_control */


/*
 * The openenable interface allows the driver get the address of the
 * higher level protocol to which it interfaces.
 */
static xkern_return_t
xec596_openenable(XObj self, XObj hlp, XObj hlptype, Part *part)
{
    int unit;

    xTrace0(xec596p, TR_FUNCTIONAL_TRACE, "xec596_openenable()");

    unit = XEC596_UNIT(self);
    if (unit < 0 || unit >= NXLAN) {
	printf("xec596: unit number out of range, assuming unit 0 for %s\n",
	       self->instName);
	unit = 0;
    } /* if */
    self_protocol[unit] = self;

    /* store high-level protocol in up pointer: */
    xSetUp(self, hlp);

    xTrace0(xec596p, TR_FUNCTIONAL_TRACE,
	    "xec596_openenable: returning XK_SUCCESS");

    return XK_SUCCESS;
} /* xec596_openenable */


/*
 * Send a message out to ether.  This works by setting up an io request
 * which is then enqueued in the drivers send queue.
 */
static xmsg_handle_t
xec596_push(XObj s, Msg *downMsg)
{
    ETHhdr  *hdr = (ETHhdr *)msgGetAttr(downMsg, 0);
    io_req_t ior;
    char *msg_attr;

    xTrace1(xec596p, TR_FUNCTIONAL_TRACE, "xec596_push(len=%d)",
	    msgLen(downMsg));

    /* allocate io request: */
    io_req_alloc(ior, 0);

    /* let io_data point to the message: */
    ior->io_data = (char *)xMalloc(sizeof(Msg));
    msgConstructCopy((Msg*)ior->io_data, downMsg);

    msg_attr = xMalloc(sizeof(ETHhdr));
    bcopy((char *)hdr, msg_attr, sizeof(ETHhdr));
    msgSetAttr((Msg*)ior->io_data, 0, msg_attr, sizeof(ETHhdr));

    ior->io_count = msgLen(downMsg) + sizeof(ETHhdr);

    /* operation is write and call xmt_done when done */
    ior->io_op = IO_WRITE | IO_CALL;
    ior->io_done = xmt_done;
    ior->io_unit = XEC596_UNIT(s);

    /* simulate an inband pc586output(ior->io_unit, ior): */

    if (xec596_net_write(&pc_softc[ior->io_unit].ds_if, pc586start, ior)
	!= D_IO_QUEUED)
    {
	panic( "xec596: xec596_net_write() failed");
    } /* if */

    xTrace0(xec596p, TR_FUNCTIONAL_TRACE,
	    "xec596_push: returning successfully");

    return XMSG_NULL_HANDLE;
} /* xec596_push */

#endif	/* NXLAN > 0 */

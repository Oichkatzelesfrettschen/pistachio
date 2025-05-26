/* 
 * Mach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
 * Copyright (c) 1992 Helsinki University of Technology
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON AND HELSINKI UNIVERSITY OF TECHNOLOGY ALLOW FREE USE
 * OF THIS SOFTWARE IN ITS "AS IS" CONDITION.  CARNEGIE MELLON AND
 * HELSINKI UNIVERSITY OF TECHNOLOGY DISCLAIM ANY LIABILITY OF ANY KIND
 * FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
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
 * 06-Dec-92  Ian Dall (ian) at University of Adelaide
 *	Remove #if 0 code and some printfs.
 *
 * 21-Nov-92  Ian Dall (ian) at University of Adelaide
 *	Be more careful we don't write over the end of the tr and trname
 *	arrays.
 *
 * 19-Nov-92  Ian Dall (ian) at University of Adelaide
 *	Clear SDP_EMR_ARB bit when a reconnection collides with a
 *	selection.
 *
 * 12-Nov-92  Ian Dall (ian) at University of Adelaide
 *	Remove lots of unnecessary SDP_CLR_INTR's.
 *
 * 11-Nov-92  Ian Dall (ian) at University of Adelaide
 *	Add new SDP_STATE_DMA_DONE bit to ensure that the residual
 *	adjustment only gets done once at the end of each transfer.
 *
 * 10-Nov-92  Ian Dall (ian) at University of Adelaide
 *	Make new SDP_CLR_PEND_INTR macro which works like the old
 *	SDP_CLR_INTR macro.
 *
 * 09-Nov-92  Ian Dall (ian) at University of Adelaide
 *	Remove dummy read of isr in SDP_CLR_INTR.
 *
 * 04-Oct-92  Ian Dall (ian) at University of Adelaide
 *	If BSY *and* SEL interrrupts, do disconnect and reconnect.
 *
 * 04-Oct-92  Ian Dall (ian) at University of Adelaide
 *	Make sure sdp_sel_enb has our ID in it when commands end.
 *
 * 04-Oct-92  Ian Dall (ian) at University of Adelaide
 *	Clear ARB interrupt in interrupt routine instead of complete
 *	selection. Look for reconnects before arb interrupts.
 *
 * 25-Sep-92  Ian Dall (ian) at University of Adelaide
 *	Wow, it works on all targets with and without disconnect even
 *	when thrashing all devices simultaneously! Clean up for general
 *	release.
 *
 * 25-Sep-92  Ian Dall (ian) at University of Adelaide
 *	Give up on trying to make phase change interrupts after EDMA
 *	reliable. Instead, poll for REQ.
 *
 * 23-Sep-92  Ian Dall (ian) at University of Adelaide
 *	Add delay to EDMA interrupt kludge.
 *
 * 22-Sep-92  Ian Dall (ian) at University of Adelaide
 *	If we got an EDMA interrupt and are expecting a Phase mismatch
 *	interrupt, then disable EDMA interrupts between servicing the
 *	EDMA and when we get the Phase mismatch interrupt.
 *
 * 21-Sep-92  Ian Dall (ian) at University of Adelaide
 *	Sometimes we lose phase mismatch interrupts at the end of a DMA.
 *	Try not resetting DMA until we get the phase mismatch interrupt.
 *
 * 21-Sep-92  Ian Dall (ian) at University of Adelaide
 *	Experiment with writing one byte "by hand" before the pdma_xfer
 *	to do all the rest.
 *
 * 20-Sep-92  Ian Dall (ian) at University of Adelaide
 *	Sigh, going to PDMA_VERSION 4.
 *
 * 17-Sep-92  Ian Dall (ian) at University of Adelaide
 *	Use PDMA_VERSION instead of NEW/OLD_PDMA since it looks like
 *	being an on going saga finding something which works!
 *
 * 15-Sep-92  Ian Dall (ian) at University of Adelaide
 *	Add code for a new PDMA which copies in chunks of block_size with
 *	interrupts disabled. Define OLD_PDMA in pdma.h to get the old
 *	code.
 *
 * 14-Sep-92  Ian Dall (ian) at University of Adelaide
 *	Make delay after reset depend on initiator id. This is the same
 *	as for the 53C94.
 *
 * 08-Sep-92  Ian Dall (ian) at University of Adelaide
 *	Monitor BSY. Necessary for disconnects to work properly.
 *
 * 07-Sep-92  Ian Dall (ian) at University of Adelaide
 *	Add identify message phase to the scripts. Make select with
 *	attention work.
 *
 * 07-Sep-92  Ian Dall (ian) at University of Adelaide
 *	We now assume level triggered interrupts.
 *
 * 06-Sep-92  Ian Dall (ian) at University of Adelaide
 *	Slight changes to do_sdp_intr() to allow level triggered
 *	interrupts instead of edge triggered interrupts. (See also
 *	pic.c).
 *
 * 02-Sep-92  Ian Dall (ian) at University of Adelaide
 *	Added disconnect code -- as yet untested.
 *
 * 02-Sep-92  Ian Dall (ian) at University of Adelaide
 *	Added code in sdp_intr() to deal with extra buffered byte in dma
 *	transfers. The only time it matters is if we do disconnects.
 *
 * 30-Aug-92  Ian Dall (ian) at University of Adelaide
 *	Multiple p-dma transfers per scsi cmd work now. Put
 *	PER_TGT_BURST_SIZE back to 8k.
 *
 * 30-Aug-92  Ian Dall (ian) at University of Adelaide
 *	Pseudo-dma seems to work! Compile with PER_TGT_BURST_SIZE of 1k
 *	to check that multiple dma transfers for one scsi command work.
 *
 * 29-Aug-92  Ian Dall (ian) at University of Adelaide
 *	Round up residual reported by pdma_xfer_abort to next block
 *	boundary. This is a kludge, but should work for most targets.
 *
 * 28-Aug-92  Ian Dall (ian) at University of Adelaide
 *	Changed to new pseudo-dma scheme with pdma_xfer() and
 *	pdma_xfer_abort() functions.
 *
 * 25-Aug-92  Ian Dall (ian) at University of Adelaide
 *	Pseudo dma works if reads are less than 3k! Try raising spl
 *	around bcopy_bytes to see if we are loosing a byte due to an
 *	interrupt.
 *
 * 25-Aug-92  Ian Dall (ian) at University of Adelaide
 *	Sometimes isr = 0 on interupt when it isn't really a reset (don't
 *	know why). Hopefully we can kludge around this by examining the
 *	bus for the reset bit.
 *
 * 19-Aug-92  Ian Dall (ian) at University of Adelaide
 *	Added pseudo-dma code for data in data out phases.
 *
 * 17-Aug-92  Ian Dall (ian) at University of Adelaide
 *	Added unit to sdp_buffer_sbrk() args.
 *
 * 17-Aug-92  Ian Dall (ian) at University of Adelaide
 *	Add unit no to sdp_buffer_init() args.
 *
 * 14-Aug-92  Ian Dall (ian) at University of Adelaide
 *	Fixed most "gcc -Wall" warnings.
 *
 * 12-Aug-92  Ian Dall (ian) at University of Adelaide
 *	Added function prototypes.
 *
 * 09-Aug-92  Ian Dall (ian) at University of Adelaide
 *	Moved definition of struct sdp_softc etc to include file.
 *
 * 09-Aug-92  Ian Dall (ian) at University of Adelaide
 *	Consolidated sdp_select target and sdp_start_select_target so the
 *	code is in only one place.
 *
 * 04-Aug-92  Ian Dall (ian) at University of Adelaide
 *	Clear interrupts immediately after phase mismatch interrupts are
 *	enabled in sdp_complete_selection.
 *
 * 03-Aug-92  Ian Dall (ian) at University of Adelaide
 *	There is no point in waiting for bus free in sdp_end_transaction()
 *	a) we might have missed it in a multi-initiator environment
 *	b) we shouldn't need to wait anyway.
 *	
 *	Don't mask interrupts in sdp_release_bus() since we do it in
 *	sdp_end_transaction().
 *
 * 03-Aug-92  Ian Dall (ian) at University of Adelaide
 *	Don't enable phase mis-match interrupts until selection
 *	successfully completed.
 *
 * 02-Aug-92  Ian Dall (ian) at University of Adelaide
 *	Remove spin wait for bus free in sdp_start_selection. Enhanced
 *	mode arbitration is supposed to do this for us.
 *
 * 01-Aug-92  Ian Dall (ian) at University of Adelaide
 *	Disable interrupts from everything except select when a command
 *	completes. We don't want to react to phase changes etc for other
 *	initiators.
 *
 * 01-Aug-92  Ian Dall (ian) at University of Adelaide
 *	Changed do_sdp_intr() to make full use of the enhanced mode ISR
 *	register.
 *
 * 31-Jul-92  Ian Dall (ian) at University of Adelaide
 *	Check more carefully for phase changes in data_in() and
 *	data_out(). Previously a phase change while waiting for REQ would
 *	result in an extra ACK being sent.
 *
 * 31-Jul-92  Ian Dall (ian) at University of Adelaide
 *	Check that a selection/reselection is for *this* initiator before
 *	proceeding.
 *	 
 * 11-May-92  Johannes Helander (jvh) at Helsinki University of Technology
 *	Created.
 *
 * $Log: scsi_dp8490_hdw.c,v $
 */
/*
 * This layer works based on small simple 'scripts' that are installed
 * at the start of the command and drive the chip to completion.
 * The idea comes from the specs of the NCR 53C700 'script' processor.
 *
 * There are various reasons for this, mainly
 * - Performance: identify the common (successful) path, and follow it;
 *   at interrupt time no code is needed to find the current status
 * - Code size: it should be easy to compact common operations
 * - Adaptability: the code skeleton should adapt to different chips without
 *   terrible complications.
 * - Error handling: and it is easy to modify the actions performed
 *   by the scripts to cope with strange but well identified sequences
 *
 */

/*
 * 	File: scsi_dp8490_hdw.c
 *
 * 	Author: Johannes Helander, Helsinki University of Technology 1992.
 *
 * 	From spec: 	National Semiconductor
 *			DP8490 Enhanced Asynchronous SCSI Interface (EASI)
 *			PRELIMINARY, January 1989
 *
 * 	Based on the NCR 5380 driver by Alessandro Forin and the
 *	Fujitsu MB89352 driver by Daniel Stodolsky.
 */

/* This driver still needs work.
 * Still to do:
 *		better msg handling.
 *		better error handling
 *		parity checking code
 *		target mode
 */

#define SDP_SOFT_DEGLITCH	/* Do de-glitching in software */
#define SDP_KLUDGE
#define SDP_DMA_IN
#define SDP_DMA_OUT

#include "sdp.h"
#include "scsi.h"
#include "mach_kdb.h"

#if	NSDP > 0
#include <platforms.h>

#include <mach/std_types.h>
#include <sys/types.h>
#include <chips/busses.h>
#include <scsi/compat_30.h>
#include <machine/machparam.h>

#include <sys/syslog.h>

#include <scsi/scsi.h>
#include <scsi/scsi2.h>
#include <scsi/scsi_defs.h>

#define DP(level, action) do { if ((level) <= scsi_debug) action; } while(0)

#include <ns532/pdma.h>
#include <scsi/adapters/scsi_dp8490.h>
#include <ns532/PC532/board.h>


#define delay_1p2_us()	delay(1)

/*
 * The 5380 can't tell you the scsi ID it uses, so
 * unless there is another way use the defaults
 */
#ifndef	my_scsi_id
#define	my_scsi_id(ctlr)	(scsi_initiator_id[(ctlr)])
#endif

/*
 * Statically partition the DMA buffer between targets.
 * This way we will eventually be able to attach/detach
 * drives on-fly.  And 18k/target is enough.
 */
#define PER_TGT_DMA_SIZE		((SDP_RAM_SIZE/8) & ~(sizeof(int)-1))

#ifdef PC532
/* Don't align; we like hard debugging and small buffers */
#define PER_TGT_BUFF_SIZE		(PER_TGT_DMA_SIZE)
/* This can probably be increased but check autoconf.c. Only
 * 8k is mapped. The hardware allows up to 4MB.
 */
#define PER_TGT_BURST_SIZE		(PAGE_SIZE * 2)
#else PC532
/*
 * Round to 4k to make debug easier
 */
#define	PER_TGT_BUFF_SIZE		((PER_TGT_DMA_SIZE >> 12) << 12)
#define PER_TGT_BURST_SIZE		(PER_TGT_BUFF_SIZE>>1)
#endif PC532

extern int sdp_unit;

/* address of the mapped pseudo DMA space */
extern volatile char *pc532_dma;
extern volatile char *pc532_dmaeop;


/*
 * Macros to make certain things a little more readable
 */
#define	SDP_ACK(ptr,phase)	(ptr)->sdp_tcmd = (phase)

#define	SDP_CLR_INTR(regs) \
    MACRO_BEGIN \
    regs->sdp_emr |= SDP_EMR_FN_RPI; \
    regs->sdp_emr |= SDP_EMR_FN_NOP; \
    MACRO_END

#define	SDP_CLR_PEND_INTR(regs) \
    MACRO_BEGIN \
    register int temp; \
    regs->sdp_emr |= SDP_EMR_FN_INT; \
    temp = regs->sdp_istatus; \
    regs->sdp_emr |= SDP_EMR_FN_RPI; \
    regs->sdp_emr |= SDP_EMR_FN_NOP; \
    MACRO_END

#define SDP_ISR_GET(regs, value) \
    MACRO_BEGIN \
    regs->sdp_emr |= SDP_EMR_FN_INT; \
    value = regs->sdp_istatus; \
    MACRO_END

#if NSDP == 1
unsigned char xxx_sdp_imr = 0;
#else
-- Do it some better way. --
#endif

#define SDP_IMR_PUT(regs, value) \
    MACRO_BEGIN \
    regs->sdp_emr |= SDP_EMR_FN_INT; \
    regs->sdp_imask = (value); \
    xxx_sdp_imr = (value); \
    MACRO_END

#define SDP_IMR_GET(regs, value) \
    MACRO_BEGIN \
    value = xxx_sdp_imr; \
    MACRO_END


#define	SCRIPT_MATCH(cs,bs)	(((bs)&SDP_BUS_BSY)|SDP_CUR_PHASE((bs)))

#define SCRIPT_ADVANCE_TO(scp, phase) (while(SCSI_PHASE((scp)->condition) != (phase)) (scp)++)

#define	SDP_PHASE_DISC	0x0	/* sort of .. */


/* forward decls of script actions */
boolean_t
  	sdp_identify(sdp_softc_t, unsigned, unsigned),
  	sdp_issue_command(sdp_softc_t, unsigned, unsigned),
	sdp_xfer_in(sdp_softc_t, unsigned, unsigned),
	sdp_xfer_out(sdp_softc_t, unsigned, unsigned),
        /* get status from target */
	sdp_get_status(sdp_softc_t, unsigned, unsigned),
        /* all come to an end */
	sdp_end_transaction(sdp_softc_t, unsigned, unsigned),
        /* get disconnect message(s) */
	sdp_msg_in(sdp_softc_t, unsigned, unsigned), 
        /* current target disconnected */
	sdp_disconnected(sdp_softc_t, unsigned, unsigned);
/* forward decls of error handlers */
boolean_t
        /* generic error handler */
	sdp_err_generic(sdp_softc_t, unsigned, unsigned),
	/* when a target disconnects */
	sdp_err_disconn(sdp_softc_t, unsigned, unsigned);

int sdp_reset_scsibus(sdp_softc_t);
void sdp_reset(sdp_softc_t, boolean_t);
void sdp_bus_reset(sdp_softc_t);
void sdp_attempt_selection(sdp_softc_t);

boolean_t sdp_probe_target(target_info_t *, io_req_t);

scsi_ret_t sdp_select_target(sdp_softc_t, sdp_padded_regmap_t *,
				  unsigned char, unsigned char);
scsi_ret_t sdp_complete_selection(sdp_softc_t, unsigned char);

/*
 * This should be somewhere else, and it was a
 * mistake to share this buffer across SCSIs.
 */
struct dmabuffer {
	volatile char	*base;
	char		*sbrk;
} dmab[ NSCSI ];

static volatile char *
sdp_buffer_base(int unit)
{
  if (unit >= NSCSI)
    return 0;
  else
    return dmab[unit].base;
}

void sdp_buffer_init(int unit, volatile char *ram)
{
  if (unit >= NSCSI)
    panic ("sdp: no such scsi unit configured");
  dmab[unit].base = dmab[unit].sbrk = (char *) ram;
  bzero((char *) ram, SDP_RAM_SIZE);
}

static char *
sdp_buffer_sbrk(int unit, int size)
{
	char	*ret = dmab[unit].sbrk;

	dmab[unit].sbrk += size;
	if ((dmab[unit].sbrk - dmab[unit].base) > SDP_RAM_SIZE)
		panic("sdpalloc");
	return ret;
}


sdp_softc_t	sdp_softc[NSCSI];

/*
 * Definition of the controller for the auto-configuration program.
 */

int	sdp_probe(char *, struct bus_ctlr *), scsi_slave(), scsi_attach(),
     sdp_go(target_info_t *, int, int, boolean_t);
void  sdp_intr(int, int, struct ns532_saved_state *), do_sdp_intr(int, int, struct ns532_saved_state *);
static void sdp_target_intr(sdp_softc_t, unsigned, unsigned);
static int  sdp_data_out(sdp_padded_regmap_t *, unsigned, unsigned, unsigned char *);
static int sdp_data_in(sdp_padded_regmap_t *, int, int, unsigned char *);
static boolean_t sdp_dma_out(sdp_softc_t, unsigned);
static boolean_t sdp_dma_in(sdp_softc_t, unsigned);
static boolean_t sdp_end(sdp_softc_t, unsigned, unsigned);
static boolean_t sdp_err_to_status(sdp_softc_t, unsigned, unsigned);
static boolean_t sdp_reconnect(sdp_softc_t, unsigned, unsigned);
static boolean_t sdp_release_bus(sdp_softc_t);


caddr_t	sdp_std[NSCSI] = { 0 };
struct	bus_device *sdp_dinfo[NSCSI*8];
struct	bus_ctlr *sdp_minfo[NSCSI];
struct	bus_driver sdp_driver = 
        { sdp_probe, scsi_slave, scsi_attach, sdp_go, sdp_std, "rz", sdp_dinfo,
	  "sdp", sdp_minfo, BUS_INTR_B4_PROBE};

/*
 * Scripts
 */
struct script
sdp_script_data_in[] = {
        { SCSI_PHASE_MSG_OUT|SDP_BUS_BSY, sdp_identify},
        { SCSI_PHASE_CMD|SDP_BUS_BSY, sdp_issue_command},
        { SCSI_PHASE_DATAI|SDP_BUS_BSY, sdp_xfer_in},
        { SCSI_PHASE_STATUS|SDP_BUS_BSY, sdp_get_status},
        { SCSI_PHASE_MSG_IN|SDP_BUS_BSY, sdp_end_transaction}
},
#define SCP_DATA_IN_XFER (2)
#define SCP_DATA_IN_STATUS (3)

sdp_script_data_out[] = {
        { SCSI_PHASE_MSG_OUT|SDP_BUS_BSY, sdp_identify},
        { SCSI_PHASE_CMD|SDP_BUS_BSY, sdp_issue_command},
        { SCSI_PHASE_DATAO|SDP_BUS_BSY, sdp_xfer_out},
        { SCSI_PHASE_STATUS|SDP_BUS_BSY, sdp_get_status},
        { SCSI_PHASE_MSG_IN|SDP_BUS_BSY, sdp_end_transaction}
},
#define SCP_DATA_OUT_XFER (2)
#define SCP_DATA_OUT_STATUS (3)

sdp_script_cmd[] = {
        { SCSI_PHASE_MSG_OUT|SDP_BUS_BSY, sdp_identify},
        { SCSI_PHASE_CMD|SDP_BUS_BSY, sdp_issue_command},
        { SCSI_PHASE_STATUS|SDP_BUS_BSY, sdp_get_status},
        { SCSI_PHASE_MSG_IN|SDP_BUS_BSY, sdp_end_transaction}
},

/* Disconnect sequence */

sdp_script_disconnect[] = {
	{ SDP_PHASE_DISC, sdp_disconnected}
};



#define	u_min(a,b)	(((a) < (b)) ? (a) : (b))

#if MACH_KDB

int sdp_state(vm_offset_t base)
{
	sdp_padded_regmap_t	*regs;
	extern char 	*sdp;
	int		old;
	unsigned char imr, isr;
	
	if (base == 0)
		base = (vm_offset_t)sdp;

	regs = (sdp_regmap_t *)base;
	old = PC532_SCSI_SELECT(ICU_DP);
	SDP_ISR_GET(regs, isr);
	SDP_IMR_GET(regs, imr);
	db_printf("scsi(dp): ph %x (sb %x), mode %x, tph %x, csr %x, cmd %x, emr %x, isr %x, imr %x",
		  (unsigned) SDP_CUR_PHASE(regs->sdp_bus_csr),
		  (unsigned) regs->sdp_bus_csr,
		  (unsigned) regs->sdp_mode,
		  (unsigned) regs->sdp_tcmd,
		  (unsigned) regs->sdp_csr,
		  (unsigned) regs->sdp_icmd,
                  (unsigned) regs->sdp_emr,
                  (unsigned) isr,
                  (unsigned) imr);
	PC532_SCSI_SELECT(old);
	return 0;
}

void sdp_sdp(int unit)
{
	sdp_softc_t sdp = sdp_softc[unit];
	if (!sdp)
	  return;
	db_printf("in_count = 0x%x\n", sdp->in_count);
	db_printf("out_count = 0x%x\n", sdp->out_count);
	db_printf("resid = 0x%x\n", sdp->resid);
	db_printf("state = 0x%x\n", sdp->state);
	db_printf("ntargets = 0x%x\n", sdp->ntargets);
	db_printf("done = 0x%x\n", sdp->done);
}

int sdp_target_state(target_info_t *tgt)
{
	if (tgt == 0)
		tgt = sdp_softc[0]->active_target;
	if (tgt == 0)
		return 0;
	db_printf("fl %x dma %x+%x cmd %x id %x per %x off %x ior %x ret %x\n",
		tgt->flags, tgt->dma_ptr, tgt->transient_state.dma_offset,
		tgt->cmd_ptr, tgt->target_id, tgt->sync_period, tgt->sync_offset,
		tgt->ior, tgt->done);
	if (tgt->flags & TGT_DISCONNECTED){
		script_t	spt;

		spt = tgt->transient_state.script;
		db_printf("disconnected at ");
		db_printsym(spt,1);
		db_printf(": %x ", spt->condition);
		db_printsym(spt->action,1);
		db_printf(", ");
		db_printsym(tgt->transient_state.handler, 1);
		db_printf("\n");
	}

	return 0;
}

void sdp_all_targets(int unit)
{
	int i;
	target_info_t	*tgt;
	for (i = 0; i < 8; i++) {
		tgt = sdp_softc[unit]->sc->target[i];
		if (tgt)
			sdp_target_state(tgt);
	}
}


int sdp_script_state(int unit)
{
	script_t	spt = sdp_softc[unit]->script;

	if (spt == 0) return 0;
	db_printsym(spt,1);
	db_printf(": %x ", spt->condition);
	db_printsym(spt->action,1);
	db_printf(", ");
	db_printsym(sdp_softc[unit]->error_handler, 1);
	return 0;

}

#define	PRINT(x)	if (scsi_debug) printf x

#define TRMAX 200
#define TRMARGIN 10		/* How many TRs before we have to TRCHECK */
int tr[TRMAX+TRMARGIN];
char *trname[TRMAX+TRMARGIN];
int trpt, trpthi, trcheck;
#define	TR(x)	do { trname[trpt] = "?"; tr[trpt++] = x; } while(0)
#define TRWRAP	if (trpt - trcheck > TRMARGIN) panic("sdp: insufficient trace checks."); trcheck = trpt; trpthi = trpt; trpt = 0; trcheck = 0;
#define TRCHECK	if (trpt - trcheck > TRMARGIN) panic("sdp: insufficient trace checks."); trcheck = trpt; if (trpt >= TRMAX) {TRWRAP}

#define	TR2(name, x)	do { trname[trpt] = name; tr[trpt++] = x; } while(0)

void sdp_print_trace(int skip)
{
	int i;
	
	db_printf ("trpthi = 0x%x, trpt = 0x%x\n", trpthi, trpt);
	for (i = trpt; i < trpthi; i++) {
		if (skip > 0)
		    skip--;
		else
		    db_printf ("%x\t%s\t%x\n", i, trname[i], tr[i]);
	}
	for (i = 0; i < trpt; i++) {
		if (skip > 0)
		    skip--;
		else
		    db_printf ("%x\t%s\t%x\n", i, trname[i], tr[i]);
	}
}

#define TRACE

#ifdef TRACE

#define LOGSIZE 256
int sdp_logpt;
char sdp_log[LOGSIZE];

/* #define MAXLOG_VALUE	0x24 */
#define MAXLOG_VALUE 0x7f

struct {
	char *name;
	unsigned int count;
} logtbl[MAXLOG_VALUE];

void static LOG(int e, char *f)
{
	sdp_log[sdp_logpt++] = (e);
	if (sdp_logpt == LOGSIZE) sdp_logpt = 0;
	if ((e) < MAXLOG_VALUE) {
		logtbl[(e)].name = (f);
		logtbl[(e)].count++;
	}
}

int sdp_print_log(int skip)
{
	register int i, j;
	register unsigned char c;

	for (i = 0, j = sdp_logpt; i < LOGSIZE; i++) {
		c = sdp_log[j];
		if (++j == LOGSIZE) j = 0;
		if (skip-- > 0)
			continue;
		if (c < MAXLOG_VALUE)
			db_printf(" %s", logtbl[c].name);
		else
			db_printf("-%d", c & 0x7f);
	}
	db_printf("\n");
	return 0;
}

void sdp_print_stat(void)
{
	register int i;
	register char *p;
	for (i = 0; i < MAXLOG_VALUE; i++) {
		if (p = logtbl[i].name)
			printf("%d %s\n", logtbl[i].count, p);
	}
}

#else	/* TRACE */
#define	LOG(e,f)
#endif	/* TRACE */

/* Call this from kdb */
sdp_padded_regmap_t	*last_sc = 0;

void sdp_dump(void)
{
	int old;

	old = PC532_SCSI_SELECT(ICU_DP);

	if (last_sc) {
		int foo;
		db_printf("\n");
		db_printf("CSD	0x%x\n", last_sc->sdp_data);
		db_printf("ICR	0x%x\n", last_sc->sdp_icmd);
		db_printf("MR2	0x%x\n", last_sc->sdp_mode);
		db_printf("TCR	0x%x\n", last_sc->sdp_tcmd);
		db_printf("CSB	0x%x\n", last_sc->sdp_bus_csr);
		db_printf("BSR	0x%x\n", last_sc->sdp_csr);
		db_printf("IDR	0x%x\n", last_sc->sdp_idata);
		db_printf("EMR	0x%x\n", last_sc->sdp_emr);
		SDP_ISR_GET (last_sc, foo);
		db_printf("ISR	0x%x\n", foo);
	}
	PC532_SCSI_SELECT(old);
}

#else	MACH_KDB

#define	PRINT(x)
#define	LOG(e,f)
#define TR(x)
#define TR2(x,y)
#define TRCHECK
#define TRWRAP

#endif	MACH_KDB


void sdp_break(void)
{
}
void sdp_break1(void)
{
}
void sdp_break2(void)
{
}
void sdp_break3(void)
{
}
void sdp_break4(void)
{
}
void sdp_break5(void)
{
}

/*
 *	Probe/Slave/Attach functions
 */

int sdp_skip_target = 0xe0;
int sdp_delay_after_intr = 0;	/* XXX */
int sdp_kludge_delay = 1000;

/*
 * Probe routine:
 *	Should find out (a) if the controller is
 *	present and (b) which/where slaves are present.
 *
 * Implementation:
 *	Send an identify msg to each possible target on the bus
 *	except of course ourselves.
 */
int sdp_probe(char *reg, struct bus_ctlr *ui)
{
	int             unit = ui->unit;
	sdp_softc_t     sdp = &sdp_softc_data[unit];
	int		target_id, i;
	scsi_softc_t	*sc;
	register sdp_padded_regmap_t	*regs;
	int		s, old;
	boolean_t	did_banner = FALSE;
	char		*cmd_ptr;

	/*
	 * We are only called if the chip is there,
	 * but make sure anyways..
	 */
	regs = (sdp_padded_regmap_t *) (reg);

#if MACH_KDB
	last_sc = regs;
#endif MACH_KDB

#if	notyet
	/* Mappable version side */
	SDP_probe(reg, ui);
#endif

	/*
	 * Initialize hw descriptor
	 */
	sdp_softc[unit] = sdp;
	sdp->regs = regs;

	sdp->buff = sdp_buffer_base(0);

	queue_init(&sdp->waiting_targets);

	sc = scsi_master_alloc(unit, sdp);
	sdp->sc = sc;

	sc->go = sdp_go;
	sc->probe = sdp_probe_target;
	sc->watchdog = scsi_watchdog;
	sdp->wd.reset = sdp_reset_scsibus;

#ifdef	MACH_KERNEL
	sc->max_dma_data = -1;				/* unlimited */
#else
	sc->max_dma_data = scsi_per_target_virtual;
#endif

	/*
	 * Reset chip
	 */
#if defined(PC532)
	s = splhi();
#else
	s = splbio();
#endif PC532
	old = PC532_SCSI_SELECT(ICU_DP);

	sdp_reset(sdp, TRUE);

	/*
	 * Our SCSI id on the bus.
	 */

	sc->initiator_id = my_scsi_id(unit);
	printf("%s%d: my SCSI id is %d", ui->name, unit, sc->initiator_id);

	/*
	 * For all possible targets, see if there is one and allocate
	 * a descriptor for it if it is there.
	 */
	cmd_ptr = sdp_buffer_sbrk(unit, 0);

	sdp->state |= SDP_STATE_PROBING;
	for (target_id = 0; target_id < 8; target_id++) {
		scsi_status_byte_t	status;
		scsi_ret_t ret;

		/* except of course ourselves */
		if (target_id == sc->initiator_id)
			continue;

		if (sdp_skip_target & (1 << target_id))
		{
			printf ("sdp: target id %d will not be probed\n",
				target_id);
			continue;
		}

		while ((ret = sdp_select_target (sdp, regs,
				       sc->initiator_id,
				       target_id)) == SCSI_RET_RETRY);
		if ( ret == SCSI_RET_DEVICE_DOWN) {
			continue;
		}

		printf(",%s%d", did_banner++ ? " " : " target(s) at ",
				target_id);

		/* should be command phase here: we selected wo ATN! */
		while (SDP_CUR_PHASE(regs->sdp_bus_csr) != SCSI_PHASE_CMD)
			;

		SDP_ACK(regs,SCSI_PHASE_CMD);

		/* build command in dma area */
		{
			unsigned char	*p = (unsigned char*) cmd_ptr;

			p[0] = SCSI_CMD_TEST_UNIT_READY;
			p[1] = 
			p[2] = 
			p[3] = 
			p[4] = 
			p[5] = 0;
		}

		sdp_data_out(regs, SCSI_PHASE_CMD, 6, cmd_ptr);

		while (SDP_CUR_PHASE(regs->sdp_bus_csr) != SCSI_PHASE_STATUS)
			;

		SDP_ACK(regs,SCSI_PHASE_STATUS);

		sdp_data_in(regs, SCSI_PHASE_STATUS, 1, &status.bits);

		if (status.st.scsi_status_code != SCSI_ST_GOOD)
			scsi_error( 0, SCSI_ERR_STATUS, status.bits, 0);

		/* get cmd_complete message */
		while (SDP_CUR_PHASE(regs->sdp_bus_csr) != SCSI_PHASE_MSG_IN)
			;

		SDP_ACK(regs,SCSI_PHASE_MSG_IN);

		sdp_data_in(regs, SCSI_PHASE_MSG_IN, 1, (unsigned char *) &i);

		/* check disconnected, clear all intr bits */
		while (regs->sdp_bus_csr & SDP_BUS_BSY)
			;
		SDP_ACK(regs, SDP_PHASE_DISC);

		/* ... */

		/*
		 * Found a target
		 */
		sdp->ntargets++;
		{
			register target_info_t	*tgt;

			tgt = scsi_slave_alloc(unit, target_id, sdp);

			/* "virtual" address for our use */
			tgt->cmd_ptr = sdp_buffer_sbrk(unit, PER_TGT_DMA_SIZE);

			/* pc532 does not have true dma */
			tgt->dma_ptr = (char*)0x0;
		}
	}
	printf(".\n");

	sdp->state &= ~SDP_STATE_PROBING;
	{
	  SDP_IMR_PUT(regs, ~SDP_INT_SEL);
	}
	SDP_CLR_PEND_INTR(regs);

	PC532_SCSI_SELECT(old);
	splx(s);

	return 1;
}

boolean_t
sdp_probe_target(target_info_t *tgt, io_req_t ior)
{
	boolean_t	newlywed;

	newlywed = (tgt->cmd_ptr == 0);
	if (newlywed) {
		/* desc was allocated afresh */

		/* "virtual" address for our use */
		tgt->cmd_ptr = sdp_buffer_sbrk(sdp_unit, PER_TGT_DMA_SIZE);

		/* pc532 does not have dma */
		tgt->dma_ptr = (char*)0x0;
	}

	if (scsi_inquiry(tgt, SCSI_INQ_STD_DATA) == SCSI_RET_DEVICE_DOWN)
		return FALSE;

	tgt->flags = TGT_ALIVE;
	return TRUE;
}


/* XXX Nobody calls this? /jtv */
#ifndef PC532
static sdp_wait(preg, until)
	volatile unsigned char	*preg;
{
	int timeo = 1000000;
	/* read it over to avoid bus glitches */
	while ( ((*preg & until) != until) ||
		((*preg & until) != until) ||
		((*preg & until) != until)) {
		delay(1);
		if (!timeo--) {
			printf("sdp_wait TIMEO with x%x\n", *preg);
			break;
		}
	}
	return *preg;
}
#endif PC532


static int sdp_data_out(sdp_padded_regmap_t *regs, unsigned phase, unsigned count, unsigned char *data)
{
	register unsigned char	icmd;
	register int bs;

	/* ..checks.. */

	icmd = (regs->sdp_icmd & ~SDP_ICMD_DIFF) | SDP_ICMD_ENHANCED;
loop:
        while ((((bs = regs->sdp_bus_csr) & (SDP_BUS_REQ | SDP_PHASE_MASK))
		== SDP_CSB_PHASE(phase))
#ifdef SDP_SOFT_DEGLITCH
	       || (((bs = regs->sdp_bus_csr) & (SDP_BUS_REQ | SDP_PHASE_MASK))
		   == SDP_CSB_PHASE(phase))
	       || (((bs = regs->sdp_bus_csr) & (SDP_BUS_REQ | SDP_PHASE_MASK))
		   == SDP_CSB_PHASE(phase))
#endif
	       );
	if ((bs & SDP_PHASE_MASK) != SDP_CSB_PHASE(phase)){
	  return count;
	}
	      
	icmd |= SDP_ICMD_DATA;
	assert(icmd & SDP_ICMD_ENHANCED);
	regs->sdp_icmd = icmd;
	regs->sdp_odata = *data++;
	icmd |= SDP_ICMD_ACK;
	assert(icmd & SDP_ICMD_ENHANCED);
	regs->sdp_icmd = icmd;

	icmd &= ~(SDP_ICMD_DATA|SDP_ICMD_ACK);
        while (((regs->sdp_bus_csr & (SDP_BUS_REQ | SDP_PHASE_MASK))
		== (SDP_BUS_REQ | SDP_CSB_PHASE(phase)))
#ifdef SDP_SOFT_DEGLITCH
	       || ((regs->sdp_bus_csr & (SDP_BUS_REQ | SDP_PHASE_MASK))
		   == (SDP_BUS_REQ | SDP_CSB_PHASE(phase)))
	       || ((regs->sdp_bus_csr & (SDP_BUS_REQ | SDP_PHASE_MASK))
		   == (SDP_BUS_REQ | SDP_CSB_PHASE(phase)))
#endif
	       );
	assert(icmd & SDP_ICMD_ENHANCED);
	regs->sdp_icmd = icmd;
	if (--count > 0)
		goto loop;
	return 0;
}

static int sdp_data_in(sdp_padded_regmap_t *regs, int phase, int count, unsigned char *data)
{
	register unsigned char	icmd;
	register int bs;
	int s;

	/* ..checks.. */

	icmd = (regs->sdp_icmd & ~SDP_ICMD_DIFF) | SDP_ICMD_ENHANCED;
	s = splhi();		/* protect against 32532 prefetch */
loop:
        while ((((bs = regs->sdp_bus_csr) & (SDP_BUS_REQ | SDP_PHASE_MASK))
		== SDP_CSB_PHASE(phase))
#ifdef SDP_SOFT_DEGLITCH
	       || (((bs = regs->sdp_bus_csr) & (SDP_BUS_REQ | SDP_PHASE_MASK))
		   == SDP_CSB_PHASE(phase))
	       || (((bs = regs->sdp_bus_csr) & (SDP_BUS_REQ | SDP_PHASE_MASK))
		   == SDP_CSB_PHASE(phase))
#endif
	       ) ;
	if ((bs & SDP_PHASE_MASK) != SDP_CSB_PHASE(phase)){
	  splx(s);
	  return count;
	}

	*data++ = regs->sdp_data;
	icmd |= SDP_ICMD_ACK;
	assert(icmd & SDP_ICMD_ENHANCED);
	regs->sdp_icmd = icmd;

	icmd &= ~SDP_ICMD_ACK;
        while (((regs->sdp_bus_csr & (SDP_BUS_REQ | SDP_PHASE_MASK))
		== (SDP_BUS_REQ | SDP_CSB_PHASE(phase)))
#ifdef SDP_SOFT_DEGLITCH
	       || ((regs->sdp_bus_csr & (SDP_BUS_REQ | SDP_PHASE_MASK))
		   == (SDP_BUS_REQ | SDP_CSB_PHASE(phase)))
	       || ((regs->sdp_bus_csr & (SDP_BUS_REQ | SDP_PHASE_MASK))
		   == (SDP_BUS_REQ | SDP_CSB_PHASE(phase)))
#endif
	       ) ;
	
	assert(icmd & SDP_ICMD_ENHANCED);
	regs->sdp_icmd = icmd;
	if (--count > 0)
		goto loop;
	splx(s);
	return 0;

}

void sdp_reset(sdp_softc_t sdp, boolean_t quickly)
{
	register sdp_padded_regmap_t	*regs = sdp->regs;
	int ss;

	ss = splhi();		/* I guess we are there already. Make sure */

	DP(1, printf("sdp: resetting chip\n"));

	regs->sdp_icmd = 0;
	regs->sdp_mode = 0;
	regs->sdp_sel_enb = 0;
	regs->sdp_odata = 0;
	regs->sdp_tcmd = 0;

	regs->sdp_icmd = SDP_ICMD_ENHANCED;
	SDP_IMR_PUT(regs, 0);
	SDP_CLR_PEND_INTR(regs);
	SDP_IMR_PUT(regs, 0xff); /* mask all interrupts first */

	regs->sdp_mode = 0;
	regs->sdp_sel_enb = 0;
	regs->sdp_icmd = SDP_ICMD_ENHANCED;
	regs->sdp_tcmd = 0;

	/* set interrupt mask */
	/* Enable the selection interrupts. Other interrupts enabled
         * at the start of each command and disabled at the end.
         */
	SDP_IMR_PUT(regs, ~SDP_INT_SEL );

	/* make sure we do not miss transition */
	regs->sdp_tcmd = SDP_PHASE_DISC;

	/* don't listen to target intrs */
	regs->sdp_sel_enb = 0;

	/* get phase mismatch signals. */
	regs->sdp_emr = SDP_EMR_PHASE_CHECK;

	/* clear interrupt */
	SDP_CLR_PEND_INTR(regs);

	splx(ss);

        if (quickly)
                return;

        /*
         * reset the scsi bus, the interrupt routine does the rest
         * eventually calling sdp_bus_reset().
         */
	
	DP(1,printf ("sdp:resetting bus\n"));
        regs->sdp_icmd = SDP_ICMD_RST | SDP_ICMD_ENHANCED;
}

/*
 *	Operational functions
 */

/*
 * Start a SCSI command on a target
 */
int sdp_go(target_info_t *tgt, int cmd_count, int in_count, boolean_t cmd_only)
{
	sdp_softc_t		sdp;
	register int		s;
	boolean_t		disconn;
	script_t		scp;
	boolean_t		(*handler)();
	int			late;

	LOG(1,"\ngo");

	sdp = (sdp_softc_t)tgt->hw_state;

	/*
	 * We cannot do real DMA.
	 */

	if ((tgt->cur_cmd == SCSI_CMD_WRITE) ||
	    (tgt->cur_cmd == SCSI_CMD_LONG_WRITE)){
		io_req_t	ior = tgt->ior;
		register int	len = ior->io_count;

		tgt->transient_state.out_count = len;
#ifdef  PC532
		/* No need to copy stuff for DMA */
		tgt->transient_state.copy_count = 0;
#else PC532
		if (len > PER_TGT_BUFF_SIZE)
			len = PER_TGT_BUFF_SIZE;
		bcopy(	ior->io_data,
			tgt->cmd_ptr + cmd_count,
			len);
		tgt->transient_state.copy_count = len;
#endif PC532
		/* avoid leaks */
		if (len < tgt->block_size) {
#ifndef PC532
			bzero(	tgt->cmd_ptr + cmd_count + len,
				tgt->block_size - len);
#endif PC532
			tgt->transient_state.out_count = tgt->block_size;
		}
	} else {
		tgt->transient_state.out_count = 0;
		tgt->transient_state.copy_count = 0;
	}

	tgt->transient_state.cmd_count = cmd_count;

	disconn  = BGET(scsi_might_disconnect,sdp->sc->masterno,tgt->target_id);
	disconn  = disconn && (sdp->ntargets > 1);
	disconn |= BGET(scsi_should_disconnect,sdp->sc->masterno,tgt->target_id);


	/*
	 * Setup target state
	 */
	tgt->done = SCSI_RET_IN_PROGRESS;

	handler = (disconn) ? sdp_err_disconn : sdp_err_generic;

        /* determine wether or not to use the late forms of the scripts */
        late = cmd_only ? FALSE : (tgt->flags & TGT_DID_SYNCH);

	switch (tgt->cur_cmd) {
	    case SCSI_CMD_READ:
	    case SCSI_CMD_LONG_READ:
		LOG(0x13,"readop");
		scp = sdp_script_data_in;
		break;
	    case SCSI_CMD_WRITE:
	    case SCSI_CMD_LONG_WRITE:
		LOG(0x14,"writeop");
		scp = sdp_script_data_out;
		break;
	    case SCSI_CMD_INQUIRY:
		/* This is likely the first thing out:
		   do the synch neg if so 
		   NO SYNC on dp8490 */
	    case SCSI_CMD_REQUEST_SENSE:
	    case SCSI_CMD_MODE_SENSE:
	    case SCSI_CMD_RECEIVE_DIAG_RESULTS:
	    case SCSI_CMD_READ_CAPACITY:
	    case SCSI_CMD_READ_BLOCK_LIMITS:
		scp = sdp_script_data_in;
		LOG(0x1c,"cmdop");
		LOG(0x80+tgt->cur_cmd,0);
		break;
	    case SCSI_CMD_MODE_SELECT:
	    case SCSI_CMD_REASSIGN_BLOCKS:
	    case SCSI_CMD_FORMAT_UNIT:
		tgt->transient_state.cmd_count = sizeof(scsi_command_group_0);
		tgt->transient_state.out_count = cmd_count - sizeof(scsi_command_group_0);
		scp = sdp_script_data_out;
		LOG(0x1c,"cmdop");
		LOG(0x80+tgt->cur_cmd,0);
		break;
	    case SCSI_CMD_TEST_UNIT_READY:
		/*
		 * Do the synch negotiation here, unless prohibited
		 * or done already
		 * NO SYNC supported on dp8490.
		 */
		scp = sdp_script_cmd;
		LOG(0x1c,"cmdop");
		LOG(0x80+tgt->cur_cmd,0);
		break;
	    default:
		LOG(0x1c,"cmdop");
		LOG(0x80+tgt->cur_cmd,0);
		scp = sdp_script_cmd;
	}

	tgt->transient_state.script = scp;
	tgt->transient_state.handler = handler;
	tgt->transient_state.identify = (cmd_only) ? 0xff :
		(disconn ? SCSI_IDENTIFY|SCSI_IFY_ENABLE_DISCONNECT :
			   SCSI_IDENTIFY);

	if (in_count)
		tgt->transient_state.in_count =
			(in_count < tgt->block_size) ? tgt->block_size : in_count;
	else
		tgt->transient_state.in_count = 0;
	tgt->transient_state.dma_offset = 0;

	/*
	 * See if another target is currently selected on
	 * this SCSI bus, e.g. lock the sdp structure.
	 * Note that it is the strategy routine's job
	 * to serialize ops on the same target as appropriate.
	 */
#if 0
locking code here
#endif
	s = splbio();

	if (sdp->wd.nactive++ == 0)
		sdp->wd.watchdog_state = SCSI_WD_ACTIVE;

	if (sdp->state & SDP_STATE_BUSY) {
		/*
		 * Queue up this target, note that this takes care
		 * of proper FIFO scheduling of the scsi-bus.
		 */
		LOG(3,"enqueue");
		enqueue_tail(&sdp->waiting_targets, (queue_entry_t) tgt);
	} else {
		/*
		 * It is down to at most two contenders now,
		 * we will treat reconnections same as selections
		 * and let the scsi-bus arbitration process decide.
		 */
		sdp->state |= SDP_STATE_BUSY;
		sdp->next_target = tgt;
		sdp_attempt_selection(sdp);
		/*
		 * Note that we might still lose arbitration..
		 */
	}
	splx(s);
	return 0;
}

#if 1				/* DEBUG */
int sdp_old_ipl = 0;
#include <ns532/ipl.h>		/* XXX */
#endif

void sdp_attempt_selection(sdp_softc_t sdp)
{
	target_info_t	*tgt;
	sdp_padded_regmap_t	*regs;
	int		old;

	regs = sdp->regs;

	tgt = sdp->next_target;

	LOG(4,"select");
	LOG(0x80+tgt->target_id,0);

	/*
	 * Init bus state variables and set registers.
	 */
	sdp->active_target = tgt;

#if 1				/* DEBUG */
	{
		extern int curr_ipl;

		sdp_old_ipl = curr_ipl;
		if (curr_ipl < SPLSCSI)
		    panic ("sdp_attempt_selection: bad ipl");
	}
#endif	
	old = PC532_SCSI_SELECT(ICU_DP);

	/* At this point, the ncr5380 code attempted to check if a
         * reselection pending ? That isn't reliable anyway and
         * for the dp8490, it turns out we don't need to.
         */

	sdp->script = tgt->transient_state.script;
	sdp->error_handler = tgt->transient_state.handler;
	sdp->done = SCSI_RET_IN_PROGRESS;

	sdp->in_count = 0;
	sdp->out_count = 0;

	sdp->done = sdp_select_target(sdp, regs, sdp->sc->initiator_id,
				      tgt->target_id);

	PC532_SCSI_SELECT(old);

	/* the interrupt routine takes care of the rest */
	return;
}

scsi_ret_t
sdp_select_target(sdp_softc_t sdp, sdp_padded_regmap_t *regs,
		  unsigned char myid, unsigned char id)
{	
	unsigned char imr, isr;

	LOG(0x62, "start_arb");

	/* for our purposes.. */
	myid = 1 << myid;

	regs->sdp_tcmd = 0;	/* in case it wasn't */
	regs->sdp_icmd = SDP_ICMD_ENHANCED;

	SDP_IMR_GET(regs, imr);
	imr &= ~SDP_INT_ARB; /* mask */
	SDP_IMR_PUT(regs, imr);

	regs->sdp_odata = myid;

	/* arbitrate in enhanced mode see figure on page 34, A1 */

	if (regs->sdp_emr & SDP_EMR_ARB)
	  {
	    /* Shouldn't happen ? */
	    printf("Clearing SDP_EMR_ARB\n");
	    regs->sdp_emr &= ~SDP_EMR_ARB;
	  }
	regs->sdp_emr |= SDP_EMR_ARB;
	if (!(sdp->state & SDP_STATE_PROBING))
	  return SCSI_RET_IN_PROGRESS;

	do {
	  delay(1); /* save electricity and the chip's mind */
	  SDP_ISR_GET(regs, isr);
	} while (!(isr & SDP_INT_ARB));

	SDP_CLR_INTR(regs);

	return sdp_complete_selection(sdp, id);
}

static scsi_ret_t
sdp_retry_arbitration(sdp_softc_t sdp, sdp_padded_regmap_t *regs,
		      unsigned char myid, unsigned char id)
{
	LOG(0x61, "retry_arb");
	/* reset ARB since arbitration failed. see 4.4.2 */
	regs->sdp_mode &= ~SDP_MODE_ARB; /* shouldn't be on */
	regs->sdp_emr = regs->sdp_emr & ~SDP_EMR_ARB;
	return (sdp->state & SDP_STATE_PROBING) ? SCSI_RET_RETRY: sdp_select_target(sdp, regs, myid, id);
}

scsi_ret_t
sdp_complete_selection(sdp_softc_t sdp, unsigned char target_id)
{
	sdp_padded_regmap_t	*regs;
	unsigned char		myid, id;
	boolean_t		with_atn;
	register unsigned char	csd, icmd, imr;

	LOG(0x60, "sel_comp");

	regs = sdp->regs;

	myid = 1 << sdp->sc->initiator_id;
	id = 1 << target_id;

	if (regs->sdp_icmd & SDP_ICMD_LST) /* what about AIP? */
	    return sdp_retry_arbitration(sdp, regs, sdp->sc->initiator_id,
					 target_id);

	/* did we win? */
	csd = regs->sdp_data;
	if (!(myid & csd) || (myid < (csd & ~myid))) {
		return sdp_retry_arbitration(sdp, regs, sdp->sc->initiator_id,
					     target_id);
	}

	with_atn = !(sdp->state & SDP_STATE_PROBING) &&
	  (sdp->active_target->transient_state.identify != 0xff);

	regs->sdp_sel_enb = 0;
	regs->sdp_tcmd = 0;

	icmd = SDP_ICMD_ENHANCED;

	TR2("---------------- sel", 0);
	TR2("id", myid | id);
	TR2("BSR", regs->sdp_csr);
	TR2("CSB", regs->sdp_bus_csr);
	TR2("MR2", regs->sdp_mode);
	TR2("ICR", regs->sdp_icmd);
	TRCHECK;

	icmd |= SDP_ICMD_SEL;
	assert(icmd & SDP_ICMD_ENHANCED);
	regs->sdp_icmd = icmd;

	delay(2);		/* 1.2 us */

	regs->sdp_odata = myid | id;
	icmd |= SDP_ICMD_DATA | (with_atn ? SDP_ICMD_ATN : 0);
	assert(icmd & SDP_ICMD_ENHANCED);
	regs->sdp_icmd = icmd;
	
	{int i = 0; i++; i++;}	/* 2 deskew delay each 45 ns */
	
	regs->sdp_emr &= ~SDP_EMR_ARB;

	/* bus settle delay, 400ns */
	delay(0 + 1); /* too much ? */

	{
		/* 250 msecs in 100 usecs chunks */
		register int timeo  = 50000; /* 2500 XXX */
		while ((regs->sdp_bus_csr & SDP_BUS_BSY) == 0)
			if (--timeo > 0)
				delay(100);
			else {
				goto nodev;
			}
	}

	{int i = 0; i++; i++;}	/* 2 deskew delays */

	/* Enable phase change interrupts for the data transfer phases */
	if( !(sdp->state & SDP_STATE_PROBING) )
	  {
	    SDP_IMR_GET(regs, imr);
	    imr &= ~(SDP_INT_PHASE|SDP_INT_BSY); /* mask */
	    SDP_IMR_PUT(regs, imr);
	    /* Clear interrupts. This shouldn't be necessary but enabling
	     * phase change interrupts seems to cause an interrupt from some
	     * past phase change event. The chip specs don't say this happens,
	     * but then, they don't say it *doesn't* happen either!
	     */
	    SDP_CLR_PEND_INTR(regs);
	  }

	icmd = (icmd & ~(SDP_ICMD_DATA | SDP_ICMD_SEL));
	assert(icmd & SDP_ICMD_ENHANCED);
	regs->sdp_icmd = icmd;
	if (!with_atn)
	  (sdp->script)++;	/* Advance past MSG_OUT phase */
	DP(1, sdp_break3());
	return SCSI_RET_SUCCESS;

nodev:
	DP(2, printf("sdp: selection failed (2)\n"));
	DP(1, sdp_break2());
	regs->sdp_sel_enb = 0;
	regs->sdp_icmd = SDP_ICMD_ENHANCED;

	return SCSI_RET_DEVICE_DOWN;
}

/*
 * Interrupt routine
 *	Take interrupts from the chip
 *
 * Implementation:
 *	Move along the current command's script if
 *	all is well, invoke error handler if not.
 */
void sdp_intr(int unit, int old_ipl, struct ns532_saved_state *int_regs)
{
	int	old;

	/*
	 * Take care of the pc532 brain damage that the two SCSI
	 * controllers are in the same physical addresses.
	 * And selected by the ICU select signal.
	 */
	old = PC532_SCSI_SELECT(ICU_DP);

	do_sdp_intr(unit, old_ipl, int_regs);

	PC532_SCSI_SELECT(old);
}

void do_sdp_intr(int unit, int old_ipl, struct ns532_saved_state *int_regs)
{
	sdp_softc_t	sdp;
	script_t	scp;
	unsigned	csr, bs, cmd, isr, imr, mode, tcmd, emr;
	sdp_padded_regmap_t	*regs;
	boolean_t		try_match;

#if	notyet
	extern boolean_t	rz_use_mapped_interface;

	if (rz_use_mapped_interface)
		return SDP_intr(unit);
#endif

	sdp = sdp_softc[unit];
	regs = sdp->regs;


	LOG(5,"\n\tintr");

	{	/* XXX for testing purposes XXX */
		int i;
		for (i = 0; i < sdp_delay_after_intr; i++)
		    delay(1);	/* XXX */
	}
	/* ack interrupt */
	csr = regs->sdp_csr;
	bs = regs->sdp_bus_csr;
	cmd = regs->sdp_icmd;
	mode =  regs->sdp_mode;
	tcmd = regs->sdp_tcmd;
	emr = regs->sdp_emr;
	SDP_ISR_GET(regs, isr);
	SDP_IMR_GET(regs, imr);

	TR2("---------intr--------", 0);
	TR2("imr  7 IMR", imr);
	TR2("isr  7 ISR", isr);
	TR2("mode 2 MR2", mode);

	TR2("csr  5 BSR", csr);
	TR2("bs   4 CSB", bs);
	TR2("tcmd 3 TCR", tcmd);
	TR2("cmd  1 ICR", cmd);
	TRCHECK;


#if defined(PC532) && (PDMA_VERSION == 1 || PDMA_VERSION == 2)
	/* This is code is for pseudo dma only.
	 * All we do is ensure that the block copy function
	 * exits and we remember how much was copied.
	 * Then we set SDP_STATE_DMA_INTR, mask off interrupts
	 * and return. Later, when the xfer_in/out terminates, we check
         * for the presence of SDP_STATE_DMA_INTR and this function gets
         * called again with SDP_STATE_DMA_IN/OUT clear. On the second
	 * time through we really take the correct action. This might
	 * appear tortuous, but otherwise it is at least possible for the
	 * stack depth to get out of hand.
	 */
	if (isr && (sdp->state &SDP_STATE_PDMA))
	  {
	    int count;
	    LOG(0x41, "xfer_abort");
	    TR2("-------DMA intr------", 0);
	    TR2("sdp state", sdp->state);

	    sdp->state |= SDP_STATE_DMA_INTR;
	    if (sdp->state & SDP_STATE_DMA_IN)
	      count = sdp->in_count;
	    else
	      count = sdp->out_count;
	    if (count == 0)
	      panic("sdp: unexpected dma count of zero");
	    sdp->resid = pdma_xfer_abort(count, &(sdp->pdma), int_regs);
	    SDP_IMR_PUT(regs, 0xff);
	    SDP_ISR_GET(regs, isr);

	    TR2("isr  7 ISR", isr);
	    TR2("csr  5 BSR", regs->sdp_csr);

	    /* With a IMR of 0xff, this should just clear the BSR int bit. */
	    SDP_CLR_INTR(regs);
	    TR2("csr  5 BSR", regs->sdp_csr);
	    TRCHECK;
	    return;
	  }
#endif
	if ( (isr & (SDP_INT_EDMA | SDP_INT_DMA_PHASE))
	    && !(sdp->state & SDP_STATE_DMA_DONE))
	  {
	    /* It is possible for the DMA_PHASE and EDMA interrupts to
             * arrive at slightly different times.
	     * Make sure this code only gets done once for each
             * dma xfer.
	     */
	    sdp->state |= SDP_STATE_DMA_DONE;
	    if (isr & SDP_INT_EDMA)
	      assert(regs->sdp_tcmd & SDP_TCMD_LAST_SENT);
	    
	    if(!(isr & SDP_INT_EDMA) &&
	       !(csr & SDP_CSR_DREQ) &&
	       !(sdp->state & SDP_STATE_DMA_IN))
	      {
	        /* We are not at the end of the transfer, so if the
		 * last write was successful, DREQ would be asserted
		 * again. DREQ not asserted here, means the last write
		 * wasn't successful.
		 */
		DP(1, printf("sdp: increment resid based on DRQ\n"));
		sdp->resid++;
		LOG(0x35, "resid++");
	      }

	    /* Round residual to block size if necessary */
	    TR2("resid", sdp->resid);
	    TRCHECK;
	    if (sdp->resid & (sdp->active_target->block_size - 1))
	      {
		printf("sdp: Residual (0x%x) not block size multiple.\n", sdp->resid);
		/* The residual reported by pdma_xfer_abort can be
		 * underestimated for unavoidable hardware reasons.
		 * Round up to block_size as a work around.
		 * Maybe should just round up to an even byte boundary.
		 */
		sdp->resid = (sdp->resid & ~(sdp->active_target->block_size - 1))
		  + sdp->active_target->block_size;
		printf("sdp: adjusted residual: 0x%x.\n", sdp->resid);
		LOG(0x36, "resid^block_size");
		gimmeabreak();
	      }

	    if((csr & SDP_CSR_DREQ) &&
	       (sdp->state & SDP_STATE_DMA_IN))
	      if( ! (isr & SDP_INT_EDMA))
		{
		  /* We are not at the end of the transfer.
		   * DREQ implies that there is a byte not yet read
		   * from the chip (though it is has been read from
		   * the target).
		   */
		  target_info_t *tgt = sdp->active_target;
		  int offset;
		  char *dma_ptr = tgt->ior->io_data;
		  offset = tgt->transient_state.dma_offset +
		    sdp->in_count - sdp->resid - 1;
		  dma_ptr += offset;
		  DP(1, printf("sdp: fetch extra byte based on DRQ. buf[0x%x] 0x%d => 0x%x\n", offset, *dma_ptr, regs->sdp_data));
		  *dma_ptr = regs->sdp_data;
		  LOG(0x37, "extra-byte");
		}
	      else
		{
		  panic("sdp: DRQ not xero");
		}
	    /* Stop DMA */
	    regs->sdp_mode = mode & ~(SDP_MODE_DMA|SDP_MODE_DMA_IE);
	  }

	SDP_CLR_INTR(regs);

	/* check who got the bus */
	if ((isr & SDP_INT_SEL) && !(isr & SDP_INT_BSY) && (bs & SDP_BUS_IO)) {
	  if (sdp_reconnect(sdp, csr, bs) || (isr == SDP_INT_SEL))
	    {
	      return;
	    }
	  }

	if (isr & SDP_INT_ARB) {
		LOG(7, "iarb");
		/* XXX check return value! (better check) */
		if (sdp_complete_selection(sdp, sdp->active_target->target_id) == SCSI_RET_DEVICE_DOWN) {
			sdp->done = SCSI_RET_DEVICE_DOWN;
			sdp_end(sdp, csr, bs);
		}
		/* interrupt cleared in called routine */
		return;
	}


	if (cmd & SDP_ICMD_RST){
	  assert(isr == 0);
		sdp_bus_reset(sdp);
		return;
	}

	/* we got an interrupt allright */
	if (sdp->active_target)
		sdp->wd.watchdog_state = SCSI_WD_ACTIVE;

	/* drop spurious calls */
	if ((csr & SDP_CSR_INT) == 0) {
	  DP(1,sdp_break1());
	  LOG(2, "SPURIOUS");
	  return;
	}

	if (isr == 0){
	  if (bs & SDP_BUS_RST)
	    sdp_bus_reset(sdp);
	  else
	    {
	      DP(1,sdp_break1());
	      LOG(2, "SPURIOUS");
	    }
	  return;
	}

        /* Any interrupt if we are a target means call sdp_target_intr.
	 * A selection interrupt means call sdp_target_intr also
	 * (and become a target?).
	 * Note: reselect has I/O asserted, select has not */
	if ((sdp->state & SDP_STATE_TARGET) ||
	    ((isr & SDP_INT_SEL) && !(bs & SDP_BUS_IO))) {
		sdp_target_intr(sdp,csr,bs);
		return;
	}

	scp = sdp->script;

	/* BSY lost implies disconnect. What if we don't expect it? */
	if (isr & SDP_INT_BSY)
	   if (scp && (scp->condition == SDP_PHASE_DISC)) {
		(void) (*scp->action)(sdp, csr, bs);
		if ((isr & SDP_INT_SEL) && (bs & SDP_BUS_IO)) {
		  if (sdp_reconnect(sdp, csr, bs))
		    {
		      return;
		    }
		}
		return;
	      }
	   else
	     {
	       printf ("sdp: Unexpexpected disconnect\n");
	       gimmeabreak();
	       return;
	     }
	  


	if (! (isr & (SDP_INT_PHASE | SDP_INT_DMA_PHASE | SDP_INT_EDMA))) {
	  /* Can do parity checks etc here */
		printf ("sdp: unexpected intr");
		gimmeabreak();
		return;
	}


	if (! scp) {
	  /* This could be interrupts due to bus activity which
	   * doesn't concern us. We attempt to avoid these with
           * the right IMR value. See sdp_end_transaction.
	   */
		printf ("sdp: unexpected intr, null scp.\n");
		gimmeabreak();
		return;
	}

	if (SCRIPT_MATCH(csr,bs) != scp->condition) {

	  /* We should get a phase mismatch interrupt eventually
	   */
	  if(isr == SDP_INT_EDMA)
	    {
	      LOG(0x33, "EDMA-noaction");
	      return;
	    }
#ifdef SDP_KLUDGE
	  if(isr & SDP_INT_EDMA)
	    {
	      int i;
	      /* Sometimes the REQ line hasn't settled.
               */
	      for (i = 0; ((bs & SDP_BUS_REQ) == 0) && (i < sdp_kludge_delay); i++)
		{
		  delay(1);
		  csr = regs->sdp_csr;
		  bs = regs->sdp_bus_csr;
		}

	      /* Maybe we should take special action if i == sdp_kludge_delay
               * Currently we will handle it the same as any other non
               * responding target. */
	      TR2("---------intr--------", 4);
	      TR2("csr  5 BSR", csr);
	      TR2("bs   4 CSB", bs);
	      TR2("EDMA Phase wait", i);
	      TRCHECK;
	    }
#endif
	  if (try_match = (*sdp->error_handler)(sdp, csr, bs)) {
	    csr = regs->sdp_csr;
	    bs = regs->sdp_bus_csr;
	  }
	} else
	  try_match = TRUE;

	/* might have been side effected */
	scp = sdp->script;

	if (try_match && (SCRIPT_MATCH(csr,bs) == scp->condition)) {
		/*
		 * Perform the appropriate operation,
		 * then proceed
		 */
		if ((*scp->action)(sdp, csr, bs)) {
			/* might have been side effected */
			scp = sdp->script;
			sdp->script = scp + 1;
		}
	}
	return;
}



void sdp_target_intr(sdp_softc_t sdp, unsigned csr, unsigned bs)
{
	panic("SDP: TARGET MODE !!!\n");
}

/*
 * All the many little things that the interrupt
 * routine might switch to
 */
boolean_t
sdp_end_transaction(sdp_softc_t sdp, unsigned csr, unsigned bs)
{
	register sdp_padded_regmap_t	*regs = sdp->regs;
	char			cmc;
	unsigned char imr;

	LOG(0x1f,"end_t");

	regs->sdp_icmd = SDP_ICMD_ENHANCED;
	regs->sdp_mode &= ~SDP_MODE_MONBSY;

        /* Turn off interrupts due to other activity on the bus.
	 * We only want select (as a target) interrupts. */
	SDP_IMR_GET(regs, imr);
	imr = ~SDP_INT_SEL;
	SDP_IMR_PUT(regs, imr);
	regs->sdp_sel_enb = (1 << sdp->sc->initiator_id);

	SDP_ACK(regs,SCSI_PHASE_MSG_IN);

	sdp_data_in(regs, SCSI_PHASE_MSG_IN, 1, &cmc);

	if (cmc != SCSI_COMMAND_COMPLETE)
	  {
	    TR2("bad msg in", cmc);
	    printf("sdp:{T%x}\n", cmc);
	  }

TR2("sdp_bus_csr", regs->sdp_bus_csr);

	SDP_ACK(regs,SDP_PHASE_DISC);

	sdp_end(sdp, csr, bs);
	return FALSE;
}

static boolean_t
sdp_end(sdp_softc_t sdp, unsigned csr, unsigned bs)
{
	register target_info_t	*tgt;
	register sdp_padded_regmap_t	*regs = sdp->regs;
	boolean_t		reconn_pending;

	LOG(6,"end");

	tgt = sdp->active_target;

	if ((tgt->done = sdp->done) == SCSI_RET_IN_PROGRESS)
		tgt->done = SCSI_RET_SUCCESS;

	sdp->script = 0;

	if (sdp->wd.nactive-- == 1)
		sdp->wd.watchdog_state = SCSI_WD_INACTIVE;

	sdp_release_bus(sdp);

	if (tgt->ior) {
	  io_req_t ior = tgt->ior;
		LOG(0xA,"ops->restart");
		(*tgt->dev_ops->restart)(tgt, TRUE);
	}

	return FALSE;
}

static boolean_t
sdp_release_bus(sdp_softc_t sdp)
{
	boolean_t	ret = FALSE;

	LOG(9,"release");

	sdp->script = 0;

	if (sdp->state & SDP_STATE_COLLISION) {

		LOG(0xB,"collided");
		sdp->state &= ~SDP_STATE_COLLISION;
		sdp_attempt_selection(sdp);

	} else if (queue_empty(&sdp->waiting_targets)) {

		sdp->state &= ~SDP_STATE_BUSY;
		sdp->active_target = 0;
		ret = TRUE;
	} else {
		LOG(0xC,"dequeue");
		sdp->next_target = (target_info_t *)
				dequeue_head(&sdp->waiting_targets);
		sdp_attempt_selection(sdp);
	}
	return ret;
}

boolean_t
sdp_get_status(sdp_softc_t sdp, unsigned csr, unsigned bs)
{
	register sdp_padded_regmap_t	*regs = sdp->regs;
	scsi2_status_byte_t	status;
	register target_info_t	*tgt;

	LOG(0xD,"get_status");
	TRCHECK;

	regs->sdp_icmd = SDP_ICMD_ENHANCED;

	sdp->state &= ~SDP_STATE_PDMA;

	tgt = sdp->active_target;

	SDP_ACK(regs,SCSI_PHASE_STATUS);

	if (sdp_data_in(regs, SCSI_PHASE_STATUS, 1, &status.bits))
	  {
	    TR2("---------get_status--------", 0);
	    TR2("mode 2 MR2", regs->sdp_mode);
	    TR2("csr  5 BSR", regs->sdp_csr);
	    TR2("bs   4 CSB", regs->sdp_bus_csr);
	    TR2("tcmd 3 TCR", regs->sdp_tcmd);
	    TRCHECK;
	    panic("sdp_get_status: couldn't get status");
	  }

	TR2("---------get_status--------", 1);
	TR2("mode 2 MR2", regs->sdp_mode);
	TR2("csr  5 BSR", regs->sdp_csr);
	TR2("bs   4 CSB", regs->sdp_bus_csr);
	TR2("tcmd 3 TCR", regs->sdp_tcmd);
	TRCHECK;

	if (status.st.scsi_status_code != SCSI_ST_GOOD) {
		DP(1, printf("sdp_get_status: not good\n"));
		scsi_error(sdp->active_target, SCSI_ERR_STATUS, status.bits, 0);
		sdp->done = (status.st.scsi_status_code == SCSI_ST_BUSY) ?
			SCSI_RET_RETRY : SCSI_RET_NEED_SENSE;
	} else
		sdp->done = SCSI_RET_SUCCESS;

	return TRUE;
}

boolean_t
sdp_identify(sdp_softc_t sdp, unsigned cst, unsigned bs)
{
        register sdp_padded_regmap_t   *regs = sdp->regs;
	unsigned char icmd;
	target_info_t	*tgt = sdp->active_target;

        LOG(0x11, "identify");
        /* we have just done a select;
           Bus is in MSG_OUT phase;
           need to phase match */
	icmd = (regs->sdp_icmd & ~(SDP_ICMD_DIFF|SDP_ICMD_ATN)) | SDP_ICMD_ENHANCED;
	regs->sdp_icmd = icmd;
	SDP_ACK(regs,SCSI_PHASE_MSG_OUT);
	return (sdp_data_out(regs, SCSI_PHASE_MSG_OUT,
			   1, &tgt->transient_state.identify) == 0);
}

boolean_t
sdp_issue_command(sdp_softc_t sdp, unsigned cst, unsigned bs)
{
        register sdp_padded_regmap_t   *regs = sdp->regs;

        LOG(0x12, "cmd_issue");
        /* we have just done a select;
           Bus is in CMD phase;
           need to phase match */
        SDP_ACK(regs, SCSI_PHASE_CMD);
	regs->sdp_mode = SDP_MODE_MONBSY;

        return sdp_data_out(regs, SCSI_PHASE_CMD,
                     sdp->active_target->transient_state.cmd_count,
                     sdp->active_target->cmd_ptr) ? FALSE : TRUE;
}


boolean_t
sdp_xfer_in(sdp_softc_t sdp, unsigned cst, unsigned bs)
{
        register target_info_t  *tgt;
	register sdp_padded_regmap_t	*regs = sdp->regs;
        register int            count;
        boolean_t               advance_script = TRUE;

        LOG(0xE,"xfer_in");

        tgt = sdp->active_target;

        count = tgt->transient_state.in_count;

        SDP_ACK(regs, SCSI_PHASE_DATAI);

        if ((tgt->cur_cmd != SCSI_CMD_READ) &&
            (tgt->cur_cmd != SCSI_CMD_LONG_READ))
          advance_script = sdp_data_in(regs, SCSI_PHASE_DATAI, count, tgt->cmd_ptr) == 0;
        else
          {
#ifdef SDP_DMA_IN
	      LOG(0x2E, "cmd");
	      LOG(0x80 + tgt->cur_cmd,0);
	      advance_script = sdp_dma_in(sdp, SCSI_PHASE_DATAI);
#else
            advance_script = sdp_data_in(regs, SCSI_PHASE_DATAI, count, tgt->ior->io_data) == 0;
#endif
          }

        return advance_script;
}

boolean_t
sdp_xfer_out(sdp_softc_t sdp, unsigned  cst, unsigned bs)
{
        register sdp_padded_regmap_t   *regs = sdp->regs;
        register target_info_t  *tgt;
        boolean_t               advance_script = TRUE;
        int                     count = sdp->out_count;

        LOG(0xF,"xfer_out");

        tgt = sdp->active_target;

        count = tgt->transient_state.out_count;

        SDP_ACK(regs, SCSI_PHASE_DATAO);

        if ((tgt->cur_cmd != SCSI_CMD_WRITE) &&
            (tgt->cur_cmd != SCSI_CMD_LONG_WRITE))
          advance_script = sdp_data_out(regs, SCSI_PHASE_DATAO, count,
                       tgt->cmd_ptr + tgt->transient_state.cmd_count) == 0;
        else
	  {
#ifdef SDP_DMA_OUT
	    LOG(0x2F, "cmd");
	    LOG(0x80+tgt->cur_cmd,0);
	    advance_script = sdp_dma_out(sdp, SCSI_PHASE_DATAO);
#else
	    advance_script = sdp_data_out(regs, SCSI_PHASE_DATAO, count, tgt->ior->io_data) == 0;
#endif
	  }
        return advance_script;
}


/* send data to target. Called in three different ways:
   (a) to start transfer (b) to restart a bigger-than-PER_TGT_BURST_SIZE
   transfer (c) after reconnection
 */


static boolean_t
sdp_dma_out (sdp_softc_t sdp, unsigned phase)
{
  sdp_padded_regmap_t	*regs = sdp->regs;

  char		*dma_ptr;
  target_info_t	*tgt;
  boolean_t      advance_script = TRUE;
  int            count = sdp->out_count;

  LOG(0x30,"dma_out");

  tgt = sdp->active_target;
  sdp->state &= ~SDP_STATE_DMA_IN;

  if (sdp->out_count == 0)
    {
      /*
       * Nothing committed: either just sent the
       * command or reconnected
       */

      regs->sdp_icmd =  SDP_ICMD_DATA | SDP_ICMD_ENHANCED;

      count = tgt->transient_state.out_count;
      count = u_min (count, PER_TGT_BURST_SIZE);

      /* PC532 transfers directly from IO_DATA buffer */
#ifndef PC532
      remains = PER_TGT_BUFF_SIZE - tgt->transient_state.dma_offset;
      count = u_min (count, remains);
#endif
      
      TR2("dma out(initial) count",count);

      /* common case of 8k-or-less write ? */
      advance_script = (tgt->transient_state.out_count == count);
    }
  else
    {
      /*
       * We already sent some data.
       *
       * Fix the DMA pointer to the first byte that was not sent.
       *
       * Also, take care of bogus interrupts
       */
      int offset, xferred;

#ifdef PC532
      xferred = sdp->out_count - sdp->resid;
#else
#error Get bytes not transferred from dma hardware somehow
      xferred = sdp->out_count - xferred;
#endif
      assert (xferred > 0);

      tgt->transient_state.out_count -= xferred;
      assert (tgt->transient_state.out_count > 0);

      offset = tgt->transient_state.dma_offset;
      tgt->transient_state.dma_offset += xferred;
      count = u_min (tgt->transient_state.out_count, PER_TGT_BURST_SIZE);

#ifndef PC532
      /* This is only for machines where dma transfers have to be
       * copy_in/out to a local buffer.
       */
      if (tgt->transient_state.dma_offset == PER_TGT_BUFF_SIZE)
	{
	  tgt->transient_state.dma_offset = 0;
	}
      else
	{
	  register int remains;
	  remains = PER_TGT_BUFF_SIZE - tgt->transient_state.dma_offset;
	  count = u_min (count, remains);
	}
#endif

      TR2("dma out(cont) count",count);

      /* last chunk ? */
      if (tgt->transient_state.out_count == count)
	goto quickie;

#ifdef PC532      
      /* No need to copy data for scsi pseudo dma */
      advance_script = FALSE;
#else
      /* ship some more */
      dma_ptr = tgt->ior->io_data + tgt->transient_state.dma_offset;

      sdp->out_count = count;

#error      Do Hardware Dma Here

      /* copy some more data */
      sdp_copyout(tgt, offset, xferred);
      return FALSE;
#endif
    }

 quickie:
  {
    unsigned char imr;
    TRCHECK;
    sdp->out_count = count;
    dma_ptr = tgt->ior->io_data + tgt->transient_state.dma_offset;

    SDP_ACK(regs, phase);

    SDP_IMR_GET(regs, imr);
    imr &= ~(SDP_INT_DMA_PHASE | SDP_INT_EDMA);
    SDP_IMR_PUT(regs, imr);
    regs->sdp_mode |= (SDP_MODE_DMA|SDP_MODE_DMA_IE);

    delay(1);

    DP(1, sdp_break3());
    regs->sdp_dma_send = 1;
    sdp->state &= ~SDP_STATE_DMA_DONE;

#ifdef PC532
    {
#if PDMA_VERSION == 1 || PDMA_VERSION == 2      
      sdp->state |= SDP_STATE_PDMA;
      sdp->state &= ~SDP_STATE_DMA_INTR;
#endif
      /* Ensure that on the last byte, eop gets set */
#if PDMA_VERSION == 4
      /* On writes, we seem to get the interrupt after writing the first
       * byte of the next block. So we fool it by copying one byte by
       * hand first. This is purely for efficiency. The code in the interrupt
       * routine seems smart enough to correct the resid regardless.
       * If a mis-aligned buffer is too slow, wo could write 4 bytes
       * here instead of one.
       */
      *(pc532_dmaeop - count + 1) = *dma_ptr;
      dma_ptr++;
      count--;
      sdp->resid = pdma_xfer (dma_ptr, pc532_dmaeop - count + 1, count, tgt->block_size,
		 &(regs->sdp_csr), SDP_CSR_INT, &(regs->sdp_csr) , SDP_CSR_DREQ, 1000000, 0);
#elif PDMA_VERSION == 3
      sdp->resid = pdma_xfer (dma_ptr, pc532_dmaeop - count + 1, count, tgt->block_size,
		 &(regs->sdp_csr), SDP_CSR_INT,  100);
#elif PDMA_VERSION == 2
      pdma_xfer (dma_ptr, pc532_dmaeop - count + 1, count, tgt->block_size,
		 &(sdp->state), SDP_STATE_DMA_INTR,  &(sdp->pdma));
#elif PDMA_VERSION == 1
      pdma_xfer (dma_ptr, pc532_dmaeop - count + 1, count,
		 &(sdp->state), SDP_STATE_DMA_INTR,  &(sdp->pdma));
#endif

#if PDMA_VERSION == 1 || PDMA_VERSION == 2      
      sdp->state &= ~SDP_STATE_PDMA;
      SDP_IMR_PUT(regs, imr);
#endif
    }
#endif
  }

  return advance_script;
}

static boolean_t
sdp_dma_in (sdp_softc_t sdp, unsigned phase)
{
  register target_info_t	*tgt;
  sdp_padded_regmap_t	*regs = sdp->regs;
  char			*dma_ptr;
  register int		count;
  boolean_t		advance_script = TRUE;
  unsigned char imr;

  LOG(0x31,"dma_in");

  tgt = sdp->active_target;
  sdp->state |= SDP_STATE_DMA_IN;

  if (sdp->in_count == 0)
    {
      /*
       * Got nothing yet: either just sent the command
       * or just reconnected
       */
      regs->sdp_icmd = SDP_ICMD_ENHANCED;
      
      count = tgt->transient_state.in_count;
      count = u_min (count, PER_TGT_BURST_SIZE);
      
#ifndef PC532
      avail = PER_TGT_BUFF_SIZE - tgt->transient_state.dma_offset;
      count = u_min (count, avail);
#endif
      TR2("INA (drv)", count);
      TR2("INA offset", tgt->transient_state.dma_offset);
      
      if (scsi_debug)
	{
	  io_req_t ior = tgt->ior;
	  TR2("INA: alloc", ior->io_alloc_size);
	  TR2("INA: requ", ior->io_count);
	  TR2("INA: buff *", (int)ior->io_data);
	}

      /* common case of 8k-or-less read ? */
      advance_script = (tgt->transient_state.in_count == count);
    }
  else
    {
      /*
       * We received some data.
       */
      register int  offset, xferred;
      
#ifdef PC532
      assert(sdp->resid == 0);
      xferred = sdp->in_count - sdp->resid;
#else
#error Get bytes not transferred from dma hardware somehow
      xferred = sdp->in_count - xferred;
#endif
      
      if (scsi_debug)
	{
	  printf("{INB %x %x}", sdp->in_count, xferred);
	}

      assert(xferred > 0);

      tgt->transient_state.in_count -= xferred;
      assert(tgt->transient_state.in_count > 0);

      offset = tgt->transient_state.dma_offset;
      tgt->transient_state.dma_offset += xferred;
      count = u_min(tgt->transient_state.in_count, (PER_TGT_BURST_SIZE));

#ifndef PC532
      /* This is only for machines where dma transfers have to be
       * copy_in/out to a local buffer.
       */
      if (tgt->transient_state.dma_offset == PER_TGT_BUFF_SIZE)
	{
	  printf("sdp: at end of dma_buffer\n");
	  tgt->transient_state.dma_offset = 0;
	}
      else
	{
	  register int avail;
	  avail = PER_TGT_BUFF_SIZE - tgt->transient_state.dma_offset;
	  count = u_min(count, avail);
	}
#endif
      advance_script = (tgt->transient_state.in_count == count);

#ifdef PC532
      
      TR2("dma< in(cont) count",count);
#else
      /* get some more */

#error Do hardware dma here

      /* copy what we got */
      sdp_copyin( tgt, offset, xferred);

#endif
    }

  TRCHECK;
  sdp->in_count = count;
  dma_ptr = tgt->ior->io_data + tgt->transient_state.dma_offset;
  
  SDP_ACK(regs, phase);
    
  SDP_IMR_GET(regs, imr);
  imr &= ~(SDP_INT_DMA_PHASE | SDP_INT_EDMA);
  SDP_IMR_PUT(regs, imr);
  regs->sdp_mode |= (SDP_MODE_DMA|SDP_MODE_DMA_IE);

  delay(1);

  DP(1, sdp_break3());
  regs->sdp_emr |= SDP_EMR_FN_SDI;
  sdp->state &= ~SDP_STATE_DMA_DONE;
#ifdef PC532
    {
#if PDMA_VERSION == 1 || PDMA_VERSION == 2      
      sdp->state |= SDP_STATE_PDMA;
      sdp->state &= ~SDP_STATE_DMA_INTR;
#endif
      /* Ensure that on the last byte, eop gets set */
#if PDMA_VERSION == 4
      sdp->resid = pdma_xfer (pc532_dmaeop - count + 1, dma_ptr, count, tgt->block_size,
		 &(regs->sdp_csr), SDP_CSR_INT, &(regs->sdp_csr), SDP_CSR_DREQ, 1000000, 0);
#elif PDMA_VERSION == 3
      sdp->resid = pdma_xfer (pc532_dmaeop - count + 1, dma_ptr, count, tgt->block_size,
		 &(regs->sdp_csr), SDP_CSR_INT, 100);
#elif PDMA_VERSION == 2
      pdma_xfer (pc532_dmaeop - count + 1, dma_ptr, count, tgt->block_size,
		 &(sdp->state), SDP_STATE_DMA_INTR,  &(sdp->pdma));
#elif PDMA_VERSION == 1
      pdma_xfer (pc532_dmaeop - count + 1, dma_ptr, count,
		 &(sdp->state), SDP_STATE_DMA_INTR,  &(sdp->pdma));
#endif
#if PDMA_VERSION == 1 || PDMA_VERSION == 2      
      SDP_IMR_PUT(regs, imr);
      sdp->state &= ~SDP_STATE_PDMA;
#endif
    }
#endif
  
  return advance_script;
}

/* disconnect-reconnect ops */

/* get the message in via polled io ?? */
boolean_t
sdp_msg_in(sdp_softc_t sdp, unsigned csr, unsigned bs)
{
        char msgs[6];
        register target_info_t  *tgt;
	sdp_padded_regmap_t		*regs = sdp->regs;
	int offset, len;
	boolean_t		ok = FALSE;

        LOG(0x15,"msg_in");

        tgt = sdp->active_target;

	len =  sizeof(msgs) - sdp_data_in(regs, SCSI_PHASE_MSG_IN, sizeof(msgs), msgs);
	/* check the message is indeed a DISCONNECT */
	if ((len == 0) || (len > 2))
		ok = FALSE;
	else {
		/* A SDP message preceeds it in non-completed READs */
		ok = ((msgs[0] == SCSI_DISCONNECT) ||	/* completed op */
		      ((msgs[0] == SCSI_SAVE_DATA_POINTER) && /* incomplete */
		       (msgs[1] == SCSI_DISCONNECT)));
	}
	if (!ok)
	  {
	    int i;
	    printf("sdp: tgt %d bad msg (%d):",
		   sdp->active_target->target_id, len);
	    if (len > 0)
	      printf(" [ 0x%x", msgs[0]);
	    for (i = 1; i < len; i++)
	      printf(", 0x%x", msgs[i]);
	    printf("]\n");
	  }

	return ok;
}


/* save all relevant data, free the BUS */
boolean_t
sdp_disconnected(sdp_softc_t sdp, unsigned csr, unsigned bs)
{
	register target_info_t	*tgt;
	int isr;
	sdp_padded_regmap_t		*regs = sdp->regs;

	regs->sdp_mode &= ~SDP_MODE_MONBSY;
        /* Turn off interrupts due to other activity on the bus.
	 * We only want select (reconnect or as a target) interrupts. */

	SDP_ISR_GET(regs, isr);
	SDP_IMR_PUT(regs, ~SDP_INT_SEL);
	/* Cancel any outstanding interrupts (BSY loss) unless there
         * is a select */
	if (isr && !(isr & SDP_INT_SEL))
	  SDP_CLR_INTR(regs);

	SDP_ACK(regs,SDP_PHASE_DISC);

	LOG(0x16,"disconnected");

	tgt = sdp->active_target;
	tgt->flags |= TGT_DISCONNECTED;
	tgt->transient_state.handler = sdp->error_handler;
	/* the rest has been saved in sdp_err_disconn() */

	LOG(0x80 + tgt->target_id, 0);
	PRINT(("{D%d}", tgt->target_id));

	sdp_release_bus(sdp);

	return FALSE;
}

/* get reconnect message, restore BUS */
static boolean_t
sdp_reconnect(sdp_softc_t sdp, unsigned csr, unsigned bs)
{
	sdp_padded_regmap_t		*regs;
	target_info_t	*tgt;
	register int		id;
	int msg;

	regs = sdp->regs;

	regs->sdp_mode &= ~SDP_MODE_PAR_CHK;
	id = regs->sdp_data;/*parity!*/
	regs->sdp_mode |= SDP_MODE_PAR_CHK;
		
	/* xxx check our id is in there and there is one and only one target id */
	
	if ((id & (1 << sdp->sc->initiator_id)) == 0)
	  return FALSE;
	else
	  {
	    register int i;
	    id &= ~(1 << sdp->sc->initiator_id);
	    {
	      register int tid;
	      for (i = 0, tid = 1; i < 8; i++, tid <<= 1)
		if (id == tid) break;
	      if (i == 8)
		{
		  return FALSE;
		}
	    }
	    id = i;
	  }

	LOG(0x17,"reconnect");
	LOG(0x80+id,0);
	/*
	 * See if this reconnection collided with a selection attempt
	 */
	if (sdp->state & SDP_STATE_BUSY)
	  {
	    sdp->state |= SDP_STATE_COLLISION;
	    regs->sdp_emr &= ~SDP_EMR_ARB;
	  }
	sdp->state |= SDP_STATE_BUSY;

	TR2("---------reconn--------", 0);
	TR2("csr  5 BSR", regs->sdp_csr);
	TR2("bs   4 CSB", bs);
	TR2("cmd  1 ICR", regs->sdp_icmd);
	TRCHECK;
	DP(2, printf("sdp_reconnect. id = 0x%x\n", id));
	regs->sdp_icmd = SDP_ICMD_BSY|SDP_ICMD_ENHANCED;
	while (regs->sdp_bus_csr & SDP_BUS_SEL)
		;
	regs->sdp_icmd = SDP_ICMD_ENHANCED;
	TR2("---------reconn--------", 1);
	TR2("csr  5 BSR", regs->sdp_csr);
	TR2("bs   4 CSB", regs->sdp_bus_csr);
	TR2("cmd  1 ICR", regs->sdp_icmd);
	TRCHECK;
	delay_1p2_us();
	while ( ((regs->sdp_bus_csr & SDP_BUS_BSY) == 0) 
#ifdef SDP_SOFT_DEGLITCH
		|| ((regs->sdp_bus_csr & SDP_BUS_BSY) == 0)
		|| ((regs->sdp_bus_csr & SDP_BUS_BSY) == 0)
#endif
	       )
		;

	regs->sdp_mode |= SDP_MODE_MONBSY;
	TR2("---------reconn--------", 2);
	TR2("csr  5 BSR", regs->sdp_csr);
	TR2("bs   4 CSB", regs->sdp_bus_csr);
	TR2("mode 2 MR2", regs->sdp_mode);
	TR2("cmd  1 ICR", regs->sdp_icmd);
	TRCHECK;

	/* Now should wait for correct phase: REQ signals it */
	while (	((regs->sdp_bus_csr & SDP_BUS_REQ) == 0)
#ifdef SDP_SOFT_DEGLITCH
		|| ((regs->sdp_bus_csr & SDP_BUS_REQ) == 0)
		|| ((regs->sdp_bus_csr & SDP_BUS_REQ) == 0)
#endif
	       )
		;

	TR2("---------reconn--------", 3);
	TR2("csr  5 BSR", regs->sdp_csr);
	TR2("bs   4 CSB", regs->sdp_bus_csr);
	TR2("mode 2 MR2", regs->sdp_mode);
	TR2("cmd  1 ICR", regs->sdp_icmd);
	TRCHECK;

	/* Get identify msg */
	bs = regs->sdp_bus_csr;

	if (SDP_CUR_PHASE(bs) != SCSI_PHASE_MSG_IN)
	  {
	    printf("sdp: reconnect -- not MSG IN\n");
	    gimmeabreak();
	  }

	SDP_ACK(regs,SCSI_PHASE_MSG_IN);
	msg = 0;
	SDP_IMR_PUT(regs, ~(SDP_INT_PHASE|SDP_INT_BSY|SDP_INT_SEL));
	/* Clear interrupts. This shouldn't be necessary but enabling
	 * phase change interrupts seems to cause an interrupt from some
	 * past phase change event. The chip specs don't say this happens,
	 * but then, they don't say it *doesn't* happen either!
	 */
	SDP_CLR_PEND_INTR(regs);

	/* Should get a phase mismatch when tgt changes phase */
	sdp_data_in(regs, SCSI_PHASE_MSG_IN, 1, (char *) &msg);
	regs->sdp_mode = SDP_MODE_MONBSY;
	regs->sdp_sel_enb = 0;

	if (msg != SCSI_IDENTIFY)
		printf("{I%x %x}", id, msg);

	tgt = sdp->sc->target[id];
	if (id > 7 || tgt == 0) panic("sdp_reconnect");

	DP(1,printf("{R%d}", id));
	if (sdp->state & SDP_STATE_COLLISION)
		DP(1, printf("[B %d-%d]", sdp->active_target->target_id, id));


	sdp->active_target = tgt;
	tgt->flags &= ~TGT_DISCONNECTED;

	sdp->script = tgt->transient_state.script;
	sdp->error_handler = tgt->transient_state.handler;
	sdp->in_count = 0;
	sdp->out_count = 0;

	
	return TRUE;
}

/*
 * The bus was reset
 */
void sdp_bus_reset(sdp_softc_t sdp)
{
	LOG(0x21,"bus_reset");

	/*
	 * Clear bus descriptor
	 */
	sdp->script = 0;
	sdp->error_handler = 0;
	sdp->active_target = 0;
	sdp->next_target = 0;
	sdp->state = 0;
	queue_init(&sdp->waiting_targets);
	sdp->wd.nactive = 0;
	sdp_reset(sdp, TRUE);

	printf("sdp%d: (%d) bus reset ", sdp->sc->masterno, ++sdp->wd.reset_count);
	delay(scsi_delay_after_reset + sdp->sc->initiator_id * 500000); /* some targets take long to reset */

	if (sdp->sc == 0)	/* sanity */
		return;

	scsi_bus_was_reset(sdp->sc);
}

/*
 * Error handlers
 */

/*
 * Generic, default handler
 */
boolean_t
sdp_err_generic(sdp_softc_t sdp, unsigned csr, unsigned bs)
{
	LOG(0x10,"err_generic");

	if (SDP_CUR_PHASE(bs) == SCSI_PHASE_STATUS)
		return sdp_err_to_status(sdp, csr, bs);
#if 1
	if (SDP_CUR_PHASE(bs) == SCSI_PHASE_MSG_IN) {
		DP(1, printf ("sdp: premature MSG IN phase\n"));
		LOG(0x64, "pre_MSG_IN");
		sdp_end_transaction (sdp, csr, bs);
		return FALSE;
	}
#endif 1
	TR2("--------Phase Error-------",0);
	TR2("bs   4 CSB", bs);
	TRCHECK;
	return FALSE;
}

/*
 * Handle generic errors that are reported as
 * an unexpected change to STATUS phase
 */
static boolean_t sdp_err_to_status(sdp_softc_t sdp, unsigned csr, unsigned bs)
{
	script_t		scp = sdp->script;

	LOG(0x20,"err_tostatus");
	while (SCSI_PHASE(scp->condition) != SCSI_PHASE_STATUS)
		scp++;
	sdp->script = scp;
	return TRUE;
}

/*
 * Watch for a disconnection
 */
boolean_t
sdp_err_disconn(sdp_softc_t sdp, unsigned csr,  unsigned bs)
{
	register sdp_padded_regmap_t	*regs;
	register target_info_t	*tgt;

	LOG(0x18,"err_disconn");

	if (SDP_CUR_PHASE(bs) != SCSI_PHASE_MSG_IN)
		return sdp_err_generic(sdp, csr, bs);

	regs = sdp->regs;

	tgt = sdp->active_target;

	switch (SCSI_PHASE(sdp->script->condition)) {
	case SCSI_PHASE_DATAO:
		LOG(0x1b,"+DATAO");

	regs->sdp_icmd = SDP_ICMD_ENHANCED;

		if (sdp->out_count) {
		  register int xferred, offset;

			
#ifdef PC532
		  xferred = sdp->out_count - sdp->resid;
#else
#error Get bytes not transferred from dma hardware somehow
		  xferred = sdp->out_count - xferred;
#endif
		  tgt->transient_state.out_count -= xferred;
		  offset = tgt->transient_state.dma_offset;
		  tgt->transient_state.dma_offset += xferred;
		  DP(1,  printf("{O %x %x}", xferred, sdp->out_count));
#ifndef PC532
		  if (tgt->transient_state.dma_offset >= PER_TGT_BUFF_SIZE)
				tgt->transient_state.dma_offset = 0;

			sdp_copyout( tgt, offset, xferred);
#endif

		}
		tgt->transient_state.script = sdp_script_data_out + SCP_DATA_OUT_XFER;
		break;

	case SCSI_PHASE_DATAI:
		LOG(0x19,"+DATAI");

		regs->sdp_icmd = SDP_ICMD_ENHANCED;

		if (sdp->in_count) {
			register int offset, xferred;
#ifdef PC532
			xferred = sdp->in_count - sdp->resid;
#else
#error Get bytes not transferred from dma hardware somehow
			xferred = sdp->in_count - xferred;
#endif

			assert(xferred > 0);
			tgt->transient_state.in_count -= xferred;
			assert(tgt->transient_state.in_count > 0);
			offset = tgt->transient_state.dma_offset;
			tgt->transient_state.dma_offset += xferred;
#ifndef PC532
			if (tgt->transient_state.dma_offset >= PER_TGT_BUFF_SIZE)
				tgt->transient_state.dma_offset = 0;

			/* copy what we got */
			sdp_copyin( tgt, offset, xferred, 0, 0/*extrab*/);
#endif
		}
		tgt->transient_state.script = sdp_script_data_in + SCP_DATA_IN_XFER;
		break;

	case SCSI_PHASE_STATUS:
		/* If we got a disconnect during the last (or only)
                 * dma chunk for this command, the script will already
		 * be advanced to status phase, even though the data
		 * phase isn't finished.
		 * Will have to restart dma */

		if (sdp->state & SDP_STATE_DMA_IN) {
			register int offset, xferred;

			LOG(0x1a,"+STATUS+R");

			regs->sdp_icmd = SDP_ICMD_ENHANCED;

#ifdef PC532
			xferred = sdp->in_count - sdp->resid;
#else
#error Get bytes not transferred from dma hardware somehow
			xferred = sdp->in_count - xferred;
#endif
			assert(xferred > 0);
			tgt->transient_state.in_count -= xferred;
			offset = tgt->transient_state.dma_offset;
			tgt->transient_state.dma_offset += xferred;
#ifndef PC532
			if (tgt->transient_state.dma_offset >= PER_TGT_BUFF_SIZE)
				tgt->transient_state.dma_offset = 0;

			/* copy what we got */
			sdp_copyin( tgt, offset, xferred, 0, 0/*/extrab*/);
#endif

			if (tgt->transient_state.in_count == 0)
			  /* Really was status phase */
			  tgt->transient_state.script = sdp_script_data_in + SCP_DATA_IN_STATUS;
			else
			  tgt->transient_state.script = sdp_script_data_in + SCP_DATA_IN_XFER;


		} else {

			register int xferred, offset;
			LOG(0x1d,"+STATUS+W");


			regs->sdp_icmd = SDP_ICMD_ENHANCED;

#ifdef PC532
			assert(sdp->resid == 0);
			xferred = sdp->out_count - sdp->resid;
#else
#error Get bytes not transferred from dma hardware somehow
			xferred = sdp->out_count - xferred;
			DP(1, printf("{O %x}", sdp->out_count));

			/* how much we xferred */
			xferred = sdp->out_count - count - 1;/*prefetch*/
#endif
			
			tgt->transient_state.out_count -= xferred;
			assert(tgt->transient_state.out_count > 0);
			offset = tgt->transient_state.dma_offset;
			tgt->transient_state.dma_offset += xferred;
#ifndef PC532
			if (tgt->transient_state.dma_offset >= PER_TGT_BUFF_SIZE)
			  tgt->transient_state.dma_offset = 0;
			
			sdp_copyout( tgt, offset, xferred);
#endif
			
			tgt->transient_state.script = sdp_script_data_out + SCP_DATA_OUT_XFER;
			sdp->out_count = 0;
		}
		break;
	default:
		gimmeabreak();
	}

	if (sdp_msg_in(sdp,csr,bs))
	  {
	    regs->sdp_sel_enb = (1 << sdp->sc->initiator_id);
	    sdp->script = sdp_script_disconnect;
	    return TRUE;
	  }
	else
	  return FALSE;
}

/*
 * Watchdog
 *
 */
int sdp_reset_scsibus(sdp_softc_t sdp)
{
        register target_info_t  *tgt = sdp->active_target;
	int old, s;

        if (tgt) {
		int cnt = 0;

                log(	LOG_KERN,
			"Target %d was active, cmd x%x in x%x out x%x Sin x%x Sou x%x dmalen x%x\n",
                        tgt->target_id, tgt->cur_cmd,
                        tgt->transient_state.in_count, tgt->transient_state.out_count,
                        sdp->in_count, sdp->out_count, cnt);
		gimmeabreak();
	}
#if 1
	s = splhi();
	old = PC532_SCSI_SELECT(ICU_DP);

	sdp_reset(sdp, FALSE);

	PC532_SCSI_SELECT(old);
	splx(s);
#else
        sdp->regs->sdp_icmd = SDP_ICMD_RST;
        delay(25);
        sdp->regs->sdp_icmd = SDP_ICMD_ENHANCED; /* CHECK SOMEDAY */
#endif
	return 0;
}

#endif	/*NSDP > 0*/


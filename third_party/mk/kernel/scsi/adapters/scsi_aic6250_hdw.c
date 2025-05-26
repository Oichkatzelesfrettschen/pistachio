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
 * 07-Aug-92  Ian Dall (ian) at University of Adelaide
 *	Move enabling of phase mismatch interrupts from
 *	aic_issue_command() to do_intr(). Also smarten up test for
 *	CMD_DONE(SPURIOUS).
 *
 * 06-Aug-92  Ian Dall (ian) at University of Adelaide
 *	Clear interrupts after enabling phase mismatch interrupts in
 *	aic_issue_command(). Otherwise get a spurious phase change
 *	interrupt with some targets.
 *
 * 04-Aug-92  Ian Dall (ian) at University of Adelaide
 *	Enable phase mismatch interrupts in aic_issue_command() instead
 *	of aic_complete_selection().
 *
 * 03-Aug-92  Ian Dall (ian) at University of Adelaide
 *	There is no point in waiting for bus free in aic_end_transaction()
 *	a) we might have missed it in a multi-initiator environment
 *	b) we shouldn't need to wait anyway.
 *
 * 03-Aug-92  Ian Dall (ian) at University of Adelaide
 *	Increased MAXLOG to 0x40.
 *
 * 03-Aug-92  Ian Dall (ian) at University of Adelaide
 *	Don't enable phase mis-match interrupts until selection
 *	successfully completed.
 *
 * 02-Aug-92  Ian Dall (ian) at University of Adelaide
 *	Enable phase mismatch interrupts only while we are active as an
 *	initiator.
 *
 * 02-Aug-92  Ian Dall (ian) at University of Adelaide
 *	Add test for null scp in do_aic_intr(). We can (but shouldn't?)
 *	get interrupts when we are not ready.
 *
 * $Log: scsi_aic6250_hdw.c,v $
 * Revision 1.22  1992/05/20  07:16:59  jtv
 * version before cleanup
 *
 * Revision 1.2  1992/04/24  16:48:14  jtv
 * 	Compiles and links
 *
 *
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
 * scsi_aic6250_hdw.c
 *
 * From spec: 	Adaptec, inc.
 *		AIC-6250, High-performance SCSI protocol chip
 *		PRELIMINARY, 1988
 *
 * 	Author: Jukka Virtanen,
 *		Helsinki University of Technology, Computing Centre
 *	Email:  jtv@hut.fi
 *	Date:	5/92
 *
 *	This driver is heavily based on the scsi chip drivers
 *	written by Alessandro Forin, CMU
 *
 *	Bottom layer of the SCSI driver: chip-dependent functions
 *
 *	This file contains the code that is specific to the AIC-6250
 *	SCSI chip (Host Bus Adapter in SCSI parlance): probing, start
 *	operation, and interrupt routine.
 */

/* If you use this please note:
 *  This driver is under construction. It does not yet support
 *  disconnects, target mode or synchronous transfers.
 *
 * If you have problems with some specific target, you can skip
 * its probing by setting the corresponding ID BIT in the
 * variable aic_skip_target (if you set it to 0xff the driver won't
 * probe anything)
 *
 * I have seen some disks dropping to kernel debugger when probed and
 * the phase does not match; but you can type a couple of continues
 * and it should again run. I will fix these, but jvh wanted to get this
 * out *now* so I have to apologize for the quality of this.
 *
 * If you attach both the AIC and the SDP drivers to the same
 * SCSI-cable, make sure you change the AIC ID something else
 * than 7. (SDP is 7). You can do this at the first break to
 * kernel debugger with the command (e.g. set the scsi id to 3,
 * when AIC is the unit number 0 as is the default):
 *
 * w/b scsi_initiator_id 3
 *
 * 					Juki
 *					jtv@hut.fi
 */

#include <aic.h>
#include <scsi.h>

#if	NAIC > 0
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

/* until fixed */
#define OLD_DMA
#define CLEAR_INTR
#define NO_SYNC

/* Use pseudo dma */
#define AIC_DMA_IN
#define AIC_DMA_OUT

#ifdef PC532
#include <ns532/PC532/board.h>
#endif

#include <scsi/adapters/scsi_aic6250.h>

#define DP(level, action) do { if ((level) <= scsi_debug) action; } while(0)

/*
 * The AIC can't tell you the scsi ID it uses, so
 * unless there is another way use the defaults
 */
#ifndef	my_scsi_id
#define	my_scsi_id(ctlr)	(scsi_initiator_id[(ctlr)])
#endif

/*
 * Statically partition the DMA buffer between targets.
 * This way we will eventually be able to attach/detach
 * drives on-fly.
 */
#define PER_TGT_DMA_SIZE	((AIC_RAM_SIZE/8) & ~(sizeof(int)-1))

#ifdef PC532
/* Don't align; we like hard debugging and small buffers */
#define PER_TGT_BUFF_SIZE		PER_TGT_DMA_SIZE

/* Max size of one pseudo-DMA burst in bytes */
#define PER_TGT_BURST_SIZE		8196
#else PC532
/*
 * Round to 4k to make debug easier
 */
#define	PER_TGT_BUFF_SIZE		((PER_TGT_DMA_SIZE >> 12) << 12)
#define PER_TGT_BURST_SIZE		(PER_TGT_BUFF_SIZE>>1)
#endif PC532

extern int current_scsi_device;

extern int aic_unit;

/* address of the mapped pseudo DMA space */
extern volatile char *pc532_dma;

char aic_default_state [ 16 ] = {
  0,	/* DMA counters */
  0,
  0,
  AICwIMR_ENACMDDONE | AICwIMR_ENASEL | AICwIMR_ENARESEL | AICwIMR_ENAERROR,
  AIC_DEFAULT_SYNC_RATE<<4,
  0,
#if 0
  AICwIMR1_PHASEMISMATCH | AICwIMR1_SCSIRST,
#else
  AICwIMR1_SCSIRST,
#endif
  AICwCNTRL0_OUTPORTA, /* aic_probe() will insert out SCSI ID here */
  (AIC_CLOCK_FREQUENCY > 10 ? AICwCONTROL1_CLOCKFREQ : 0)
    | AICwCONTROL1_ENAPORTB,
  0,
  0,
  0,	/* 11: Read only */
  0,	/* 12: Don't write */
  0xf,  /* 13: PORT A */
  0xbb, /* 14: PORT B */
  0
};

/*
 * Macros to make certain things a little more readable
 *
 * Side-effect: Select the AIC_REG_BUSOUT register (same as AIC_REG_BUSIN)
 */
#define	AIC_ACK(regs,phase) \
  AIC_WRITE(regs, AIC_REG_BUSOUT, SCSI_AIC_PHASE((phase)&0x7)|((phase)&0xf8))

#define	AIC_CLR_INTR(regs) \
  MACRO_BEGIN 			\
  AIC_REG (regs, AIC_REG_IMR);  \
  AIC_FORCE_IN (regs, 0); 	\
  AIC_REG (regs, AIC_REG_IMR);	\
  AIC_RESTORE (regs);		\
  AIC_REG (regs, AIC_REG_CNTRL0); \
  AIC_FORCE_IN (regs, 0);	\
  AIC_REG (regs, AIC_REG_CNTRL0); \
  AIC_RESTORE (regs);		\
  AIC_REG (regs, AIC_REG_IMR1); \
  AIC_FORCE_IN (regs, 0);	\
  AIC_REG (regs, AIC_REG_IMR1);	\
  AIC_RESTORE (regs);		\
  MACRO_END

#define ANY			0xff
#define	SCRIPT_MATCH(bs, cond)	(AIC_BSY_PHASE(bs) == (cond) || (cond) == ANY)

/* This is not a real PHASE */
#define	AIC_PHASE_DISC	0x0

/* forward decls of script actions */
boolean_t
	aic_dosynch(),			/* negotiate synch xfer */
  	aic_issue_command(),
	aic_xfer_in(),
	aic_xfer_out(),
	aic_get_status(),		/* get status from target */
	aic_end_transaction(),		/* all come to an end */
	aic_msg_in(),			/* get disconnect message(s) */
	aic_disconnected();		/* current target disconnected */
/* forward decls of error handlers */
boolean_t
	aic_err_generic(),		/* generic error handler */
	aic_err_disconn();		/* when a target disconnects */

int	aic_reset_scsibus();
boolean_t aic_probe_target();

scsi_ret_t	aic_select_target(), aic_complete_selection();

int aic_data_out(), aic_data_in();

/*
 * Scripts
 */
struct script

aic_script_data_in[] = {
        { SCSI_PHASE_CMD    | AIC_BUS_BSY,  aic_issue_command},
        { SCSI_PHASE_DATAI  | AIC_BUS_BSY,  aic_xfer_in},
        { SCSI_PHASE_STATUS | AIC_BUS_BSY,  aic_get_status},
        { SCSI_PHASE_MSG_IN | AIC_BUS_BSY,  aic_end_transaction}
},

aic_script_data_out[] = {
        { SCSI_PHASE_CMD    | AIC_BUS_BSY,  aic_issue_command},
        { SCSI_PHASE_DATAO  | AIC_BUS_BSY,  aic_xfer_out},
        { SCSI_PHASE_STATUS | AIC_BUS_BSY,  aic_get_status},
        { SCSI_PHASE_MSG_IN | AIC_BUS_BSY,  aic_end_transaction}
},

aic_script_cmd[] = {
        { SCSI_PHASE_CMD    | AIC_BUS_BSY,  aic_issue_command},
        { SCSI_PHASE_STATUS | AIC_BUS_BSY,  aic_get_status},
        { SCSI_PHASE_MSG_IN | AIC_BUS_BSY,  aic_end_transaction}
},

/* Synchronous transfer negotiation */
/* this does not work yet */
aic_script_try_synch[] = {
	{ SCSI_PHASE_MSG_OUT | AIC_BUS_BSY,  aic_dosynch},
	{ SCSI_PHASE_STATUS  | AIC_BUS_BSY,  aic_get_status},
	{ ANY, 				     aic_end_transaction}
},

/* Disconnect sequence */

aic_script_disconnect[] = {
	{ AIC_PHASE_DISC, aic_disconnected}
};

struct aic_controller_type aic_device[ NSCSI ];

struct aic_softc aic_softc_data[ NSCSI ];
aic_softc_t 	 aic_softc[ NSCSI ];

/*
 * Definition of the controller for the auto-configuration program.
 */

int	aic_probe(), scsi_slave(), scsi_attach(), aic_go(), aic_intr();

caddr_t	aic_std[ NSCSI ] = { 0, };
struct	bus_device *aic_dinfo[ NSCSI*8 ];
struct	bus_ctlr *aic_minfo[ NSCSI ];
struct	bus_driver aic_driver = 
        { aic_probe, scsi_slave, scsi_attach, aic_go, aic_std, "rz",
	  aic_dinfo, "aic", aic_minfo, BUS_INTR_B4_PROBE
	};

/*
 * Synch xfer parameters, and timing conversions
 */
int	aic_min_period = AIC_SYN_MIN_PERIOD;	 /* in cycles = f(CLK,FSn) */
int	aic_max_offset = AIC_INITIATOR_SYNC_MAX; /* target=8/initiator=7 */

/* Return the scsi sync transfer period in 4 nanosecond units */
int
aic_to_scsi_period(a)
     int a;
{
	unsigned int fs;
	int setup;
	int units;

	/* T: Period of Clock
	 * Sync speed = (4*T)+(AIC sync rate)*T;
	 * (AIC manual, page 21)
	 */

	/* best we can do is 200ns at 20Mhz, 4 cycles
	 * i.e. 50 in 4ns unit
	 */
	
	units = (a*250)/AIC_CLOCK_FREQUENCY;
	
	DP(1, printf("aic: sync (*4ns) units %d\n", units));
	return units;
}


/* Inverse of the above */
int
scsi_period_to_aic(units)
     int units;
{
  int ret;
  ret = (units*AIC_CLOCK_FREQUENCY)/250;

  if (ret < aic_min_period)
    return aic_min_period;

  if (aic_to_scsi_period (ret) < units)
    return ret + 1;

  return ret;
}

/*
 * This should be somewhere else, and it was a
 * mistake to share this buffer across SCSIs.
 */
struct dmabuffer {
	volatile char	*base;
	char		*sbrk;
} dmab[ NSCSI ];

volatile char *
aic_buffer_base(unit)
{
  if (unit >= NSCSI)
    return 0;
  else
    return dmab[unit].base;
}

aic_buffer_init(aic, unit, ram)
     char  **aic;	/* IN/OUT parameter */
     int   unit;
     volatile char	*ram;
{
  if (unit >= NSCSI)
    panic ("aic: no such scsi unit configured");

  if ((int)*aic & 1)
    panic ("aic: register selector address is not even");

  /* Assume pin 25 (MODE) is tied LOW -> 
   * even address accesses a "selector register" register
   * odd  address accesses the register specified in the selector register
   */
  aic_device[ unit ].aic_reg_selector = *aic;
  aic_device[ unit ].aic_reg_access   = *aic+1;
  aic_device[ unit ].reg   	      = 0;

  *aic = (char *)&aic_device[ unit ];

  dmab[ unit ].base = dmab[ unit ].sbrk = (char *) ram;
  bzero((char *) ram, AIC_RAM_SIZE);
}

char *
aic_buffer_sbrk(unit, size)
     int unit;
     int size;
{
  char	*ret = dmab[unit].sbrk;

  dmab[unit].sbrk += size;
  if ((dmab[unit].sbrk - dmab[unit].base) > AIC_RAM_SIZE)
    panic("aic_buffer_sbrk");
  return ret;
}

#define	u_min(a,b)	(((a) < (b)) ? (a) : (b))

#if MACH_KDB

#define TRACE

#define	PRINT(x) if (scsi_debug) printf x

aic_state(base)
     vm_offset_t	base;
{
  aic_controller_t	regs;
  int cnt, i;
  int dmac;
  int ss;
  int sreg;

#ifdef PC532
  ss = PC532_SCSI_SELECT(ICU_AIC);
#endif

  if (!base)
    base = (vm_offset_t)&aic_device[ aic_unit ];

  /* Remember that reg number 0..7 autoincrements on access */

  regs = (aic_controller_t)base;
  sreg = regs->reg;
  db_printf("scsi(aic):");

  AIC_REG (regs, 0);	/* Select DMA register 0 */
  AIC_READ_DMA_COUNT (regs, dmac);

  db_printf("DMA count 0..2: 0x%x ", dmac);

  AIC_REG (regs, 5);
  db_printf("fifo stat(5): 0x%x ", AIC_VAL(regs));
  db_printf("rev      (6): 0x%x\n", AIC_VAL(regs)&3);
  db_printf("status 0 (7): 0x%x ", AIC_VAL(regs));
  AIC_REG (regs, 8);
  db_printf("status 1 (8): 0x%x\n", AIC_VAL(regs));
  AIC_REG (regs, 9);
  db_printf("scsi sigs(9): 0x%x, scsi phase : %d\n",
	    AIC_VAL (regs), AIC_SCSI_PHASE_FETCH (regs));
  AIC_REG (regs, 10);
  db_printf("id/data  (A): 0x%x ", AIC_VAL (regs));
  AIC_REG (regs, 11);
  db_printf("src/dest (B): 0x%x ", AIC_VAL (regs));
  AIC_REG (regs, 12);
  db_printf("mem data (C): 0x%x\n", AIC_VAL (regs));
  AIC_REG (regs, 13);
  db_printf("port A   (D): 0x%x ", AIC_VAL (regs));
  AIC_REG (regs, 14);
  db_printf("port B   (E): 0x%x ", AIC_VAL (regs));
  AIC_REG (regs, 15);
  db_printf("scsilatch(F): 0x%x\n", AIC_VAL (regs));

  AIC_REG (regs, sreg);

  db_printf("saved state:\n");
  for (i = 0; i <= 15; i++)
      db_printf ("reg %d : 0x%x\n", i, regs->state[ i ]&0xff);
#ifdef PC532
  PC532_SCSI_SELECT (ss);
#endif
  return 0;
}

aic_target_state(tgt)
     target_info_t *tgt;
{
  if (tgt == 0)
    tgt = aic_softc[0]->active_target;

  if (tgt == 0)
    return 0;

  db_printf("fl %x dma %x+%x cmd %x id %x per %x off %x ior %x ret %x\n",
	    tgt->flags, tgt->dma_ptr, tgt->transient_state.dma_offset,
	    tgt->cmd_ptr, tgt->target_id, tgt->sync_period,
	    tgt->sync_offset, tgt->ior, tgt->done);
  if (tgt->flags & TGT_DISCONNECTED)
    {
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

aic_all_targets(unit)
{
  int i;
  target_info_t	*tgt;

  for (i = 0; i < 8; i++)
    {
      tgt = aic_softc[unit]->sc->target[i];
      if (tgt)
	aic_target_state(tgt);
    }
}

aic_script_state(unit)
{
  script_t spt = aic_softc[unit]->script;

  if (spt == 0)
    return 0;

  db_printsym(spt,1);
  db_printf(": %x ", spt->condition);
  db_printsym(spt->action,1);
  db_printf(", ");
  db_printsym(aic_softc[unit]->error_handler, 1);
  return 0;
}

#ifdef TRACE

#define TRMAX 200
int aic_tr[TRMAX+10];
char *aic_trname[TRMAX+10];
int aic_trpt, aic_trpthi;

#define	TR(x)	do { aic_trname[aic_trpt] = "?"; aic_tr[aic_trpt++] = x; } while(0)
#define TRWRAP	aic_trpthi = aic_trpt; aic_trpt = 0;
#define TRCHECK	if (aic_trpt > TRMAX) {TRWRAP}
#define	TR2(name, x)	do { aic_trname[aic_trpt] = name; aic_tr[aic_trpt++] = x; } while(0)

#define LOGSIZE 256
int aic_logpt;
char aic_log[LOGSIZE];

/* Free: 0x25 0x2c 0x2d */
#define MAXLOG_VALUE	0x40
struct {
	char *name;
	unsigned int count;
} aic_logtbl[MAXLOG_VALUE];

static LOG(e,f)
     char *f;
{
  aic_log[aic_logpt++] = (e);

  if (aic_logpt == LOGSIZE)
    aic_logpt = 0;

  if ((e) < MAXLOG_VALUE)
    {
      aic_logtbl[(e)].name = (f);
      aic_logtbl[(e)].count++;
    }
}

aic_print_log(skip)
     int skip;
{
  register int i, j;
  register unsigned char c;

  for (i = 0, j = aic_logpt; i < LOGSIZE; i++)
    {
      c = aic_log[j];
      if (++j == LOGSIZE) j = 0;
      if (skip-- > 0)
	continue;
      if (c < MAXLOG_VALUE)
	db_printf(" %s", aic_logtbl[c].name);
      else
	db_printf("-%d", c & 0x7f);
    }
  db_printf("\n");
  return 0;
}

aic_print_trace(skip)
     int skip;
{
  int i;
	
  db_printf ("aic_trpthi = 0x%x, aic_trpt = 0x%x (%d)\n", aic_trpthi, aic_trpt, aic_trpt);
  for (i = aic_trpt; i < aic_trpthi; i++)
    {
      if (skip > 0)
	skip--;
      else
	db_printf ("%x\t%s\t%x\n", i, aic_trname[i], aic_tr[i]);
    }
  
  for (i = 0; i < aic_trpt; i++)
    {
      if (skip > 0)
	skip--;
      else
	db_printf ("%x\t%s\t%x\n", i, aic_trname[i], aic_tr[i]);
    }
}

aic_print_stat()
{
  register int i;
  register char *p;

  for (i = 0; i < MAXLOG_VALUE; i++)
    {
      if (p = aic_logtbl[i].name)
	printf("%d %s\n", aic_logtbl[i].count, p);
    }
}

#else	/* TRACE */
#define	LOG(e,f)
#endif	/* TRACE */

#else	/* MACH_KDB */
#define	PRINT(x)
#define	LOG(e,f)
#define TR(x)
#define	TR2(name, x)
#define TRCHECK
#define TRWRAP
#endif	/* MACH_KDB */

aic_break()
{
}
aic_break1()
{
}
aic_break2()
{
}
aic_break3()
{
}
aic_break4()
{
}
aic_break5()
{
}

/*
 *	Probe/Slave/Attach functions
 */

/*
 * Probe routine:
 *	Should find out (a) if the controller is
 *	present and (b) which/where slaves are present.
 *
 * Implementation:
 *	Send an test-unit-ready msg to each possible target on the bus
 *	except of course ourselves.
 *
 */

/* For LED-panel output */
int aic_initialized = 0;

/* Skip target probing if corresponding bit is set */
int aic_skip_target = 0xfc;

aic_probe(reg, ui)
     char		*reg;
     struct bus_ctlr	*ui;
{
  aic_controller_t	regs;

  int           unit = ui->unit;
  aic_softc_t   aic = &aic_softc_data[unit];
  int		target_id, i;
  scsi_softc_t	*sc;
  int		s;
  boolean_t	did_banner = FALSE;
  char		*cmd_ptr;
  static int    probed = 0;
  int ss;

  /*
   * We are only called if the chip is there,
   * but make sure anyways..
   */

  LOG(0x30,"\nprobe");

  if (aic_initialized)
    panic ("Already probed the AIC controller");

  aic_initialized++;

  regs = (aic_controller_t) (reg);

  for (s = 0; s <= 15; s++)
      regs->state[ s ] = 0xff;

  /* Sometimes we need the softc when we have the regs */
  regs->aic = aic;

  /*
   * Initialize hw descriptor
   */
  aic_softc[unit] = aic;
  aic->regs = regs;
  aic->buff = aic_buffer_base(unit);

  queue_init(&aic->waiting_targets);

  sc = scsi_master_alloc(unit, aic);
  sc->go = aic_go;
  sc->probe = aic_probe_target;
  sc->watchdog = scsi_watchdog;

  /* @@Fix this: dma_out & dma_in should take care of the dma transfer
     partitioning instead of this.
   */
  sc->max_dma_data = PER_TGT_BURST_SIZE;

  aic->sc = sc;
  aic->wd.reset = aic_reset_scsibus;

  /*
   * Reset chip
   */
#if defined(PC532)
  s = splhi();
  splx(5);
#else
  s = splbio();
#endif PC532

#ifdef PC532
  ss = PC532_SCSI_SELECT (ICU_AIC);
#endif

  /*
   * Our SCSI id on the bus.
   */
  aic_default_state [ AIC_REG_CNTRL0 ] |= my_scsi_id(unit);
  sc->initiator_id = my_scsi_id(unit);

  aic_reset(aic, TRUE);


  printf("%s%d: my SCSI id is %d", ui->name, unit, sc->initiator_id);
  
  /*
   * For all possible targets, see if there is one and allocate
   * a descriptor for it if it is there.
   */
  cmd_ptr = aic_buffer_sbrk(unit, 0);

  aic->state |= AIC_STATE_PROBING;
  for (target_id = 0; target_id < 8; target_id++)
    {
      scsi_status_byte_t status;

      /* except of course ourselves */
      if (target_id == sc->initiator_id)
	continue;

      if (aic_skip_target & (1 << target_id))
	  {
	    printf ("aic: target id %d will not be probed\n", target_id);
	    continue;
	  }

      if (aic_select_target (regs, sc->initiator_id,
			     target_id, FALSE) == SCSI_RET_DEVICE_DOWN)
	{
	  AIC_REG (regs, AIC_REG_STAT0);
	  /* Interrupts are blocked */
	  if (AIC_VAL (regs) & AICrSTAT0_SCSIRST)
	      aic_bus_reset (aic);

	  AIC_CLR_INTR(regs);
	  DP(2, printf("aic: target %d is down\n", target_id));
	  continue;
	}

      printf(",%s%d", did_banner++ ? " " : " target(s) at ",
	     target_id);
      
      /* must be command phase here: we selected wo ATN! */
      while(aic_phase_wait (regs, SCSI_PHASE_CMD) != SCSI_PHASE_CMD)
	/* wait */;

      /* build command in dma area */
      {
	unsigned char	*p = (unsigned char*) cmd_ptr;

	p[0] = SCSI_CMD_TEST_UNIT_READY;
	p[1] = p[2] = p[3] = p[4] = p[5] = 0;
      }

      aic_data_out (regs, SCSI_PHASE_CMD, 6, cmd_ptr);

      while(aic_phase_wait (regs, SCSI_PHASE_STATUS) != SCSI_PHASE_STATUS)
	/* wait */;

      aic_data_in(regs, SCSI_PHASE_STATUS, 1, &status.bits);

      if (status.st.scsi_status_code != SCSI_ST_GOOD)
	scsi_error (0, SCSI_ERR_STATUS, status.bits, 0);

      /* get command_complete message */

      while(aic_phase_wait (regs, SCSI_PHASE_MSG_IN) != SCSI_PHASE_MSG_IN)
	/* wait */;

      aic_data_in(regs, SCSI_PHASE_MSG_IN, 1, &i);

      /* check disconnected, clear all intr bits */
      AIC_REG (regs, AIC_REG_BUSIN);
      while (AIC_VAL (regs) & AICrBUSIN_BSY)
	/* wait */;

      /* Bus free */
      AIC_ACK(regs, AIC_PHASE_DISC);

      AIC_CLR_INTR(regs);

      /*
       * Found a target
       */
      aic->ntargets++;
      {
	register target_info_t	*tgt;

	tgt = scsi_slave_alloc (unit, target_id, aic);
	
	/* "virtual" address for our use */
	tgt->cmd_ptr = aic_buffer_sbrk (unit, PER_TGT_DMA_SIZE);
	
	/* pc532 does not have true dma */
	tgt->dma_ptr = (char *)0xbadfeed;
      }
    }
  printf(".\n");
  aic->state &= ~AIC_STATE_PROBING;

#ifdef PC532  
  PC532_SCSI_SELECT (ss);
#endif
  
  splx_no_check(s);
  return 1;
}

boolean_t
aic_probe_target(tgt, ior)
     target_info_t *tgt;
     io_req_t	    ior;
{
  aic_softc_t aic = aic_softc[tgt->masterno];
  boolean_t newlywed;

  newlywed = (tgt->cmd_ptr == 0);
  if (newlywed)
    {
      /* desc was allocated afresh */

      /* "virtual" address for our use */
      /* @@@@ aic_unit should be looked up! */
      tgt->cmd_ptr = aic_buffer_sbrk (aic_unit, PER_TGT_DMA_SIZE);

      /* pc532 does not have true dma */
      tgt->dma_ptr = (char *)0xebadfeed;
    }

  if (scsi_inquiry (tgt, SCSI_INQ_STD_DATA) == SCSI_RET_DEVICE_DOWN)
    return FALSE;
  
  tgt->flags = TGT_ALIVE;
  return TRUE;
}

/*
 * Note: Does not work if called with selected register in range 0..7
 */
aic_wait (regs, mask, value)
     aic_controller_t regs;
     unsigned char mask;
     unsigned char value;
{
  int timeo = 10000;

  /* read it over to avoid bus glitches */
  while ((AIC_VAL(regs) & mask) != value
	 || (AIC_VAL(regs) & mask) != value
	 || (AIC_VAL(regs) & mask) != value)
    {
      if (!timeo--) {
	printf("aic_wait TIMEOUT: mask 0x%x in reg %d (0x%x) expected 0x%x\n",
	       mask, regs->reg, AIC_VAL(regs), value);
	break;
      }

      if (timeo < 1000)
	delay (10);
    }

  return AIC_VAL (regs) & mask;
}

aic_phase_wait (regs, phase)
     aic_controller_t regs;
     unsigned char phase;
{
  int curphase = phase;
  int timeo = 10000;

  AIC_REG (regs, AIC_REG_BUSIN);

  /* read it over to avoid bus glitches */
  while (AIC_SCSI_PHASE_FETCH(regs) != phase
	 || AIC_SCSI_PHASE_FETCH(regs) != phase
	 || AIC_SCSI_PHASE_FETCH(regs) != phase)
    {
      if (!timeo--) {
	curphase = AIC_SCSI_PHASE_FETCH (regs);
	printf("aic_phase_wait TIMEOUT: phase now %d, expected %d\n",
	       curphase, phase);
	break;
      }

      /* Start to wait in longer chunks */
      if (timeo < 600)
	delay(10);
    }

  return curphase;
}

scsi_ret_t
aic_select_target (regs, myid, id, with_atn)
     volatile aic_controller_t	regs;
     unsigned char		myid, id;
     boolean_t			with_atn;
{
  unsigned char bit_id;
  scsi_ret_t ret = SCSI_RET_RETRY;
  unsigned char cntrl0, imr;
  unsigned char stat1;
  aic_softc_t aic = regs->aic;
	
  bit_id = (1 << myid) | (1 << id);

  /* Save the parameters we wrote last time to CNTRL0 register */
  cntrl0 = regs->state[ AIC_REG_CNTRL0 ];
  imr    = regs->state[ AIC_REG_IMR    ];

  if ((cntrl0 & AICwCNTRL0_IDMASK) != myid)
    printf ("aic: selecting when myid %d does not match aic id %d\n",
	    myid, cntrl0 & AICwCNTRL0_IDMASK);

  AIC_WRITE (regs, AIC_REG_CNTRL0, cntrl0 & ~AICwCNTRL0_TARGET);

  AIC_WRITE (regs, AIC_REG_SCSI_ID_DATA, bit_id);

  AIC_WRITE (regs, AIC_REG_IMR,
	     aic_default_state [ AIC_REG_IMR ]
	     | AICwIMR_ENASELOUT| AICwIMR_ARBSEL
	     | (with_atn ? AICwIMR_ENAAUTOATN : 0));


  if (with_atn)
    aic->state |= AIC_STATE_WITH_ATN;
  else
    aic->state &= ~AIC_STATE_WITH_ATN;

  if (! (aic->state & AIC_STATE_PROBING))
    return;

  /* Now wait till we have won the bus. If someone else selects
   * or reconnects to us: we get an interrupt and the ARBSEL bit
   * is reset so we know to return a failure code for this
   * operation when the interrupt returns here.
   */
  
  DP(1, aic_break4());
  AIC_REG (regs, AIC_REG_STAT1);

  while (((stat1 = AIC_VAL (regs)) & WAITFOR) == 0)
    {
      if (regs->state[ AIC_REG_IMR ] & AICwIMR_ARBSEL == 0)
	{
	  DP(1, printf("aic: select imr&arbsel==0\n"));
	  AIC_WRITE (regs, AIC_REG_IMR, imr);
	  return ret;
	}
    }

  return aic_complete_selection (aic, stat1);
}

scsi_ret_t
aic_complete_selection (aic, stat1)
     aic_softc_t aic;
     unsigned char stat1;
{  
  aic_controller_t regs = aic->regs;
  boolean_t done;

  AIC_REG (regs, AIC_REG_STAT1);

  done = !!((stat1 | AIC_VAL (regs)) & AICrSTAT1_CMDDONE);

  TR2("aic:select complete parm stat1", stat1);
  TR2("aic:select complete read stat1", AIC_VAL (regs));

  DP(1, aic_break4());

  /* Now we own the bus and either already selected or just begin to
   * select the correct target.
   */
  if (! done)
    {
      /* Just begun selecting the target; timeout this
       * so we won't hang on nonexisting/broken targets
       */
      int timeo = 2500;/* 250 msecs in 100 usecs chunks */
      while (! (AIC_VAL (regs) & AICrSTAT1_CMDDONE))
	if (--timeo > 0)
	  delay(100);
	else
	  goto nodev;
    }
  
  AIC_WRITE (regs, AIC_REG_IMR,
	     aic_default_state [ AIC_REG_IMR ]
	     | ((aic->state & AIC_STATE_WITH_ATN) ? AICwIMR_ENAAUTOATN : 0));
  

  DP (1, aic_break3());

  /* Check if we selected with ATN asserted */
  if (aic->state & AIC_STATE_WITH_ATN)
    {
      boolean_t ok;
      target_info_t *tgt = aic->active_target;

      DP(1,aic_break5());
      while(aic_phase_wait (regs, SCSI_PHASE_MSG_OUT) != SCSI_PHASE_MSG_OUT)
	/* wait */;

      if (tgt->flags & TGT_TRY_SYNCH)	/* Sync negotiation? */
	ok = aic_dosynch (aic, 0, 0);
      else				/* Identify field != 0xff */
	{
	  /* Reset ATN */
	  AIC_DEFAULT (regs, AIC_REG_IMR);
      
	  ok = (aic_data_out(regs, SCSI_PHASE_MSG_OUT,
			     1, &tgt->transient_state.identify) == 0);
	}
      
    }
  return SCSI_RET_SUCCESS;

nodev:
  DP (2, printf("aic: selection timeout, target ?\n"));
  DP (1, aic_break3());
  
  if (! (aic->state & AIC_STATE_PROBING))
    {
      aic->done = SCSI_RET_DEVICE_DOWN;
      aic_end (aic, 0, 0);
    }

  {
    u_char temp;
    /* Try to recover from the selection failure
     * (Adaptec manual pages 31 & 49)
     */
    AIC_WRITE (regs, AIC_REG_SCSI_ID_DATA, 0);
    delay (200);
    AIC_READ (regs, AIC_REG_BUSIN, temp);
    temp &= AICrBUSIN_BSY;

    AIC_DEFAULT (regs, AIC_REG_IMR);
    AIC_DEFAULT (regs, AIC_REG_CNTRL0);

    /* If bus is still busy, do a SCSI RESET
     * @@@ How about other master won the bus?
     */
    if (temp)
      aic_reset_scsibus (regs->aic);
  }
  return SCSI_RET_DEVICE_DOWN;
}

aic_data_out (regs, phase, count, data)
     volatile register aic_controller_t regs;
     int phase;
     int count;
     unsigned char *data;
{
  unsigned char control1 = regs->state[ AIC_REG_CONTROL1 ];
  unsigned char stat0;

  if (phase&0xf >= 6)
    {
      LOG(8, "message_out");
      TR2("message_out", count);
    }
  else
    {
      LOG(8, "data_out");
      TR2("data out", count);
    }
  AIC_ACK (regs, phase);
  do {

#define CHECK_OUT_PHASE
#ifdef CHECK_OUT_PHASE
    AIC_REG (regs, AIC_REG_BUSIN);
    if (! aic_wait (regs, AICrBUSIN_REQ, AICrBUSIN_REQ))
      {
	DP (1, printf("aic: data out REQ failed, count %d\n", count));
	aic_break4();
	return count;
      }

    AIC_READ (regs, AIC_REG_STAT0, stat0);
    if (stat0 & AICrSTAT0_PHASEMISMATCH)
      {
	DP(2, printf ("aic: data out PHASE MISMATCH, count %d\n", count));
	aic_break4();
	return count;
      }

    if (stat0 & AICrSTAT0_REQ == 0)
      {
	panic ("AIC claims that REQ has not been asserted(aic_data_out)");
	aic_break4();
	return count;
      }
#endif
    AIC_WRITE (regs, AIC_REG_SCSI_ID_DATA, *data++);
    AIC_WRITE (regs, AIC_REG_CONTROL1, control1 | AICwCONTROL1_AUTOPIO);
    AIC_REG (regs, AIC_REG_STAT1);
    if (!aic_wait (regs, AICrSTAT1_CMDDONE, AICrSTAT1_CMDDONE))
      {
	DP(1, printf("data out DONE wait failed, count %d\n", count));
	TR2("data out failed", phase);
	aic_break4();
#ifdef CLEAR_INTR
	AIC_CLR_INTR (regs); /* @@@ see below */
#endif
	return count;
      }
  } while (--count > 0);

#ifdef CLEAR_INTR
  AIC_CLR_INTR (regs);/* @@@ Check why it does not work if this is removed */
#endif

  return 0;
}

aic_data_in (regs, phase, count, data)
     volatile aic_controller_t regs;
     int phase;
     int count;
     unsigned char	   *data;
{
  int s;
  unsigned char stat0;
  unsigned char control1 = regs->state[ AIC_REG_CONTROL1 ];

  /* Prevent interrupts now to protect against 32532 data prefetch */

  if (phase&0xf >= 6)
    {
      LOG(7, "message_in");
      TR2("message in", count);
    }
  else
    {
      LOG (7, "data_in");
      TR2("data in", count);
    }
  AIC_ACK (regs, phase);
  s = splhi ();
  do {

    AIC_REG (regs, AIC_REG_BUSIN);
    if (! aic_wait (regs, AICrBUSIN_REQ, AICrBUSIN_REQ))
      {
	DP (1, printf("aic: data in REQ failed, count %d\n", count));
	aic_break4();
	splx (s);
	return count;
      }

    AIC_READ (regs, AIC_REG_STAT0, stat0);
    if (stat0 & AICrSTAT0_PHASEMISMATCH)
      {
	DP (2, printf("aic: data in PHASE MISMATCH, count %d\n", count));
	aic_break4();
	splx (s);
	return count;
      }

    if (stat0 & AICrSTAT0_REQ == 0)
      {
	panic ("AIC claims that REQ has not been asserted(aic_data_in)");
	aic_break4();
	splx (s);
	return count;
      }

    /* Everything OK, get the data byte */
    AIC_REG (regs, AIC_REG_SCSI_ID_DATA);
    *data++ = AIC_VAL (regs);

    AIC_WRITE (regs, AIC_REG_CONTROL1, control1 | AICwCONTROL1_AUTOPIO);

    if (! aic_wait (regs, AICrSTAT1_CMDDONE, AICrSTAT1_CMDDONE))
      {
	DP (1, printf("aic: data in CMD done, count %d\n", count));
	TR2("data in cmd done", phase);
	aic_break4();
	splx (s);
	return count;
      }
  } while (--count > 0);

  splx(s);
  return 0;
}

aic_normal_mode (regs)
     aic_controller_t regs;
{
  LOG(0x31, "normal");
  AIC_DEFAULT(regs, AIC_REG_CONTROL1);	/* 8 */
  AIC_DEFAULT(regs, AIC_REG_CNTRL0);	/* 7 */
  AIC_DEFAULT(regs, AIC_REG_OFFSET);	/* 4 */

  AIC_DEFAULT(regs, AIC_REG_IMR);	/* 3 */
  AIC_DEFAULT(regs, AIC_REG_IMR1);	/* 6 */

  AIC_WRITE_DMA_COUNT (regs, 0);	/* 0..2 */

  AIC_DEFAULT(regs, AIC_REG_DMA);	/* 5 */

  AIC_DEFAULT(regs, AIC_REG_PORT_A);	/* 13 */
}

aic_reset (aic, quickly)
     aic_softc_t aic;
     boolean_t   quickly;
{
  aic_controller_t regs = aic->regs;
  int ss;

  ss = splhi ();

  /* Wait until the SCSI reset condition passes */
  AIC_REG (regs, AIC_REG_STAT1);
  while (AIC_VAL (regs) & AICrSTAT1_SCSIRESETIN)
    ;

  /* Reset AIC-6250. */
  AIC_WRITE (regs, AIC_REG_CONTROL1, AICwCONTROL1_SOFTRESET);
  delay (200);
  AIC_WRITE (regs, AIC_REG_CONTROL1, 0); /* Clear the reset condition */

  if (! quickly)
    {
      /* Reset also the SCSI bus. Later, we will get an interrupt and
       * reset the chip twice. Who cares!
       */
      AIC_WRITE (regs, AIC_REG_CONTROL1,
		 aic_default_state [ AIC_REG_CONTROL1 ]
		   | AICwCONTROL1_SCSIRESET);
      delay (250);
      AIC_DEFAULT (regs, AIC_REG_CONTROL1);

      delay (scsi_delay_after_reset); /* some targets take long to reset */
    }

  aic_normal_mode (regs);
  if (! quickly)
    DP (1, printf("aic: slow aic_reset done\n"));
  else
    DP (1, printf("aic: quick aic_reset done\n"));
  splx (ss);
}

/*
 *	Operational functions
 */

int aic_current_scsi_cmd = 0;

/* Debug first without sync negotiation. If you set this to 1,
 * it will crash until code is fixed
 */
#ifdef NO_SYNC
int aic_sync = 0;
#else
int aic_sync = 1;
#endif

/*
 * Start a SCSI command on a target
 */
aic_go (tgt, cmd_count, in_count, cmd_only)
     target_info_t *tgt;
     int            cmd_count;
     int            in_count;
     boolean_t	    cmd_only;
{
  aic_softc_t  aic;
  boolean_t    disconn;
  script_t     scp;
  boolean_t    (*handler)();
  int s;

  LOG(1,"\ngo");

  aic = (aic_softc_t)tgt->hw_state;

  if ((tgt->cur_cmd == SCSI_CMD_WRITE)
      || (tgt->cur_cmd == SCSI_CMD_LONG_WRITE))
    {
      io_req_t	ior = tgt->ior;
      int   	len = ior->io_count;
    
      tgt->transient_state.out_count = len;
#if  PC532
      /* No need to copy stuff for DMA */
      tgt->transient_state.copy_count = 0;
#else
      if (len > PER_TGT_BUFF_SIZE)
	len = PER_TGT_BUFF_SIZE;

      bcopy (ior->io_data, tgt->cmd_ptr + cmd_count, len);
      
      tgt->transient_state.copy_count = len;
#endif
      
      /* avoid leaks */
      /* @@@ What if the tgt->block_size is HUGE?? */
      if (len < tgt->block_size)
	{
#ifndef PC532
	  bzero (tgt->cmd_ptr + cmd_count + len, tgt->block_size - len);
#endif
	  tgt->transient_state.out_count = tgt->block_size;
	}
    }
  else
    {
      tgt->transient_state.out_count = 0;
      tgt->transient_state.copy_count = 0;
    }

  tgt->transient_state.cmd_count = cmd_count;

  disconn  = BGET(scsi_might_disconnect,aic->sc->masterno,tgt->target_id);
  disconn  = disconn && (aic->ntargets > 1);
  disconn |= BGET(scsi_should_disconnect,aic->sc->masterno,tgt->target_id);
#ifdef PC532
  /* @@@ AIC driver does not support disconnects yet */
  disconn = 0;
#endif PC532

  /*
   * Setup target state
   */
  tgt->done = SCSI_RET_IN_PROGRESS;

  handler = (disconn) ? aic_err_disconn : aic_err_generic;

  DP (0, (aic_current_scsi_cmd = tgt->cur_cmd));

  switch (tgt->cur_cmd) {
  case SCSI_CMD_READ:
  case SCSI_CMD_LONG_READ:
    LOG (0x13,"readop");
    scp = aic_script_data_in;
    break;
  case SCSI_CMD_WRITE:
  case SCSI_CMD_LONG_WRITE:
    LOG (0x14,"writeop");
    scp = aic_script_data_out;
    break;
  case SCSI_CMD_INQUIRY:
    /* This is likely the first thing out: do the synch neg if so */

    if (aic_sync && !cmd_only && (tgt->flags&TGT_DID_SYNCH)==0) {
      scp = aic_script_try_synch;
      tgt->flags |= TGT_TRY_SYNCH;
      break;
    }
    /* FALLTHROUGH */

  case SCSI_CMD_REQUEST_SENSE:
  case SCSI_CMD_MODE_SENSE:
  case SCSI_CMD_RECEIVE_DIAG_RESULTS:
  case SCSI_CMD_READ_CAPACITY:
  case SCSI_CMD_READ_BLOCK_LIMITS:
    scp = aic_script_data_in;
    LOG (0x1c,"cmdop");
    LOG (0x80+tgt->cur_cmd,0);
    break;
  case SCSI_CMD_MODE_SELECT:
  case SCSI_CMD_REASSIGN_BLOCKS:
  case SCSI_CMD_FORMAT_UNIT:
    tgt->transient_state.cmd_count = sizeof(scsi_command_group_0);
    tgt->transient_state.out_count = cmd_count - sizeof(scsi_command_group_0);
    scp = aic_script_data_out;
    LOG (0x1c,"cmdop");
    LOG (0x80+tgt->cur_cmd,0);
    break;
  case SCSI_CMD_TEST_UNIT_READY:
    /*
     * Do the synch negotiation here, unless prohibited or done already
     */
    if (! aic_sync || tgt->flags & TGT_DID_SYNCH)
      scp = aic_script_cmd;
    else
      {
	scp = aic_script_try_synch;
	tgt->flags |= TGT_TRY_SYNCH;
	cmd_only = FALSE;
      }
    LOG (0x1c,"cmdop");
    LOG (0x80+tgt->cur_cmd,0);
    break;
  default:
    LOG (0x1c,"cmdop");
    LOG (0x80+tgt->cur_cmd,0);
    scp = aic_script_cmd;
  }
  
  tgt->transient_state.script   = scp;
  tgt->transient_state.handler  = handler;
  tgt->transient_state.identify = (cmd_only) ? 0xff :
    (disconn ? SCSI_IDENTIFY|SCSI_IFY_ENABLE_DISCONNECT
     	     : SCSI_IDENTIFY);
  
  if (in_count)
    tgt->transient_state.in_count =
      (in_count < tgt->block_size) ? tgt->block_size : in_count;
  else
    tgt->transient_state.in_count = 0;

  tgt->transient_state.dma_offset = 0;

  /*
   * See if another target is currently selected on
   * this SCSI bus, e.g. lock the aic structure.
   * Note that it is the strategy routine's job
   * to serialize ops on the same target as appropriate.
   */
#if 0
  locking code here
#endif
  s = splbio ();

  if (aic->wd.nactive++ == 0)
    aic->wd.watchdog_state = SCSI_WD_ACTIVE;
  
  if (aic->state & AIC_STATE_BUSY)
    {
      /*
       * Queue up this target, note that this takes care
       * of proper FIFO scheduling of the scsi-bus.
       */
      LOG (3,"enqueue");
      enqueue_tail (&aic->waiting_targets, (queue_entry_t) tgt);
    }
  else
    {
      /*
       * It is down to at most two contenders now,
       * we will treat reconnections same as selections
       * and let the scsi-bus arbitration process decide.
       */
      aic->state |= AIC_STATE_BUSY;
      aic->next_target = tgt;
      aic_attempt_selection(aic);
      /*
       * Note that we might still lose arbitration..
       */
    }
  splx (s);
}

/* State is not changed before we win the bus; this way the
 * reselection interrupt does not foul things up.
 *
 */
aic_attempt_selection (aic)
     aic_softc_t aic;
{
  target_info_t	*tgt;
  aic_controller_t	regs;
  register int	cmd;
  boolean_t	ok;
  scsi_ret_t	ret;
  int do_init = 1;
  int ss;
  boolean_t with_atn = FALSE;
  unsigned char stat1;

  regs = aic->regs;

  tgt = aic->next_target;

  LOG (4,"aselect");
  LOG (0x80+tgt->target_id,0);

  ss = PC532_SCSI_SELECT (ICU_AIC);

#if 1
  AIC_READ (regs, AIC_REG_STAT1, stat1);
  /* Why should we want to make this check? If there really is a
   * re-select we should handle it with an interrupt. See if the
   * printf ever executes.
   */
  if (stat1 & AICrSTAT1_RESELECTED)
    {
      /* Check if reconnect was for us */
      int id;
      AIC_READ (regs, AIC_REG_SOURCE_DEST, id);
      if( id & (1 << aic->sc->initiator_id))
	{
	  printf ("aic: Reselection conflict? id = 0x%x\n", id);
	  return;
	}
    }


  aic->active_target = tgt;
  aic->script = tgt->transient_state.script;
  aic->error_handler = tgt->transient_state.handler;
  aic->done = SCSI_RET_IN_PROGRESS;

  aic->in_count = 0;
  aic->out_count = 0;

  if (tgt->flags & TGT_DID_SYNCH)
    with_atn = tgt->transient_state.identify != 0xff;
  else 
    with_atn = !!(tgt->flags & TGT_TRY_SYNCH);

  if (with_atn)
    aic->state |= AIC_STATE_WITH_ATN;

  ret = aic_select_target (regs, aic->sc->initiator_id,
			   tgt->target_id, with_atn);

  PC532_SCSI_SELECT (ss);
  return;
#else  
  /* Old code; was not interrupt driven */
  /*
   * This is a bit involved, but the bottom line is we want to
   * know after we selected with or w/o ATN if the selection
   * went well (ret) and if it is (ok) to send the command.
   */
  ok = TRUE;
  if (tgt->flags & TGT_DID_SYNCH)
    {
      if (tgt->transient_state.identify == 0xff)
	{
	  /* Select w/o ATN */
	  ret = aic_select_target(regs, aic->sc->initiator_id,
				  tgt->target_id, FALSE);
	}
      else
	{
	  /* Select with ATN */
	  ret = aic_select_target(regs, aic->sc->initiator_id,
				  tgt->target_id, TRUE);

	  if (ret == SCSI_RET_SUCCESS)
	    {
	      while(aic_phase_wait (regs, SCSI_PHASE_MSG_OUT) !=
		    SCSI_PHASE_MSG_OUT)
		/* wait */;
	      
	      /* @@@ Reset ENAAUTOATN bit. XXXX */
	      AIC_DEFAULT (regs, AIC_REG_IMR);

	      ok = (aic_data_out(regs, SCSI_PHASE_MSG_OUT,
				 1, &tgt->transient_state.identify) == 0);
	    }
	}
    }
  else if (tgt->flags & TGT_TRY_SYNCH)
    {
      /* Select with ATN, do the synch xfer neg */
      ret = aic_select_target(regs, aic->sc->initiator_id,
			      tgt->target_id, TRUE);

      if (ret == SCSI_RET_SUCCESS)
	{
	  aic->active_target = tgt;
	  aic->script = tgt->transient_state.script;
	  aic->error_handler = tgt->transient_state.handler;
	  aic->done = SCSI_RET_IN_PROGRESS;

	  aic->in_count = 0;
	  aic->out_count = 0;

	  do_init = 0;
	  
	  while(aic_phase_wait (regs, SCSI_PHASE_MSG_OUT) !=
		SCSI_PHASE_MSG_OUT)
	    /* wait */;

	  ok = aic_dosynch (aic, 0, 0);
	}
    }
  else
    ret = aic_select_target(regs, aic->sc->initiator_id,
			    tgt->target_id, FALSE);

  /* Lost arbitration? Device error? */
  if ((ret != SCSI_RET_SUCCESS || !ok) && ret != SCSI_RET_DEVICE_DOWN)
    {
      DP(1, printf("aic_atte: (ret(%x) != SCSI_RET_SUCCESS) || !ok)\n", ret));
      PC532_SCSI_SELECT(ss);

      return;
    }

  if (do_init)
    {
      aic->active_target = tgt;
      aic->script = tgt->transient_state.script;
      aic->error_handler = tgt->transient_state.handler;
      aic->done = SCSI_RET_IN_PROGRESS;

      aic->in_count = 0;
      aic->out_count = 0;
    }

  if (ret == SCSI_RET_DEVICE_DOWN)
    {
      DP (1, printf("aic_atte: device down\n"));
      aic->done = ret;
      aic_end (aic, 0, 0);
      PC532_SCSI_SELECT (ss);
      return;
    }

#if 0
    {
      /* time this out or do it via dma !! */
      while (aic_phase_wait (regs, SCSI_PHASE_CMD) != SCSI_PHASE_CMD)
	/* wait */;
    }
#endif

  PC532_SCSI_SELECT (ss);
#endif /* 0 */
}

/*
 * Interrupt routine
 *	Take interrupts from the chip
 *
 * Implementation:
 *	Move along the current command's script if
 *	all is well, invoke error handler if not.
 */

/* This is wrapped in aic_intr below
 * (for PC532 weird way of selecting SCSI chips with ICU signals, sigh)
 */
do_aic_intr (unit, old_ipl, int_regs)
     int unit;
     int old_ipl;
     struct ns532_saved_state *int_regs;
{
  register aic_softc_t	aic;
  register script_t	scp;
  register aic_controller_t	regs;
  boolean_t		try_match;
  unsigned char stat0, stat1, bus;
  
  aic  = aic_softc[unit];
  regs = aic->regs;

  LOG(5,"\n\tintr");
  TR2("-----intr-----", unit);

  /* ack interrupt */
  AIC_READ (regs, AIC_REG_STAT0, stat0);
  AIC_READ (regs, AIC_REG_STAT1, stat1);
  AIC_READ (regs, AIC_REG_BUSIN, bus);
  TR2("stat0 7", stat0);
  TR2("stat1 8", stat1);
  TR2("busin 9", bus);
  TR2("phase  ", AIC_SCSI_PHASE(bus));

  TRCHECK;

  /* we got an interrupt allright */
  if (aic->active_target)
    aic->wd.watchdog_state = SCSI_WD_ACTIVE;
  
  if (stat0 & AICrSTAT0_SCSIRST)
    {
      aic_bus_reset (aic);
      return;
    }

  if (stat1 & AICrSTAT1_SELECTOUT)
    {
      LOG(0x24,"selout");

      if (aic_complete_selection (aic, stat1) == SCSI_RET_DEVICE_DOWN)
	{
	  DP (1, printf("aic_atte: device down\n"));
	  aic->done = SCSI_RET_DEVICE_DOWN;
	  if (aic->active_target)
	    aic_end (aic, 0, 0);
	}

      AIC_CLR_INTR(regs);
      return;
    }

  AIC_CLR_INTR(regs);

#ifndef OLD_DMA
  if (aic->state & (AIC_STATE_DMA_IN | AIC_STATE_DMA_OUT))
    {
      /* yes it should be like this */
      extern int bcopy_bytes;

      /* If the interrupt came after the pseudo dma registers
       * have been set up and we are in the "movsb" instruction
       */

      aic->state |= AIC_STATE_DMA_INTR;
      aic->dma_int_regs = int_regs;
      if ((unsigned int)&bcopy_bytes == int_regs->pc)
	{
	  /* Doing DMA in/out of AIC when we had a SCSI interrupt
	   * from the selected SCSI chip.
	   */
	  TR2("dma_interrupt(1)", aic->state);
	  printf ("AIC: interrupted DMA, state: 0x%x\n", aic->state);
	}
      else
	{
	  TR2("dma_interrupt(2)", aic->state);
	  DP(1, printf ("aic: DMA interrupted before set up\n"));
	}
    }
#endif

  /* drop spurious calls (stat0 == /REQ does not count!) */
  if ((stat1 & AICrSTAT1_INTMASK) == 0)
    {
      TR2("intr is spurious", 0xbadfeed);
      LOG(0x22,"SPURIOUS");
      return;
    }

  /* Have we been selected as a target? */
  if (stat1 & AICrSTAT1_SELECTED)
    {
      LOG(0x23,"TARGET_INTR");
      aic_target_intr (aic, 0, bus);
      return;
    }

  /* How about reselected by a formerly disconnected target? */
  if (stat1 & AICrSTAT1_RESELECTED)
    {
      aic_reconnect (aic, 0, bus);
      return;
    }

  if (stat1 & AICrSTAT1_CMDDONE)
    {
#if 1
      AIC_WRITE (regs, AIC_REG_IMR1,
		 aic_default_state [ AIC_REG_IMR1 ]
		 | AICwIMR1_PHASEMISMATCH);
#endif
      stat1 &= ~AICrSTAT1_SELECTOUT;
      if ((stat1 & AICrSTAT1_INTMASK) == AICrSTAT1_CMDDONE)
	{
	  /* Don't advance script if CMD DONE is the only reason we came */
	  LOG(0x2a, "CMD_DONE(SPURIOUS)");
	  return;
	}
      else
	LOG(0x2a, "cmd_done");
    }
  if (stat1 & AICrSTAT1_ERROR)
    {
      if (stat0 & AICrSTAT0_PARITY)
	{
	  LOG(0x26, "SCSI_PARITY");
	  panic ("aic: scsi parity error");
	}

      if (stat0 & AICrSTAT0_MEMPARITY)
	{
	  LOG(0x27, "MEMORY_PARITY");
	  panic ("aic: memory parity error");
	}

      if (stat0 & AICrSTAT0_BUSFREE)
	{
	  LOG(0x28, "BUS_FREE");
	}

      if (stat0 & AICrSTAT0_PHASEMISMATCH)
	{
	  LOG(0x29, "phase_mismatch");
	}
      else if (stat0 & AICrSTAT0_PHASEATN)
	{
	  LOG (2, "PHASE/ATN");
	}
    }

  scp = aic->script;

  /* Race: disconnecting, we get the disconnected notification
     (BSY dropped) at the same time a reselect is active */
  /* @@@  CHECK STANDARD! */

  AIC_REG (regs, AIC_REG_BUSIN);
  if (AIC_VAL (regs) & AICrBUSIN_BSY == 0
      && AIC_VAL (regs) & AICrBUSIN_BSY == 0
      && AIC_VAL (regs) & AICrBUSIN_BSY == 0
      && scp
      && (scp->condition == AIC_PHASE_DISC))
    {
      (void) (*scp->action)(aic, 0, AIC_VAL (regs));
      /* takes care of calling reconnect if necessary */
      return;
    }
  
  if (! scp) {
    /* This could be interrupts due to bus activity which
     * doesn't concern us.
     */
    printf ("aic: unexpected intr, null scp.\n");
    return;
  }
  
  bus = AIC_VAL (regs);

  if (! SCRIPT_MATCH(bus, scp->condition))
    {
      DP(3, printf ("script mismatch bus %x; cond %x\n",
		    AIC_SCSI_PHASE(bus),
		    scp->condition));
      DP(1, aic_break1());
      try_match = (*aic->error_handler)(aic, 0, bus);
    }
  else
    try_match = TRUE;

  /* might have been side effected */
  scp = aic->script;
  bus = AIC_VAL (regs);

  if (try_match && (SCRIPT_MATCH(bus, scp->condition)))
    {
      DP(3, printf ("script mismatch(2) bus %x; cond %x\n",
		    AIC_SCSI_PHASE(bus),
		    scp->condition));
      DP(1, aic_break1());
      /*
       * Perform the appropriate operation, then proceed
       */
      if ((*scp->action)(aic, 0, bus))
	{
	  /* might have been side effected */
	  scp = aic->script;
	  aic->script = scp + 1;
	}
    }
}

aic_intr(unit, old_ipl, regs)
     int unit;
     int old_ipl;
     struct ns532_saved_state *regs;
{
  int ss;
  int ret;
  ss = PC532_SCSI_SELECT(ICU_AIC);
  
  ret = do_aic_intr (unit, old_ipl, regs);

  PC532_SCSI_SELECT(ss);
  return ret;
}

aic_target_intr(aic)
     aic_softc_t aic;
{
  panic("AIC: TARGET MODE !!!\n");
}

/*
 * All the many little things that the interrupt
 * routine might switch to
 */
boolean_t
aic_end_transaction (aic, csr, bs)
     aic_softc_t aic;
{
  register aic_controller_t regs = aic->regs;
  char cmc;

  LOG(0x1f,"end_t");

  aic_normal_mode (regs);

  aic_data_in (regs, SCSI_PHASE_MSG_IN, 1, &cmc);

  if (cmc != SCSI_COMMAND_COMPLETE)
    {
      TR2("bad msg in", cmc);
      printf("aic:{T%x}\n", cmc);
    }

  AIC_REG (regs, AIC_REG_BUSIN);
#if 0
  while (aic_wait (regs, AICrBUSIN_BSY, 0))
    /* Sleep until bus free */;
#endif

  /* set disconnected, clear all intr bits */
#ifdef CLEAR_INTR
  AIC_CLR_INTR (regs);
#endif
  AIC_ACK(regs, AIC_PHASE_DISC);
  
  /* aic_end never returns FALSE */
  aic_end (aic, 0, 0);

  return FALSE;
}

boolean_t
aic_end (aic, csr, bs)
     register aic_softc_t	aic;
{
  register target_info_t	*tgt;
  register io_req_t	ior;
  register aic_controller_t	regs = aic->regs;

  LOG(6,"end");

  tgt = aic->active_target;

  if ((tgt->done = aic->done) == SCSI_RET_IN_PROGRESS)
    tgt->done = SCSI_RET_SUCCESS;

  aic->script = 0;

  if (aic->wd.nactive-- == 1)
    aic->wd.watchdog_state = SCSI_WD_INACTIVE;

  aic->active_target = 0;
#if notyet /* @@@ */
  /* aic->state &= ~AIC_STATE_BUSY; later */
#else
  aic->state &= ~AIC_STATE_BUSY; /* ENABLE THIS/jtv */
#endif

  if (tgt->ior) {
    LOG(0xA,"ops->restart");
    (*tgt->dev_ops->restart)(tgt, TRUE);
  }

  return TRUE;
}

boolean_t
aic_release_bus (aic)
     register aic_softc_t	aic;
{
  boolean_t	ret = FALSE;
  
  LOG(9,"release");

  aic->script = 0;

  if (aic->state & AIC_STATE_COLLISION)
    {
      LOG(0xB,"collided");
      aic->state &= ~AIC_STATE_COLLISION;
      aic_attempt_selection(aic);
    }
  else if (queue_empty(&aic->waiting_targets))
    {
      aic->state &= ~AIC_STATE_BUSY;
      aic->active_target = 0;
      ret = TRUE;
    }
  else
    {
      LOG(0xC,"dequeue");
      aic->next_target = (target_info_t *) dequeue_head(&aic->waiting_targets);
      aic_attempt_selection(aic);
    }
  return ret;
}

boolean_t
aic_get_status (aic, csr, bs)
     aic_softc_t	aic;
{
  aic_controller_t	regs = aic->regs;
  scsi2_status_byte_t	status;
  unsigned int		len, mode;

  LOG(0xD,"get_status");
  TR2("TRWRAP;", 0);

  /* @@ Need this?? */
  AIC_WRITE (regs, AIC_REG_DMA, 0);

  aic->state &= ~AIC_STATE_DMA_IN;

  aic_data_in(regs, SCSI_PHASE_STATUS, 1, &status.bits);

  if (status.st.scsi_status_code != SCSI_ST_GOOD)
    {
      DP(1, printf("aic_get_status: not good\n"));
      scsi_error(aic->active_target, SCSI_ERR_STATUS, status.bits, 0);
      aic->done = (status.st.scsi_status_code == SCSI_ST_BUSY) ?
			SCSI_RET_RETRY : SCSI_RET_NEED_SENSE;
    }
  else
    aic->done = SCSI_RET_SUCCESS;
  
  return TRUE;
}

boolean_t
aic_issue_command(aic, cst, bs)
     aic_softc_t        aic;
{
  aic_controller_t  regs = aic->regs;

  LOG(0x12, "cmd_issue");

#if 0
  AIC_WRITE (regs, AIC_REG_IMR1,
	     aic_default_state [ AIC_REG_IMR1 ]
	     | AICwIMR1_PHASEMISMATCH);
#ifdef CLEAR_INTR
  AIC_CLR_INTR (regs);
#endif
#endif

  /* we have just done a select;
     Bus is in CMD phase;
     need to phase match */

  return aic_data_out(regs, SCSI_PHASE_CMD,
		      aic->active_target->transient_state.cmd_count,
		      aic->active_target->cmd_ptr) == 0;
}

boolean_t
aic_xfer_in (aic, cst, bs)
     register aic_softc_t    aic;
{
  register target_info_t  *tgt;
  register aic_controller_t	regs = aic->regs;
  register int            count;
  boolean_t               advance_script;

  LOG(0x2e,"xfer_in");
  
  tgt = aic->active_target;
  aic->state |= AIC_STATE_DMA_IN;

  count = tgt->transient_state.in_count;

  if ((tgt->cur_cmd != SCSI_CMD_READ)
      && (tgt->cur_cmd != SCSI_CMD_LONG_READ))
    advance_script = aic_data_in(regs, SCSI_PHASE_DATAI, count, tgt->cmd_ptr) == 0;
  else
    {
#ifdef AIC_DMA_IN
      advance_script = aic_dma_in (aic, SCSI_PHASE_DATAI) == 0;
#else
      advance_script = aic_data_in(regs, SCSI_PHASE_DATAI,
				   count, tgt->ior->io_data) == 0;
#endif
    }

  return advance_script;
}

boolean_t
aic_xfer_out (aic, cst, bs)
        register aic_softc_t    aic;
{
  register aic_controller_t  regs = aic->regs;
  register target_info_t  *tgt;
  boolean_t               advance_script;
  int                     count = aic->out_count;

  LOG(0x2f,"xfer_out");

  tgt = aic->active_target;
  aic->state &= ~AIC_STATE_DMA_IN;

  count = tgt->transient_state.out_count;

  if ((tgt->cur_cmd != SCSI_CMD_WRITE) &&
      (tgt->cur_cmd != SCSI_CMD_LONG_WRITE))
    advance_script = aic_data_out
      			(regs, SCSI_PHASE_DATAO, count,
			 tgt->cmd_ptr + tgt->transient_state.cmd_count) == 0;
  else
    {
#ifdef AIC_DMA_OUT
      advance_script = aic_dma_out (aic, SCSI_PHASE_DATAO);
#else
      advance_script = aic_data_out (regs, SCSI_PHASE_DATAO,
				     count, tgt->ior->io_data) == 0;
#endif
    }

  return advance_script;
}

/* disconnect-reconnect ops */

/* get the message in via dma ?? */
boolean_t
aic_msg_in(aic, csr, bs)
     register aic_softc_t    aic;
{
  register target_info_t  *tgt;
  register aic_controller_t regs = aic->regs;

  LOG(0x15,"msg_in");
  gimmeabreak();
  
  /* ??? @@@ ??? */

  return TRUE;
}

/* send data to target. Called in three different ways:
   (a) to start transfer (b) to restart a bigger-than-8k
   transfer (c) after reconnection
 */

/* @@@ No support for sync mode yet */
boolean_t
aic_dma_out (aic, phase)
     register aic_softc_t aic;
     int phase;
{
  aic_controller_t	regs = aic->regs;

  char		*dma_ptr;
  target_info_t	*tgt;
  boolean_t      advance_script = TRUE;
  int            count = aic->out_count;
  int	 s;
  boolean_t continues = FALSE;
  unsigned char stat0, stat1, imr;

  LOG(0xF,"dma_out");

  tgt = aic->active_target;
  aic->state &= ~AIC_STATE_DMA_IN;

  /* ought to stop dma to start another */
  AIC_DEFAULT (regs, AIC_REG_DMA);

  if (aic->out_count == 0)
    {
      /*
       * Nothing committed: either just sent the
       * command or reconnected
       */
      register int remains;

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

      /* Calculate number of bytes already transferred over SCSI */
      AIC_READ_DMA_COUNT (regs, xferred);

      if (xferred)
	printf("{A %x}", xferred);

      xferred = aic->out_count - xferred;
      assert (xferred > 0);

      tgt->transient_state.out_count -= xferred;
      assert (tgt->transient_state.out_count > 0);

      offset = tgt->transient_state.dma_offset;
      tgt->transient_state.dma_offset += xferred;
      count = u_min (tgt->transient_state.out_count, PER_TGT_BURST_SIZE);

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

      TR2("dma out(cont) count",count);

      /* last chunk ? */
      if (tgt->transient_state.out_count == count)
	goto quickie;

#ifdef PC532      
      /* @@@@@@@@@@@@@@: First copies BLOCKSIZE, then the
	 rest :-) Should adjust the chained copies to the
	 same size than the first copy
       */
      /* No need to copy data for scsi pseudo dma */
      continues = TRUE;
      advance_script = FALSE;
#else
      /* ship some more */
      dma_ptr = tgt->ior->io_data + tgt->transient_state.dma_offset;

      aic->out_count = count;

      DO DMA HERE

      /* copy some more data */
      aic_copyout(tgt, offset, xferred);
      return FALSE;
#endif
    }

 quickie:
  {
    aic->out_count = count;
    dma_ptr = tgt->ior->io_data + tgt->transient_state.dma_offset;

    AIC_WRITE (regs, AIC_REG_OFFSET, 0);

    AIC_ACK(regs, phase);

    AIC_WRITE_DMA_COUNT (regs, count);

    AIC_WRITE (regs, AIC_REG_DMA, AICwDMA_ENABLE | AICwDMA_DIR_OUT);

    delay(1);

    DP(1, aic_break3());

    /* Max size of DMA to scsi: 4 MBytes, then you hit DMAEOP address */
#ifdef OLD_DMA
    bcopy (dma_ptr, pc532_dma, count);
#else
    bcopy_bytes (dma_ptr, pc532_dma, count);
#endif
  
    DP(1, aic_break4());

    AIC_READ (regs, AIC_REG_STAT0, stat0);

    TR2("dma_write stat0", stat0);
    if (!(stat0 & AICrSTAT0_DMAZERO))
      printf ("aic: DMA counter not zero after dma write\n");

    AIC_READ (regs, AIC_REG_STAT1, stat1);

    if (stat1 & AICrSTAT1_CMDDONE == 0)
      {
	printf ("aic: DONE not set after DMA stat0: 0x%x stat1: 0x%x\n",
		stat0, stat1);
      }
    
    /* Disable DMA */
    AIC_WRITE (regs, AIC_REG_DMA, 0);
  }

  if (continues)
    {
      /* @@@ Hmm, this is propably incorrect now ... */
      tgt->transient_state.out_count  -= count;
      tgt->transient_state.dma_offset += count;
    }

#ifdef CLEAR_INTR
  AIC_CLR_INTR (regs);/* @@@ Check why it does not work if this is removed */
#endif

  return advance_script;
}

boolean_t
aic_dma_in (aic, phase)
	register aic_softc_t aic;
	int phase;
{
  register target_info_t	*tgt;
  register aic_controller_t	regs = aic->regs;
  char			*dma_ptr;
  register int		count;
  boolean_t		advance_script = TRUE;
  boolean_t continues = FALSE;
  unsigned char stat0, stat1, imr;

  LOG(0xE,"dma_in");

  tgt = aic->active_target;
  aic->state |= AIC_STATE_DMA_IN;

  /* ought to stop dma to start another */
  AIC_DEFAULT (regs, AIC_REG_DMA);

  if (aic->in_count == 0)
    {
      /*
       * Got nothing yet: either just sent the command
       * or just reconnected
       */
      register int avail;
      
      count = tgt->transient_state.in_count;
      count = u_min (count, PER_TGT_BURST_SIZE);
      
#if 0
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
#if 0
	  printf("{INA %d}\n", count);
	  printf ("{INA: ior %x alloc %d request %d addr %x \n",
		  ior, ior->io_alloc_size, ior->io_count, ior->io_data);
#endif
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
      
      AIC_READ_DMA_COUNT (regs, xferred);
      assert(xferred == 0);
      
      if (scsi_debug)
	{
	  printf("{INB %x %x}", aic->in_count, xferred);
	}

      xferred = aic->in_count - xferred;
      assert(xferred > 0);

      tgt->transient_state.in_count -= xferred;
      assert(tgt->transient_state.in_count > 0);

      offset = tgt->transient_state.dma_offset;
      tgt->transient_state.dma_offset += xferred;
      count = u_min(tgt->transient_state.in_count, (PER_TGT_BURST_SIZE));

      if (tgt->transient_state.dma_offset == PER_TGT_BUFF_SIZE)
	{
	  tgt->transient_state.dma_offset = 0;
	}
      else
	{
	  register int avail;
	  avail = PER_TGT_BUFF_SIZE - tgt->transient_state.dma_offset;
	  count = u_min(count, avail);
	}
      advance_script = (tgt->transient_state.in_count == count);

#ifdef PC532
      continues = !advance_script;
      
      TR2("dma in(cont) count",count);
#else
      /* get some more */

      dmar->aic_dma_dir = AIC_DMA_DIR_READ;
      regs->aic_irecv = 1;
      
      Hmm...

      /* copy what we got */
      aic_copyin( tgt, offset, xferred, eb, extrab);

#endif
    }

  aic->in_count = count;
  dma_ptr = tgt->ior->io_data + tgt->transient_state.dma_offset;
  
  AIC_WRITE (regs, AIC_REG_OFFSET, 0);

  AIC_ACK(regs, phase);
    
  AIC_WRITE_DMA_COUNT (regs, count);

  AIC_WRITE (regs, AIC_REG_DMA, AICwDMA_ENABLE);

  delay(10);

  DP(1, aic_break3());

#ifdef OLD_DMA
  {
    /* Protect against prefetches in NS32532 */
    int s = splhi ();
    /* Max size of DMA to scsi: 4 MBytes, then you hit DMAEOP address */
    bcopy (pc532_dma, dma_ptr, count);
    splx (s);
  }
#else
  bcopy_bytes (pc532_dma, dma_ptr, count);
#endif
  
  DP(1, aic_break4());

  AIC_READ (regs, AIC_REG_STAT0, stat0);
  
  TR2("dma_in stat0", stat0);
  if (!(stat0 & AICrSTAT0_DMAZERO))
    printf ("aic: DMA counter not zero after dma read\n");
  
  AIC_READ (regs, AIC_REG_STAT1, stat1);
  
  if (stat1 & AICrSTAT1_CMDDONE == 0)
    {
      printf ("aic: DONE not set after DMA in stat0: 0x%x stat1: 0x%x\n",
		stat0, stat1);
    }
  
  /* Disable DMA */
  AIC_WRITE (regs, AIC_REG_DMA, 0);

  if (continues)
    {
      /* @@@@ Hmm, this is probably incorrect */
      tgt->transient_state.in_count   -= count;
      tgt->transient_state.dma_offset += count;
    }
  
#ifdef CLEAR_INTR
  AIC_CLR_INTR (regs);/* @@@ Check why it does not work if this is removed */
#endif
  
  return advance_script;
}

/* check the message is indeed a DISCONNECT */
boolean_t
aic_disconnect (aic, csr, bs)
     register aic_softc_t aic;
{
  register int  len;
  boolean_t	ok = FALSE;
  char		*msgs;
  unsigned int	 offset;
  char          *dma_ptr;
  aic_controller_t regs = aic->regs;
  target_info_t   *tgt  = aic->active_target;

  if (! tgt)
    panic ("aic: No active target when disconnected");

  AIC_READ_DMA_COUNT (regs, len);
  len = sizeof (scsi_command_group_0) - len;
  DP (1, printf ("{G%d}", len));

#ifdef PC532
  panic ("aic_disconnect");
  /* need to know where we wrote the last 2 bytes */
#else
  /* wherever it was, take it from there */
  AIC_DMADR_GET (dmar,offset);
  msgs = (char*)aic->buff + offset - len;
#endif

  if (len == 1 || len == 2)
    /* A message preceeds it in non-completed READs */
    ok = ((msgs[0] == SCSI_DISCONNECT) ||	  /* completed op */
	  ((msgs[0] == SCSI_SAVE_DATA_POINTER) && /* incomplete */
	   (msgs[1] == SCSI_DISCONNECT)));

  if (!ok)
    printf("[tgt %d bad msg (%d): %x]",
	   aic->active_target->target_id, len, *msgs);
  
  return TRUE;
}

/* save all relevant data, free the BUS */
boolean_t
aic_disconnected (aic, csr, bs)
     register aic_softc_t aic;
{
  panic("aic_disconnected");
}

/* get reconnect message, restore BUS */
boolean_t
aic_reconnect(aic, csr, bs)
     register aic_softc_t	aic;
{
  int id;
  aic_controller_t	regs;
  regs = aic->regs;

  /* Check if reconnect was for us */
  AIC_READ (regs, AIC_REG_SOURCE_DEST, id);

  printf("aic: initiator_id = 0x%x, id = 0x%x\n", aic->sc->initiator_id, id);

  if( (id & (1 << aic->sc->initiator_id)) == 0)
    return FALSE;

  panic("aic_reconnect");
}

/* do the synch negotiation */
/* Does not work yet. This is not called if aic_sync == 0 */
boolean_t
aic_dosynch (aic, csr, bus)
     aic_softc_t aic;
{
  /*
   * Phase is MSG_OUT here, cmd has not been xferred
   */
  register target_info_t *tgt;
  register aic_controller_t regs = aic->regs;
  register unsigned char *p;
  unsigned char off, icmd;
  int len;

  /* If target drops BSY cause an interrupt */
  AIC_WRITE (regs, AIC_REG_IMR1,
	     aic_default_state[AIC_REG_IMR1] | AICwIMR1_BUSFREE);

  LOG(0x11,"dosync");

  /* ATN still asserted */
	
  tgt = aic->active_target;

  tgt->flags |= TGT_DID_SYNCH;	/* only one chance */
  tgt->flags &= ~TGT_TRY_SYNCH;

  p = (unsigned char *)tgt->cmd_ptr + tgt->transient_state.cmd_count +
    	tgt->transient_state.dma_offset;

  p[0] = SCSI_IDENTIFY; /* @@@@ */

  p[1] = SCSI_EXTENDED_MESSAGE;
  p[2] = 3;
  p[3] = SCSI_SYNC_XFER_REQUEST;
  p[4] = aic_to_scsi_period (aic_min_period);

  if (BGET(scsi_no_synchronous_xfer,aic->sc->masterno,tgt->target_id))
    {
      p[5] = 0;
      DP(1, printf("Only async transfer for target %d\n", tgt->target_id));
    }
  else
    p[5] = (AIC_GET_STATE (regs, AIC_REG_CNTRL0) & AICwCNTRL0_TARGET) ?
      		AIC_TARGET_SYNC_MAX : AIC_INITIATOR_SYNC_MAX;

  DP(1, printf("aic: sync negotiation stuff at addr 0x%x\n", p));
#if MACH_KDB
  DP(3, aic_state(0));
#endif

  /* xfer all but last byte with ATN set */
  len = aic_data_out(regs, SCSI_PHASE_MSG_OUT | AICwBUSOUT_ATN,
		     sizeof(scsi_synch_xfer_req_t), p);

  if (! len)
    {
      /* Wait for /REQ before dropping ATN */
      AIC_REG (regs, AIC_REG_BUSIN);
      aic_wait (regs, AICrBUSIN_REQ, AICrBUSIN_REQ);
      
      /* reset ATN */
      AIC_DEFAULT (regs, AIC_REG_IMR);

      /* p[0] is the identify byte ! */
      len = aic_data_out(regs, SCSI_PHASE_MSG_OUT,
			 1, &p[sizeof(scsi_synch_xfer_req_t)]);
    }
  else
    AIC_DEFAULT (regs, AIC_REG_IMR);

  while (aic_phase_wait (regs, SCSI_PHASE_MSG_IN) != SCSI_PHASE_MSG_IN)
    /* wait */;

  /* The standard sez there nothing else the target can do but.. */
  if (AIC_SCSI_PHASE_FETCH(regs) != SCSI_PHASE_MSG_IN)
    panic("aic_dosync MSG_IN phase expected");/* XXX put offline */

  /* get answer */
  len = sizeof(scsi_synch_xfer_req_t);
  len = aic_data_in (regs, SCSI_PHASE_MSG_IN, len, p);

  /* do not cancel the phase mismatch interrupt ! */
  
  /* look at the answer and see if we like it */
  if (p[0] == SCSI_MESSAGE_REJECT)
    {
      /* did not like the question */
      DP(1, printf(" did not like SYNCH xfer len %d\n", len));
      tgt->sync_offset = 0;
    }
  else
    {
      if (p[0] != SCSI_EXTENDED_MESSAGE)
	{
	  DP(1, printf("aic: Incorrect answer to sync negotiation 0x%x\n",
		       p[0]));
	  gimmeabreak ();
	}
      
      /* synch negotiation OK */
      tgt->sync_period = scsi_period_to_aic(p[3]);
      tgt->sync_offset = p[4];
      
      /* sanity */
      if (tgt->sync_offset != 0)
	{
	  DP(1, printf("Do SYNCH xfer (rate %d ns/byte) offset %d\n",
		       4*p[3], tgt->sync_offset));
	}
    }

  aic_break5();

  AIC_REG (regs, AIC_REG_BUSIN);

  { int c;
    /* Might have to wait a bit longer for slow targets */
    for (c = 0; AIC_SCSI_PHASE_FETCH(regs) == SCSI_PHASE_MSG_IN; c++) {
      delay(2);
      if (c & 0x80) break;  /* waited too long */
    }
  }

  /* Clear the bus free interrupt XXX Check this? */
  AIC_DEFAULT(regs, AIC_REG_IMR1);

  AIC_REG (regs, AIC_REG_BUSIN);
  if (AIC_SCSI_PHASE_FETCH (regs) == SCSI_PHASE_CMD)
    {
      /* test unit ready or inquiry */

      aic_break5();
      if (tgt->cur_cmd == SCSI_CMD_INQUIRY) {
	tgt->transient_state.script = aic_script_data_in;
	aic->script = tgt->transient_state.script;
	AIC_WRITE (regs, AIC_REG_OFFSET,
		     (tgt->sync_offset&AIC_OFFSET_MASK)
		   | ((tgt->sync_period<<4)&AIC_SYNCRATE_MASK));

	return TRUE;
      }

      DP(1, printf("aic: send test unit ready\n"));
      len = aic_data_out (regs, SCSI_PHASE_CMD,
			  sizeof(scsi_command_group_0), tgt->cmd_ptr);
    }

  aic_break5();
  
  AIC_REG (regs, AIC_REG_BUSIN);

  DP(1, printf("aic: dosynch phase %d is not CMD\n",
	       AIC_SCSI_PHASE_FETCH(regs)));
  
  aic_phase_wait (regs, SCSI_PHASE_STATUS);

  if (AIC_SCSI_PHASE_FETCH(regs) != SCSI_PHASE_STATUS)
    gimmeabreak();

  return TRUE; /* intr is pending */
}

/*
 * The bus was reset
 */
aic_bus_reset(aic)
     aic_softc_t aic;
{
  target_info_t	*tgt;
  aic_controller_t	regs = aic->regs;
  int i;

  LOG(0x21,"bus_reset");

  /*
   * Clear bus descriptor
   */
  aic->script = 0;
  aic->error_handler = 0;
  aic->active_target = 0;
  aic->next_target = 0;
  aic->state = 0;
  queue_init(&aic->waiting_targets);
  aic->wd.nactive = 0;
  aic_reset(aic, TRUE);

  printf("aic%d: (%d) bus reset ", aic->sc->masterno, ++aic->wd.reset_count);
  delay(scsi_delay_after_reset); /* some targets take long to reset */
  
  if (aic->sc == 0)	/* sanity */
    return;

  scsi_bus_was_reset(aic->sc);
}

/*
 * Error handlers
 */

/*
 * Generic, default handler
 */

boolean_t
aic_err_generic(aic, csr, bs)
     aic_softc_t	aic;
{
  int cond = aic->script->condition;

  LOG(0x10,"err_generic");
  TR2("err_generic expected phase(+bsy)", cond);

  AIC_REG (aic->regs, AIC_REG_BUSIN);
  if (AIC_SCSI_PHASE_FETCH(aic->regs) == SCSI_PHASE_STATUS)
    return aic_err_to_status(aic, csr, bs);

  gimmeabreak();
  return FALSE;
}

/*
 * Handle generic errors that are reported as
 * an unexpected change to STATUS phase
 */
aic_err_to_status(aic, csr, bs)
	register aic_softc_t	aic;
{
	script_t		scp = aic->script;

	LOG(0x20,"err_tostatus");
	while (SCSI_PHASE(scp->condition) != SCSI_PHASE_STATUS)
		scp++;
	aic->script = scp;
#if 0
	/*
	 * Normally, we would already be able to say the command
	 * is in error, e.g. the tape had a filemark or something.
	 * But in case we do disconnected mode WRITEs, it is quite
	 * common that the following happens:
	 *	dma_out -> disconnect -> reconnect
	 * and our script might expect at this point that the dma
	 * had to be restarted (it didn't know it was completed
	 * because the tape record is shorter than we asked for).
	 * And in any event.. it is both correct and cleaner to
	 * declare error iff the STATUS byte says so.
	 */
	aic->done = SCSI_RET_NEED_SENSE;
#endif
	return TRUE;
}

/*
 * Watch for a disconnection
 */
boolean_t
aic_err_disconn(aic, csr, bs)
     register aic_softc_t aic;
{
  panic("aic_err_disconn");
}

/*
 * Watchdog
 */
aic_reset_scsibus(aic)
     register aic_softc_t    aic;
{
  target_info_t  *tgt = aic->active_target;
  aic_controller_t regs = aic->regs;
  unsigned char control1 = regs->state[ AIC_REG_CONTROL1 ];
  int s, ss;

  TR2("aic: watchdog reset", 0);
  s = splhi ();
  ss = PC532_SCSI_SELECT (ICU_AIC);
  if (tgt)
    {
      int cnt;
      AIC_REG (regs, AIC_REG_DMA_L);
      cnt += AIC_VAL(regs);
      cnt += AIC_VAL(regs)<<8;
      cnt += AIC_VAL(regs)<<16;
      log(LOG_KERN,
	  "Target %d was active, cmd x%x in x%x out x%x Sin x%x Sou x%x dmalen x%x\n",
	  tgt->target_id, tgt->cur_cmd,
	  tgt->transient_state.in_count, tgt->transient_state.out_count,
	  aic->in_count, aic->out_count, cnt);
    }
  AIC_WRITE (regs, AIC_REG_CONTROL1, control1 | AICwCONTROL1_SCSIRESET);
  delay(250);
  AIC_WRITE (regs, AIC_REG_CONTROL1, control1);

  delay (scsi_delay_after_reset); /* some targets take long to reset */

  PC532_SCSI_SELECT(ss);
  splx(s);
}

/*
 * Copy routines
 */
/*static*/
aic_copyin(tgt, offset, len, isaobb, obb)
     register target_info_t	*tgt;
     unsigned char obb;
{
  register char	*from, *to;
  register int	count;

#ifdef PC532
  printf("unwanted aic_copyin\n");
  return;
#endif PC532
  
  count = tgt->transient_state.copy_count;

  from = tgt->cmd_ptr + offset;
  to = tgt->ior->io_data + count;
  tgt->transient_state.copy_count = count + len;

  bcopy(from, to, len);

  /* check for last, poor little odd byte */
  if (isaobb) {
    to += len;
    to[-1] = obb;
  }
}

/*static*/
aic_copyout (tgt, offset, len)
     register target_info_t	*tgt;
{
  register char	*from, *to;
  register int	count, olen;
  unsigned char	c;
  char		*p;
  
#ifdef PC532
  printf("unwanted aic_copyout\n");  
  return;
#endif PC532

  count = tgt->ior->io_count - tgt->transient_state.copy_count;

  if (count > 0)
    {
      len = u_min (count, len);
      offset += tgt->transient_state.cmd_count;

      count = tgt->transient_state.copy_count;
      tgt->transient_state.copy_count = count + len;

      from = tgt->ior->io_data + count;
      to = tgt->cmd_ptr + offset;

      bcopy(from, to, len);
    }
}

/* The following routine is for Kivinen's
 * hex output led panel.
 * They are not used in the scsi driver.
 * It does not work yet.
 */
aic_str_to_reg (reg, length, str)
     int reg;
     int length;
     unsigned char *str;
{
  aic_controller_t regs;
  int saved_reg;
  int i;
  int s;
  int ss;

#if 1
  return; /* I told you :-) */
#endif

  if (aic_initialized < 2)
    return;

  if (! AIC_REG_VALID(reg))
    {
      printf ("aic: (str_to_reg) Invalid register %d\n", reg);
      return;
    }

  s = splhi ();

  regs = &aic_device [ aic_unit ];
  saved_reg = regs->reg;
  
  ss = PC532_SCSI_SELECT (ICU_AIC);
  if (reg > 7)
    {
      AIC_REG (regs, reg);
      for (i = 0; i < length; i++)
	AIC_STORE (regs, str[ i ]);
    }
  else
    for (i = 0; i < length; i++)
      AIC_WRITE (regs, reg, str[ i ]);

  AIC_REG (regs, saved_reg);
  PC532_SCSI_SELECT (ss);
  splx (s);
}
#endif	/*NAIC > 0*/

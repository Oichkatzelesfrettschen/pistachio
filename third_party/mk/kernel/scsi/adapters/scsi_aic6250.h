/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS 
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
 * any improvements or extensions that they make and grant Carnegie the
 * rights to redistribute these changes.
 */
/*
 *	File: scsi_aic6250.h
 * 	Author: Jukka Virtanen, Helsinki University of Technology
 *      Email:  jtv@hut.fi
 *	Date:	4/92
 *
 *	Defines for the AIC 6250 (SCSI chip)
 */

#ifndef _SCSI_AIC_6250_H_
#define _SCSI_AIC_6250_H_
/*
 * In AIC the registers are selected by first writing a index
 * to the address register and then accessing the register.
 * (PC532 has tied pin 25 (mode) to GND)
 *
 * The register addresses in the range 0-7 (inclusive) are
 * autoincremented after access. Addresses 8-15 are not incremented.
 */

	/* Auto-incrementing register numbers */
#define AIC_REG_DMA_L		0x0
#define AIC_REG_DMA_M		0x1
#define AIC_REG_DMA_H		0x2
#define AIC_REG_IMR		0x3
#define AIC_REG_OFFSET		0x4
#define AIC_REG_FIFO		0x5	/* Read  5 */
#define AIC_REG_DMA		0x5	/* Write 5 */
#define AIC_REG_REV		0x6	/* Read  6 */
#define AIC_REG_IMR1		0x6	/* Write 6 */
#define AIC_REG_STAT0		0x7	/* Read  7 */
#define AIC_REG_CNTRL0		0x7	/* Write 7 */

	/* Non auto-incrementing register numbers */
#define AIC_REG_STAT1		0x8	/* Read  8 */
#define AIC_REG_CONTROL1	0x8	/* Write 8 */
#define AIC_REG_BUSIN		0x9	/* Read  9 */
#define AIC_REG_BUSOUT		0x9	/* Write 9 */
#define AIC_REG_SCSI_ID_DATA	0xa
#define AIC_REG_SOURCE_DEST	0xb
#define AIC_REG_MEMORY_DATA	0xc
#define AIC_REG_PORT_A		0xd
#define AIC_REG_PORT_B		0xe
#define AIC_REG_SCSI_LATCH	0xf	/* Read  0xf */
#define AIC_REG_BSY_RST		0xf	/* Write 0xf */

/* Definitions of values within registers */

/* AIC_REG_DMA_L(0)
 * AIC_REG_DMA_M(1)
 * AIC_REG_DMA_H(2)
 */
/*
 * DMA count registers:
 *
 * These are three 8 bit registers combined to a 24 bit down counter.
 * Allows transfers of 16 MBytes in one dma action
 *
 * Least significant bit is AIC_REG_DMA_L bit 0
 * Middle register is 	    AIC_REG_DMA_M
 * Most significant bit is  AIC_REG_DMA_H bit 7
 */

/* AIC_REG_IMR(3) : Interrupt mask register (Write only) */
#define AICwIMR_ENASEL		0x01	/* Enable Select */
#define AICwIMR_ENARESEL	0x02	/* Enable Reselect */
#define AICwIMR_ENASELOUT	0x04	/* Enable Select out */
#define AICwIMR_ENACMDDONE	0x08	/* Enable Command done */
#define AICwIMR_ENAERROR	0x10	/* Enable Error */
#define AICwIMR_ENAAUTOATN	0x20	/* Enable Auto ATN */
#define AICwIMR_ARBSEL		0x40	/* ARB/SEL start */

/* AIC_REG_OFFSET(4) : Offset control (Write only) */
#define AICwOFFSET_OFF0		0x01	/* Offset bit 0 */
#define AICwOFFSET_OFF1		0x02	/* Offset bit 1 */
#define AICwOFFSET_OFF2		0x04	/* Offset bit 2 */
#define AICwOFFSET_OFF3		0x08	/* Offset bit 3 */
#define AICwOFFSET_SYNCRATE0	0x10	/* Sync xfer rate bit 0 */
#define AICwOFFSET_SYNCRATE1	0x20	/* Sync xfer rate bit 1 */
#define AICwOFFSET_SYNCRATE2	0x40	/* Sync xfer rate bit 2 */

#define AIC_OFFSET_MASK		0x07
#define AIC_SYNCRATE_MASK	0x70

/* AIC_REG_FIFO(5) : Fifo status register (read) */
#define AICrFIFO_COUNTER0	0x01	/* FIFO counter bit 0 */
#define AICrFIFO_COUNTER1	0x02	/* FIFO counter bit 1 */
#define AICrFIFO_COUNTER2	0x04	/* FIFO counter bit 2 */
#define AICrFIFO_FULL		0x08	/* FULL condition */
#define AICrFIFO_EMPTY		0x10	/* EMPTY condition */
#define AICrFIFO_COUNTZERO	0x20	/* Offset Count zero */
#define AICrFIFO_TEST1		0x40	/* Test signal */
#define AICrFIFO_TEST2		0x80	/* Test signal */

#define AIC_FIFO_COUNTER_MASK 	0x07

/* AIC_REG_DMA(5) : DMA control register (write) */
#define AICwDMA_ENABLE		0x01	/* DMA xfer enable */
#define AICwDMA_DIR_OUT		0x02	/* Transfer direction to scsi */
#define AICwDMA_ODDSTART	0x04	/* Odd xfer start */

/* AIC_REG_REV(6) : Revision control (read) */
#define AICrREV_V0		0x01	/* Revision bit 0 */
#define AICrREV_V1		0x02	/* Revision bit 1 */

#define AIC_REVISION_MASK	0x03

/* AIC_REG_IMR1(6)	: Interrupt mask 1 (write) */
#define AICwIMR1_ATN		0x01	/* Enable ATN on int   (target) */
#define AICwIMR1_PHASECHANGE	0x01	/* Enable phase change (initiator) */
#define AICwIMR1_PARITY		0x02	/* Enable parity */
#define AICwIMR1_BUSFREE	0x04	/* Enable bus free */
#define AICwIMR1_PHASEMISMATCH	0x08	/* Enable phase mismatch */
#define AICwIMR1_MEMPARITY	0x10	/* Enable memory parity error*/
#define AICwIMR1_SCSIRST	0x20	/* Enable SCSI reset */
#define AICwIMR1_SCSIREQ	0x40	/* Enable SCSI /REQ */

/* AIC_REG_STAT0(7)    : Status register 0 (read) */
#define AICrSTAT0_DMAZERO	0x01	/* DMA count zero */
#define AICrSTAT0_PHASEATN	0x02	/* SCSI phase change or ATN */
#define AICrSTAT0_REQ		0x04	/* SCSI /REQ active */
#define AICrSTAT0_PARITY	0x08	/* SCSI parity error */
#define AICrSTAT0_BUSFREE	0x10	/* SCSI bus free */
#define AICrSTAT0_PHASEMISMATCH	0x20	/* SCSI phase mismatch */
#define AICrSTAT0_MEMPARITY	0x40	/* Memory parity error */
#define AICrSTAT0_SCSIRST	0x80	/* SCSI reset occurred (latched) */

/* AIC_REG_CNTRL0(7)	: Control register 0 (write) */
#define AICwCNTRL0_ID0		0x01	/* SCSI id bit 0 */
#define AICwCNTRL0_ID1		0x02	/* SCSI id bit 1 */
#define AICwCNTRL0_ID2		0x04	/* SCSI is bit 2 */
#define AICwCNTRL0_DIFFMODE	0x08	/* Single end/differential mode */
#define AICwCNTRL0_OUTPORTA	0x10	/* PORT A input(0)/output(1) */
#define AICwCNTRL0_TARGET	0x20	/* Target mode */
#define AICwCNTRL0_PROCMEMWRITE	0x40	/* Processor memory read/write */
#define AICwCNTRL0_PROCMEMREQ	0x80	/* Processor memory cycle request */

#define AICwCNTRL0_IDMASK	7

	/* Non auto-incrementing register numbers */
/* AIC_REG_STAT1(8) : Status register 1 */
#define AICrSTAT1_SELECTED	0x01	/* We have been selected as target */
#define AICrSTAT1_RESELECTED	0x02	/* Reselection occurred */
#define AICrSTAT1_SELECTOUT	0x04	/* AIC is driving /SEL (setup timer) */
#define AICrSTAT1_CMDDONE	0x08	/* Command completed :	             */
					/* 1) DMA 2) Automatic PIO 3) ARBSEL */
#define AICrSTAT1_ERROR		0x10	/* Special error handling required */
#define AICrSTAT1_SCSIRESETIN	0x20	/* SCSI reset has occurred:floating */
#define AICrSTAT1_RESERVED	0x40	/* Always 1 */
#define AICrSTAT1_MEMCYCLEDONE	0x80	/* Memory cycle has completed */

/* make SELOUT a spurious interrupt */
#define AICrSTAT1_INTMASK	0x1b	/* Bit mask for interrupt bits */

/* AIC_REG_CONTROL1(8) : Control register 1 */
#define AICwCONTROL1_SOFTRESET	0x01	/* Request software reset */
#define AICwCONTROL1_SCSIRESET	0x02	/* Request SCSI reset */
#define AICwCONTROL1_CLOCKFREQ	0x04	/* Clock frequency (pc532: 20 MHz) */
#define AICwCONTROL1_PHASECHANGE 0x08	/* Intr on any SCSI phase change */
#define AICwCONTROL1_ENAPORTB	0x10	/* Enable PORT B for input/output */
#define AICwCONTROL1_RESERVED	0x20	/* Reserved */
#define AICwCONTROL1_16BITBUS	0x40	/* Enable 16 bit bus */
#define AICwCONTROL1_AUTOPIO	0x80	/* Automatic SCSI PIO request  */

/* AIC_REG_BUSIN(9) : SCSI signals read via register */
#define AICrBUSIN_ACK		0x01
#define AICrBUSIN_REQ		0x02
#define AICrBUSIN_BSY		0x04
#define AICrBUSIN_SEL		0x08
#define AICrBUSIN_ATN		0x10
#define AICrBUSIN_MSG		0x20
#define AICrBUSIN_IO		0x40
#define AICrBUSIN_CD		0x80

/* The SCRIPT entries require the BSY signal with SCSI PHASE. Map it here. */
#define AIC_BUS_BSY		0x80
#define AIC_PHASE_MASK		0xe0

/* AIC_REG_BUSOUT(9) : SCSI signal write via register   */
/* ACK/REQ is selected by INITIATOR/TARGET mode respectively */
#define AICwBUSOUT_ACK		0x02	/* ACK out (value same as REQ) */
#define AICwBUSOUT_REQ		0x02	/* REQ out (value same as ACK) */
#define AICwBUSOUT_BSY		0x04
#define AICwBUSOUT_SEL		0x08
#define AICwBUSOUT_ATN		0x10
#define AICwBUSOUT_MSG		0x20
#define AICwBUSOUT_IO		0x40
#define AICwBUSOUT_CD		0x80

/* AIC_REG_SCSI_ID_DATA(0xA) : SCSI ID/DATA */
/*
 * Before arbitration: Store both Initiator and Target ID bits here.
 *
 * After selection/reselection : Use this reg to read/write data from/to
 * the SCSI bus. (Writes are latched, reads are not)
 */

/* AIC_REG_SOURCE_DEST(0xB) : Source and destination (read only) */
/*
 * After selection/reselection this register contains the source and
 * destination ID's. Read this after SELECTION(target) or
 * RESELECTION(initiator).
 */

/* AIC_REG_MEMORY_DATA(0xC) : Memory data */
/*
 * Read/write memory data through AIC.
 *
 * Write: Store data in this register,
 *        set AICwCNTRL0_PROCMEMREQ and AICwCNTRL0_PROCMEMWRITE
 * Read:  set AICwCNTRL0_PROCMEMREQ and clear AICwCNTRL0_PROCMEMWRITE
 */

/* AIC_REG_PORT_A(0xD) : PORT A read/write */
/*
 * Read: Read data from PORT A regardless of the setup.
 * Write (8 bit, single ended mode) : Write and latch data to port A
 */

/* AIC_REG_PORT_B(0xE) : */
/*
 * Read : Read data from PORT B regardless of the setup
 * Write (8 bit, single ended mode) : Write and latch data to port B
 */

/* AIC_REG_SCSI_LATCH(0xF) : SCSI LATCH DATA */
/*
 * Read data latched during the SCSI REQ/ACK cycle.
 *  Target mode: data clocked with ACK.
 *  Initiator mode: data clocked with REQ.
 *
 * NOTE: Initiator should read this before beginning of next handshake
 *       to prevent data overrun.
 */

/* AIC_REG_BSY_RST(0xF) : SCSI BSY RESET (target) */
/*
 * Target mode: any write to this register will reset the SCSI BSY signal
 * and the SCSI bus will enter BUS FREE phase
 */

/*
 * Additional definitions
 */

#define AIC_REG_VALID(X) ((X) >= 0 && (X) <= 15)

/*
 * FIFO depth
 */
#define AIC_INITIATOR_SYNC_MAX	7
#define AIC_TARGET_SYNC_MAX	8

/*
 * Synchronous xfer register
 */
#define	AIC_SYN_OFF_MASK	0x0f
/* Don't tell anyone, but I just guessed this might be the correct number */
#define	AIC_SYN_MIN_PERIOD	4

/* Set up the fastest possible sync rate bits (0..7) */
#define AIC_DEFAULT_SYNC_RATE	0

/* Operating frequency of AIC controller, in MHz */
#define AIC_CLOCK_FREQUENCY	20

/* Map the SCSI signals to SCSI PHASE. Sigh, out of order, of course. */
#define	AIC_SCSI_PHASE(x)  SCSI_PHASE(((x)>>6) | ((!!((x)&AICrBUSIN_MSG))<<2))

#define AIC_SCSI_PHASE_FETCH(regs) AIC_SCSI_PHASE(AIC_VAL(regs))

#define AIC_BSY_PHASE(x) \
  (AIC_SCSI_PHASE(x) | (!!((x)&AICrBUSIN_BSY))*AIC_BUS_BSY)

#define SCSI_AIC_PHASE(x) ((x)<<6 | (!!((x)&SCSI_MSG))<<5)

#define AIC_NEXT_PHASE(x, phase) (((x)&~AIC_PHASE_MASK)|SCSI_AIC_PHASE(phase))

#define AIC_SCSI_PHASE_WAIT(regs, phase) \
  do { AIC_REG (regs, AIC_REG_BUSIN);    \
       while (AIC_SCSI_PHASE_FETCH(regs) != (phase)) ; \
     } while (0)
  
/* Set the default state to the register regno */
#define AIC_DEFAULT(regs, regno) \
	 AIC_WRITE(regs, regno, aic_default_state[regno])

/* Set the default state to the register regno */
#define AIC_GET_STATE(cs, reg) ((cs)->state [ reg ])

/* Macros to use the registers */

/* Select the specified register  */
#define AIC_REG(cs, regnum) \
	do { (cs)->reg = *((cs)->aic_reg_selector) = (regnum)&0xf; } while(0)

/* Read a value from a register and return that */
#define AIC_VAL(cs) \
	    ((cs)->reg += ((cs)->reg <= 7),(*(cs)->aic_reg_access)&0xff)

/* Store data to currently active AIC register */
#define AIC_STORE(cs, data)  \
  do { *(cs)->aic_reg_access = (cs)->state[(cs)->reg] = (data)&0xff; \
	(cs)->reg += (cs)->reg <= 7;                            \
     } while(0)

/* Store to the register the value previously cached for it */
#define AIC_RESTORE(cs) \
  do { *(cs)->aic_reg_access = (cs)->state[(cs)->reg]; \
	(cs)->reg += (cs)->reg <= 7;                   \
     } while(0)

/* Store data to the register; use AIC_RESTORE after this to
 * store back the value that we have cached for it.
 */
#define AIC_FORCE_IN(cs, data) \
  do { *(cs)->aic_reg_access = (data)&0xff; \
	(cs)->reg += (cs)->reg <= 7;   \
     } while(0)

/* Fetch data from currently active AIC register */
#define AIC_FETCH(cs, data)  (data) = AIC_VAL(cs)

/* Write in REGNUM the DATA */
#define AIC_WRITE(cs, regnum, data) \
	do { AIC_REG((cs), regnum); AIC_STORE((cs), (data)); } while (0)

/* Read from REGNUM the DATA */
#define AIC_READ(cs, regnum, data) \
	do { AIC_REG((cs), regnum); AIC_FETCH((cs), (data)); } while (0)

/* Write a new dma count to the chip registers */
#define AIC_WRITE_DMA_COUNT(cs, count) \
	do { AIC_WRITE ((cs), AIC_REG_DMA_L, (count)&0xff); \
	     AIC_STORE ((cs), ((count) >> 8)  & 0xff);       \
	     AIC_STORE ((cs), ((count) >> 16) & 0xff); } while (0)

/* Read the dma count currently in the dma registers */
#define AIC_READ_DMA_COUNT(cs, count) \
        do { AIC_READ ((cs), AIC_REG_DMA_L, count); \
	     count += (AIC_VAL (cs) << 8); \
	     count += (AIC_VAL (cs) << 16); } while (0)

/* Type definitions */

typedef struct aic_controller_type  *aic_controller_t;
typedef struct aic_softc *aic_softc_t;

/*
 * A script has a two parts: a pre-condition and an action.
 * The first triggers error handling if not satisfied and in
 * our case it is formed by the current bus phase and connected
 * condition as per bus status bits.  The action part is just a
 * function pointer, invoked in a standard way.  The script
 * pointer is advanced only if the action routine returns TRUE.
 * See sci_intr() for how and where this is all done.
 */

typedef struct script {
	int	condition;	/* expected state at interrupt */
	int	(*action)();	/* action routine */
} *script_t;

/*
 * State descriptor for this layer.  There is one such structure
 * per (enabled) AIC interface
 */
struct aic_softc {
	watchdog_t	wd;
	aic_controller_t	regs;	/* AIC registers */
	volatile char	*buff;		/* DMA buffer memory (I/O space) */
	script_t	script;
	int		(*error_handler)();
	int		in_count;	/* amount we expect to receive */
	int		out_count;	/* amount we are going to ship */

	volatile int	state;
#define	AIC_STATE_BUSY		0x01	/* selecting or currently connected */
#define AIC_STATE_TARGET	0x04	/* currently selected as target */
#define AIC_STATE_COLLISION	0x08	/* lost selection attempt */
#define AIC_STATE_DMA_IN	0x10	/* tgt --> initiator xfer */
#define AIC_STATE_DMA_OUT	0x20	/* initiator --> tgt xfer */
#define AIC_STATE_DMA_INTR	0x40	/* interrupted DMA */
#define AIC_STATE_INT_IGNORE	0x80	/* if INTR occurs return immediately */
#define AIC_STATE_PROBING	0x100	/* When doing aic_probe() */
#define AIC_STATE_WITH_ATN	0x200	/* select with ATN */

	unsigned char	ntargets;	/* how many alive on this scsibus */
	unsigned char	done;

	scsi_softc_t	*sc;
	target_info_t	*active_target;

	target_info_t	*next_target;	/* trying to seize bus */
	queue_head_t	waiting_targets;/* other targets competing for bus */

#ifdef PC532
	struct ns532_saved_state *dma_int_regs;
#endif
};

struct aic_controller_type {
  volatile char *aic_reg_selector;
  volatile char *aic_reg_access;
  char state[ 16 ];			/* I hate write only registers */
  char reg;
  aic_softc_t aic;
};

/* External declarations */

extern struct aic_controller_type aic_device [];
extern char aic_default_state [];

#define WAITFOR (AICrSTAT1_SELECTOUT | AICrSTAT1_CMDDONE)

#endif /* _SCSI_AIC_6250_H_ */

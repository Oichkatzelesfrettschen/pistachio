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
 * 28-Aug-92  Ian Dall (ian) at University of Adelaide
 *	Added pdma_state struct to sdp_softc_data struct to support
 *	pseudo-dma.
 *
 * 17-Aug-92  Ian Dall (ian) at University of Adelaide
 *	Added resid to sdp_softc. Maybe we don't need it if we rearrange
 *	things, but lets get it working first.
 *
 * 11-May-92  Johannes Helander (jvh) at Helsinki University of Technology
 *	Created.
 *
 * $Log:$
 */
/*
 *	Defines for the DP 8490 (SCSI chip), (almost NCR 5380)
 */

/*
 * Register map
 */

#ifndef	PAD
typedef struct {
   volatile u_char sdp_data;    	/* 0 CSD r:  Current data */
#define            sdp_odata sdp_data 	/* 0 ODR w:  Out data */
   volatile u_char sdp_icmd;    	/* 1 ICR rw: Initiator command */
   volatile u_char sdp_mode;    	/* 2 MR2 rw: Mode */
   volatile u_char sdp_tcmd;    	/* 3 TCR rw: Target command */
   volatile u_char sdp_bus_csr; 	/* 4 CSB r:  Bus Status */
#define         sdp_sel_enb sdp_bus_csr	/* 4 SER w:  Select enable */
   volatile u_char sdp_csr;     	/* 5 BSR r:  Status */
#define            sdp_dma_send sdp_csr /* 5 SDS w:  Start dma send data */
   volatile u_char sdp_idata;   	/* 6 IDR r:  Input data */
#define            sdp_trecv sdp_idata 	/* 6 SDR w:  Start dma rec target */
   volatile u_char sdp_emr;	   	/* 7 EMR rw: dp8490 enhanced mode reg */
#define 	   sdp_istatus sdp_emr	/* 7 ISR r:  Interrupt status */
#define 	   sdp_imask sdp_emr	/* 7 IMR w:  Interrupt mask */	
} sdp_regmap_t;

typedef sdp_regmap_t	sdp_padded_regmap_t;
#else
typedef struct {
	volatile unsigned char	sdp_data;	/* r:  Current data */
#define		sdp_odata sdp_data	/* w:  Out data */
	PAD(pad0);

	volatile unsigned char	sdp_icmd;	/* rw: Initiator command */
	PAD(pad1);

	volatile unsigned char	sdp_mode;	/* rw: Mode */
	PAD(pad2);

	volatile unsigned char	sdp_tcmd;	/* rw: Target command */
	PAD(pad3);

	volatile unsigned char	sdp_bus_csr;	/* r:  Bus Status */
#define		sdp_sel_enb sdp_bus_csr	/* w:  Select enable */
	PAD(pad4);

	volatile unsigned char	sdp_csr;	/* r:  Status */
#define		sdp_dma_send sdp_csr	/* w:  Start dma send data */
	PAD(pad5);

	volatile unsigned char	sdp_idata;	/* r:  Input data */
#define		sdp_trecv sdp_idata	/* w:  Start dma receive, target */
	PAD(pad6);

	volatile unsigned char	sdp_iack;	/* r:  Interrupt Acknowledge  */
#define		sdp_irecv sdp_iack	/* w:  Start dma receive, initiator */
	PAD(pad7);

} sdp_padded_regmap_t;
#endif



/*
 * ICR(1) Initiator command register
 */

#define SDP_ICMD_DATA		0x01		/* rw: Assert data bus   */
#define SDP_ICMD_ATN		0x02		/* rw: Assert ATN signal */
#define SDP_ICMD_SEL		0x04		/* rw: Assert SEL signal */
#define SDP_ICMD_BSY		0x08		/* rw: Assert BSY signal */
#define SDP_ICMD_ACK		0x10		/* rw: Assert ACK signal */
#define SDP_ICMD_LST		0x20		/* r:  Lost arbitration */
#define SDP_ICMD_DIFF	SDP_ICMD_LST		/* w:  Differential cable */
#define SDP_ICMD_AIP		0x40		/* r:  Arbitrat. in progress */
#define SDP_ICMD_ENHANCED SDP_ICMD_AIP 		/* w:  Enhaced/Normal mode */
#define SDP_ICMD_RST		0x80		/* rw: Assert RST signal */


/*
 * MR2(2) Mode register
 */

#define SDP_MODE_ARB		0x01		/* rw: Start arbitration */
#define SDP_MODE_DMA		0x02		/* rw: Enable DMA xfers */
#define SDP_MODE_MONBSY		0x04		/* rw: Monitor BSY signal */
#define SDP_MODE_DMA_IE		0x08		/* rw: Enable DMA complete interrupt */
#define SDP_MODE_PERR_IE	0x10		/* rw: Interrupt on parity errors */
#define SDP_MODE_PAR_CHK	0x20		/* rw: Check parity */
#define SDP_MODE_TARGET		0x40		/* rw: Target mode (Initiator if 0) */
#define SDP_MODE_BLOCKDMA	0x80		/* rw: Block-mode DMA handshake (MBZ) */


/*
 * TCR(3) Target command register
 */

#define SDP_TCMD_IO		0x01		/* rw: Assert I/O signal */
#define SDP_TCMD_CD		0x02		/* rw: Assert C/D signal */
#define SDP_TCMD_MSG		0x04		/* rw: Assert MSG signal */
#define SDP_TCMD_PHASE_MASK	0x07		/* r:  Mask for current bus phase */
#define SDP_TCMD_REQ		0x08		/* rw: Assert REQ signal */
#define	SDP_TCMD_LAST_SENT	0x80		/* ro: Last byte was xferred
						 *     (not on 5380/1) EDMA */

#define	SDP_PHASE(x)		SCSI_PHASE(x)

/*
 * CSB(4) Current (SCSI) Bus status
 */

#define SDP_BUS_DBP		0x01		/* r:  Data Bus parity */
#define SDP_BUS_SEL		0x02		/* r:  SEL signal */
#define SDP_BUS_IO		0x04		/* r:  I/O signal */
#define SDP_BUS_CD		0x08		/* r:  C/D signal */
#define SDP_BUS_MSG		0x10		/* r:  MSG signal */
#define SDP_BUS_REQ		0x20		/* r:  REQ signal */
#define SDP_BUS_BSY		0x40		/* r:  BSY signal */
#define SDP_BUS_RST		0x80		/* r:  RST signal */

#define	SDP_CUR_PHASE(x)	SCSI_PHASE((x)>>2)
#define SDP_PHASE_MASK (SDP_BUS_IO | SDP_BUS_CD | SDP_BUS_MSG)
#define SDP_CSB_PHASE(x) ((x) << 2 & SDP_PHASE_MASK)

/*
 * BSR(5) Bus and Status register
 */

#define SDP_CSR_ACK		0x01		/* r:  ACK signal */
#define SDP_CSR_ATN		0x02		/* r:  ATN signal */
#define SDP_CSR_DISC		0x04		/* r:  Disconnected (BSY==0) */
#define SDP_CSR_PHASE_MATCH	0x08		/* r:  Bus and SDP_TCMD match*/
#define SDP_CSR_INT		0x10		/* r:  Interrupt request */
#define SDP_CSR_PERR		0x20		/* r:  Parity error */
#define SDP_CSR_DREQ		0x40		/* r:  DMA request */
#define SDP_CSR_DONE		0x80		/* r:  DMA count is zero */

/*
 * EMR(7) Enhanced Mode Register
 */

#define SDP_EMR_ARB		0x01	/* (r)w:  extended arbitration */
#define SDP_EMR_FN_MASK		0x06	/* w: bits 1,2 encode function */
#define SDP_EMR_FN_NOP		0x00	/* no function */
#define SDP_EMR_FN_RPI		0x02	/* reset parity and interrupt latches*/
#define SDP_EMR_FN_SDI		0x04	/* start DMA receive, initiator */
#define SDP_EMR_FN_INT		0x06	/* next read/write of reg 7 (normally
					 * EMR) will access ISR/IMR */
#define SDP_EMR_LOOP		0x08	/* (r)w: loop back mode */
#define SDP_EMR_SPOL		0x10	/* Parity polarity. 0=odd, 1=even */
#define SDP_EMR_MPOL		0x20	/* cpu parity, ???. 0=odd */
#define SDP_EMR_MPEN		0x40	/* cpu bus parity checking */
#define SDP_EMR_PHASE_CHECK	0x80	/* Interrupt on phase mismatch */

/*
 * ISR(7) / IMR(7)
 */
#define SDP_INT_ARB		0x01	/* arbitration complete */
#define SDP_INT_SEL		0x02	/* selection/reselection */
#define SDP_INT_BSY		0x04	/* Busy loss */
#define SDP_INT_PHASE		0x08	/* any phase mismatch */
#define SDP_INT_DMA_PHASE	0x10	/* DMA phase mismatch */
#define SDP_INT_EDMA		0x20	/* end of DMA */
#define SDP_INT_MPE		0x40	/* CPU bus parity error */
#define SDP_INT_PARITY		0x80	/* SCSI bus parity error */


/*
 * A script has a two parts: a pre-condition and an action.
 * The first triggers error handling if not satisfied and in
 * our case it is formed by the current bus phase and connected
 * condition as per bus status bits.  The action part is just a
 * function pointer, invoked in a standard way.  The script
 * pointer is advanced only if the action routine returns TRUE.
 * See sdp_intr() for how and where this is all done.
 */

typedef struct sdp_softc *sdp_softc_t;

typedef struct script {
	int	condition;	/* expected state at interrupt */
	int	(*action)(sdp_softc_t, unsigned, unsigned);	/* action routine */
} *script_t;

/*
 * State descriptor for this layer.  There is one such structure
 * per (enabled) 5380 interface
 */
struct sdp_softc {
	watchdog_t	wd;
	sdp_padded_regmap_t	*regs;		/* 5380 registers */
	volatile char	*buff;		/* DMA buffer memory (I/O space) */
	script_t	script;
	int		(*error_handler)(sdp_softc_t, unsigned, unsigned);
	int		in_count;	/* amnt we expect to receive */
	int		out_count;	/* amnt we are going to ship */
	int		resid;		/* amount left if rudely interrupted */

	volatile int	state;
#define	SDP_STATE_BUSY		0x01	/* selecting or currently connected */
#define SDP_STATE_TARGET	0x04	/* currently selected as target */
#define SDP_STATE_COLLISION	0x08	/* lost selection attempt */
#define SDP_STATE_DMA_IN	0x10	/* tgt --> initiator xfer */
#define SDP_STATE_PDMA  	0x20	/* Pseudo-dma process in progress */
#define SDP_STATE_DMA_INTR	0x40	/* Handling a dma interrupt */
#define SDP_STATE_PROBING       0x80    /* When doing sdp_probe */
#define SDP_STATE_DMA_DONE      0x100   /* Set when xfer count has been adjusted */

	unsigned char	ntargets;	/* how many alive on this scsibus */
	unsigned char	done;

	scsi_softc_t	*sc;
	target_info_t	*active_target;

	target_info_t	*next_target;	/* trying to seize bus */
	queue_head_t	waiting_targets;/* other targets competing for bus */
#if defined(PDMA_VERSION) && (PDMA_VERSION == 1 || PDMA_VERSION == 2)
	struct pdma_state pdma;	/* Support for pseudo-dma */
#endif
} sdp_softc_data[NSCSI];


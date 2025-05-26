/* 
 * $RCSfile: xmed.c,v $
 *
 * x-kernel v3.2
 *
 * Copyright (c) 1991  Arizona Board of Regents
 *
 * x-kernel driver for the Medusa FDDI board.
 *
 */
/*
 * HISTORY
 * $Log: xmed.c,v $
 * Revision 1.11  1993/05/04  21:54:52  davidm
 * Bogus call to enlist_tail() fixed.  Also added some type casts.
 *
 * Revision 1.10  1993/02/23  20:50:23  davidm
 * assert(old_handler==NULL)---never worked (the old handler points to
 * int_illegal() if everything works properly).
 *
 * Revision 1.9  1993/02/20  23:52:58  davidm
 * Fixed error message in line 419.
 *
 * Revision 1.8  1993/02/18  22:17:11  davidm
 * Test code removed to keep source code uncluttered with conditional
 * compilation.
 *
 * Revision 1.7  1993/02/18  22:15:51  davidm
 * This version contains the roundtrip test which was used to measure board-
 * to-board roundtrip times and throughput.
 *
 * Revision 1.6  1993/02/13  02:01:24  davidm
 * Well, this time the MTU is really set correctly to a multiple of 4.
 *
 * Revision 1.5  1993/02/13  01:59:58  davidm
 * Changed MTU to be a multiple of 4 (8164 instead of 8165).
 *
 * Revision 1.4  1992/12/01  18:54:28  davidm
 * This driver contains code for a board-to-board throughput
 * test (conditionally compiled by the TEST_TRUPUT macro).
 * With this test, we got 99.54 MBits/sec throughput for
 * 8185 byte packets).
 *
 * Revision 1.3  1992/11/18  04:21:19  davidm
 * This is the first **WORKING** version.  No elaborate handling of
 * rx and tx buffers, but it's working after all!
 *
 * Revision 1.2  1992/11/07  07:37:33  davidm
 * Some more functionality added.
 *
 * Revision 1.1  1992/11/05  18:46:42  davidm
 * Initial revision
 *
 */

/* get number of configured interfaces: */
#include "xmed.h"

#if NXMED > 0

#include "xmed_i.h"
#include "xmed_util.h"
/*
 * Get definitions to access STI ROM:
 */
#include "hpsgc/grf_stireg.h"
/*
 * Get system stuff:
 */
#include <hpsgc/ostype.h>
#include <sys/types.h>
#include <machine/spl.h>
#include <machine/Jasp.h>
#include <machine/psw.h>

/*
 * Get x-kernel stuff:
 */
#include "xkernel.h"
#include "eth.h"	/* pretend to be an ethernet device for now... */
#include "eth_i.h"
#include "msg.h"
#include "msg_internal.h"
#include "trace.h"
#include "list.h"
#include "kern_process_msg.h"
#include "process.h"

/*
 * Divide VRAM buffers into rx and tx buffers:
 */
#define NUM_BUFFERS	128	/* total # of buffers in VRAM */
#define NUM_TX_BUFFERS	32				/* # of tx buffers */
#define NUM_RX_BUFFERS	(NUM_BUFFERS-NUM_TX_BUFFERS)	/* # of rx buffers */

#define VENOM_SUP_CONTROL 	0xfffbd080	/* Venom supervisor ctrl adr */

#define FDDI_HEADER	0x307800d0	/* magic at start of FDDI packet */

#define FDDI_HEADER_SIZE (4)
#define MAC_HEADER_SIZE	(2*XMED_NET_ADDR_LEN)
#define ETH_TYPE_SIZE	(4)	/* eth-type (is a short, but we use a word) */
#define TRAILER_SIZE	(6)	/* size of checksum & junk added by Medusa */
#define MTU		(XMED_BUF_SIZE - 2 - \
			 (FDDI_HEADER_SIZE + MAC_HEADER_SIZE \
			  + ETH_TYPE_SIZE + TRAILER_SIZE))

/*
 * Medusa board descriptor.  There is one for each board:
 */
typedef struct xmed_desc {
    bool		seated;		/* is board seated and driver ready? */
    int			unit;		/* unit number */
    xmed_board_t	*board;		/* hard physical address */
    char		net_addr[XMED_NET_ADDR_LEN];	/* network address */
    XObj		my_protl;	/* pointer to own protocol */
    /* statistics: */
} xmed_desc_t;

/*
 * Interval to check status of ring (in micro-seconds):
 */
#define XMED_WATCHDOG_INTERVAL	(10*1000*1000)

/*
 * The unit number is stored in the protocol's state field:
 */
#define XMED_UNIT(s)	((int)(s)->state)

int tracexmedp = 0;

static xmed_desc_t xmed_desc[NXMED];
static int zeros = 0;			/* four zero bytes */

static int xmed_intr(int unit);
static xkern_return_t xmed_openenable(XObj self, XObj hlp, XObj hlptype,
				      Part *part);
static xmsg_handle_t xmed_push(XObj s, Msg *downMsg);
static int xmed_control(XObj self, int op, char *buf, int len);

/********************** non-protocol operations **********************/

/*
 * Dump ethernet header:
 */
static void
xmed_dump_hdr(ETHhdr *hp)
{
    printf(
      "dump of header: src=[%04x:%04x:%04x], dst=[%04x:%04x:%04x], type=%x\n",
	   hp->src.high, hp->src.mid, hp->src.low,
	   hp->dst.high, hp->dst.mid, hp->dst.low, hp->type);
} /* xmed_dump_hdr */


/*
 * Dump the first N bytes of a packet:
 */
static void
xmed_dump_pkt(u_int *pkt, int n)
{
    int i;

    printf("packet dump of first %d bytes:\n", n);
    for (i = 0; i < n/sizeof(int); i++) {
	printf("%08x ", pkt[i]);
	if (i % 8 == 7) {
	    putchar('\n');
	} /* if */
    } /* for */
    if (i % 8) {
	putchar('\n');
    } /* if */
} /* xmed_dump_pkt */


/*
 * Restart the Motorola chip set (and therefore the FDDI ring):
 */
static void
xmed_restart_fddi(xmed_desc_t *xd)
{
    xmed_board_t *m = xd->board;

    xTrace0(xmedp, TR_FUNCTIONAL_TRACE, "xmed_restart_fddi()");

    m->med.reset_fddi = 0;
    m->med.reset_fddi = 0xffffffff;

    /* turn off state machine: */

    xTrace0(xmedp, TR_DETAILED,
	    "xmed_restart_fddi: turning off PCM state machine");

    m->elm.cntrl_b = ELM_PCM_CNTRL_STOP;
    while ((m->elm.status_b & ELM_PCM_STATE) != ELM_PCM_OFF) {
	/* wait for the state machine to go to the OFF state */
    } /* while */
    
    /* configure ELM: */

    xTrace0(xmedp, TR_DETAILED, "xmed_restart_fddi: configuring ELM");

    m->elm.intr_mask = ELM_INTR_MASK_CONFIG;
    m->elm.le_threshold = ELM_LE_THRESHOLD_CONFIG;
    m->elm.a_max = ELM_A_MAX_CONFIG;
    m->elm.ls_max = ELM_LS_MAX_CONFIG;
    m->elm.tb_min = ELM_TB_MIN_CONFIG;
    m->elm.t_out = ELM_T_OUT_CONFIG;
    m->elm.lc_short = ELM_LC_SHORT_CONFIG;
    m->elm.t_scrub = ELM_T_SCRUB_CONFIG;
    m->elm.ns_max = ELM_NS_MAX_CONFIG;
    m->elm.cntrl_a = ELM_CNTRL_A_CONFIG;
    m->elm.cntrl_b = ELM_PC_MAINT | 0x600;

    /* stop MAC: */
    m->mac.cntrl_a = ~MAC_ON;

    /* configure MAC: */

    xTrace0(xmedp, TR_DETAILED, "xmed_restart_fddi: configuring MAC");

    m->mac.intr_mask_a = 0;
    m->mac.intr_mask_b = 0;
    m->mac.msa = MAC_MSA_CONFIG;
    m->mac.mla_a = (xd->net_addr[4] << 8) | xd->net_addr[5];
    m->mac.mla_b = (xd->net_addr[2] << 8) | xd->net_addr[3];
    m->mac.mla_c = (xd->net_addr[0] << 8) | xd->net_addr[1];
    m->mac.t_req = MAC_T_REQ_CONFIG;
    m->mac.u1.t_max = MAC_T_MAX_CONFIG;

    /* start MAC: */

    xTrace0(xmedp, TR_DETAILED, "xmed_restart_fddi: starting MAC");

    m->mac.cntrl_a = MAC_CNTRL_A_CONFIG;
    m->mac.cntrl_b = MAC_CNTRL_B_CONFIG;

    xTrace0(xmedp, TR_DETAILED,
	    "xmed_restart_fddi: waiting for ring to become operational");

    while (!(m->mac.tx_status & MAC_RING_OPERATIONAL)) {
	/* skip */;
    } /* while */

    xTrace0(xmedp, TR_FUNCTIONAL_TRACE, "xmed_restart_fddi: returning");
} /* xmed_restart_fddi */


static void
xmed_check_ring(Event ev, void *arg)
{
    xmed_desc_t *xd = (xmed_desc_t*) arg;
    xmed_board_t *m = xd->board;
    
    xTrace1(xmedp, TR_FUNCTIONAL_TRACE, "xmed_check_ring(unit=%d)",
	    xd->unit);

    if (!(m->mac.tx_status & MAC_RING_OPERATIONAL)) {
	printf(
	   "xmed%d: ring not operational, restarting it (mac.tx_status=%x)\n",
	       xd->unit, m->mac.tx_status);

	xmed_restart_fddi(xd);
    } /* if */

    evDetach(evSchedule(xmed_check_ring, (void*) xd, XMED_WATCHDOG_INTERVAL));

    xTrace0(xmedp, TR_FUNCTIONAL_TRACE, "xmed_check_ring: returning");
} /* xmed_check_ring */


/*
 * Check for the presence of a Medusa board.
 */
static bool
xmed_probe(xmed_desc_t *xd)
{
    /* check whether high word of ROM ID matches Medusa ROM ID: */
    return STI_ID_HI(xd->board) == XMED_STI_ID;
} /* xmed_probe */


/*
 * Perform the following tasks:
 *
 *   - Reset the Medusa card.
 *   - Check the values of selected registers against their expected boot-up
 *     values.
 *   - Partition the VRAM into transmit and receive buffers and fill up the
 *     respective fifos.
 *   - Enable interrupts.
 */
static bool
xmed_hw_init(xmed_desc_t *xd)
{
    xmed_board_t *m = xd->board;
    u_int result;
    bool okay;
    int i;
    u_int *buf;

    xTrace1(xmedp, TR_FUNCTIONAL_TRACE, "xmed_hw_init(board=0x%x)", m);

    /* assert reset: */
    m->med.reset_fifo = 0;
    m->med.reset_misc = 0;
    m->med.reset_fddi = 0;

    /* initialize other card registers:  */
    m->chk_sum = INIT_CHECKSUM_UNIT;
    m->med.test_fcg = INIT_TEST_FCG;
    m->med.test_elm = INIT_TEST_ELM;
    m->med.test_mac = INIT_TEST_MAC;
    /* disable all interrupts: */
    m->med.interrupt_mask = INIT_INTERRUPT_MASK;
    /* reset interrupt event mask: */
    m->med.interrupt_event = ~INIT_INTERRUPT_EVENT & 0xff; /* kludge */
    m->med.misc_output = INIT_MISC_OUTPUT;

    /* send all one's to release reset: */
    m->med.reset_fifo = 0xffffffff;
    m->med.reset_misc = 0xffffffff;
    m->med.reset_fddi = 0xffffffff;

    okay = TRUE;

#ifdef __STDC__
# define check(regn,reg,val) \
   if ((result = reg & 0xff) != val) { \
       printf( \
     "xmed_hw_init: "#regn" incorrectly set to 0x%08lx instead of 0x%08lx\n", \
	      result, val); \
       okay = FALSE; \
   } /* if */
#else
# define check(regn,reg,val) \
   if ((result = reg & 0xff) != val) { \
       printf( \
      "xmed_hw_init: regn incorrectly set to 0x%08lx instead of 0x%08lx\n", \
	      result, val); \
       okay = FALSE; \
   } /* if */
#endif

    check(RESET_MISC, m->med.reset_misc, INIT_RESET_MISC);
    check(RESET_FIFO, m->med.reset_fifo, INIT_RESET_FIFO);
    check(RESET_FDDI, m->med.reset_fddi, INIT_RESET_FDDI);
    check(TEST_FCG, m->med.test_fcg, INIT_TEST_FCG);
    check(TEST_ELM, m->med.test_elm, INIT_TEST_ELM);
    check(TEST_MAC, m->med.test_mac, INIT_TEST_MAC);
    check(INTERRUPT_MASK, m->med.interrupt_mask, INIT_INTERRUPT_MASK);
    check(INTERRUPT_EVENT, m->med.interrupt_event, INIT_INTERRUPT_EVENT);
    check(MISC_OUTPUT, m->med.misc_output, INIT_MISC_OUTPUT);
    check(STATUS_SYNCH_FIFO, m->med.status_synch_fifo, INIT_STATUS_SYNCH_FIFO);
    check(STATUS_BUFF_FIFO, m->med.status_buff_fifo, INIT_STATUS_BUFF_FIFO);
    check(MISC_INPUT, m->med.misc_input, INIT_MISC_INPUT);
    check(CHECK_SUM_UNIT, m->chk_sum, INIT_CHECKSUM_UNIT);

    /* do an extra cycle for reset_fifo: */
    m->med.reset_fifo = 0;
    m->med.reset_fifo = 0xffffffff;

    /* put some tx buffers into TX_FREE_FIFO: */
    buf = (u_int*) m->vram_std;
    for (i=0; i < NUM_TX_BUFFERS; i++) {
	m->tx_ready = buf;
	buf += XMED_BUF_SIZE / sizeof(u_int);
    } /* if */

    /* give receiver a couple of receive buffers: */
    for (i=0; i < NUM_RX_BUFFERS; i++) {
	m->rx_free = buf;
	buf += XMED_BUF_SIZE / sizeof(u_int);
    } /* for */

    /* allow receive interrupts: */
    m->med.interrupt_mask = IMR_RX_READY_NON_EMPTY;

    xTrace1(xmedp, TR_FUNCTIONAL_TRACE,
	    "xmed_hw_init: returning %s", okay ? "successfully" : "failure");

    return okay;
} /* xmed_hw_init */


/*
 * Attach board to system.  First the hardware is initialized, then
 * some fields in the xd descriptor are filled in.
 */
static void
xmed_attach(xmed_desc_t *xd)
{
    int (*old_handler)();
    u_int intr_mask;
    bool first = TRUE;
    int rom_addr;
    int i;

    xTrace0(xmedp, TR_FUNCTIONAL_TRACE, "xmed_attach()");

    /*
     * Fetch network address (it's stored in the last 6 words of the
     * STI ROM; only the LSB is significant):
     */
    rom_addr = XMED_NET_ADDR_OFFSET / sizeof(int);
    printf("xmed%d: net address [", xd->unit);
    for (i=0; i < XMED_NET_ADDR_LEN; i++) {
	xd->net_addr[i] = xd->board->sti_rom[rom_addr++] & 0xff;
	printf(first ? "%x" : ":%x", xd->net_addr[i]);
	first = FALSE;
    } /* for */
    printf("]\n");

    xmed_hw_init(xd);

    /* driver and board are now ready for interrupts: */
    xd->seated = TRUE;

    /* install interrupt handler: */
    if ((int) xd->board == SGC_SLOT1) {
	/* SGC slot #1: */
	intr_mask = INT_GRF1;
    } else if ((int) xd->board == SGC_SLOT2) {
	/* SGC slot #2: */
	/* appears to use the same interrupt line!??! */
	intr_mask = INT_GRF1;
    } else {
	printf("xmed%d: board in unknown slot at addr %x\n",
	       xd->unit, xd->board);
	intr_mask = 0;
    } /* if */

    /* get ring watchdog started: */
    xmed_check_ring(0, xd);

    if (aspitab(intr_mask, SPLGRF, xmed_intr, xd->unit, &old_handler) < 0) {
	printf("xmed%d: couldn't install interrupt handler\n", xd->unit);
    } /* if */
    xTrace0(xmedp, TR_FUNCTIONAL_TRACE, "xmed_attach: returning");
} /* xmed_attach */


/*
 * Handle Medusa interrupt.  The Medusa board generates an interrupt for every
 * incoming packet (even if it's not addressed to us).
 */
static int
xmed_intr(int unit)
{
    xmed_desc_t *xd = &xmed_desc[unit];
    xmed_board_t *m = xd->board;
    u_int *buf;
    int len;
    struct threadIBlock *ti;
    struct input_buffer *xbuf;

    xTrace1(xmedp, TR_FUNCTIONAL_TRACE, "xmed_intr(unit=%d)", unit);

    if (!xd->seated) {
	xTrace1(xmedp, TR_SOFT_ERRORS, "xmed_intr(unit=%d): board not seated",
		unit);
	return 0;
    } /* if */

    ledcontrol(0, 0, LED_LANRCV);

    do {
	while ((u_int) (buf = m->rx_ready) & FIFO_NON_EMPTY) {
	    len = (u_int) buf & FIFO_LENGTH_MASK;
	    buf = (u_int*) ((u_int) buf & ~FIFO_LENGTH_MASK);

	    xTrace2(xmedp, TR_EVENTS, "xmed_intr: got rx buffer @ %x len=%d",
		    buf, len);

	    /*
	     * The following if statement can go away once the Motorola
	     * chipset is fixed.
	     */
	    if ((len <= FDDI_HEADER_SIZE + MAC_HEADER_SIZE) ||
		!(XMED_SAME_ADDRESS(&buf[1], (u_int*)xd->net_addr) ||
		  XMED_BROADCAST_ADDRESS(&buf[1])))
	    {
		/* junk pkt (bug in Motorola chip set): recycle rx buffer: */
		xTrace0(xmedp, TR_SOFT_ERRORS,
			"xmed_intr: dropping junk packet");
		m->rx_free = buf;
		continue;
	    } /* if */

	    /* lop off length of headers and trailer: */
	    len -= FDDI_HEADER_SIZE + MAC_HEADER_SIZE +
	      ETH_TYPE_SIZE + TRAILER_SIZE;

#if 0
	    xmed_dump_pkt(buf, 64);
#endif

	    /* try to find a free shepherd thread: */
	    ti = (struct threadIBlock*) delist_head(&xkThreadQ);
	    if (ti) {
		ti->buf = xbuf =
		  (struct input_buffer*) delist_head(&xkBufferPool);
		if (xbuf) {
		    xbuf->driverProtl = xd->my_protl;
		    xmed_copy_in(buf, len, (ETHhdr*) &xbuf->hdr,
				 (u_int*) xbuf->data);
#if 0
xmed_dump_hdr((ETHhdr*)&xbuf->hdr);
xmed_dump_pkt((u_int*)xbuf->data, 64);
#endif
		    msgSetAttr(&ti->buf->upmsg, 0, &xbuf->hdr, sizeof(ETHhdr));
		} else {
		    xError(
		 "xmed_intr: dropped incoming msg (got thread but no buffer)");
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
		if (!xkIncomingData_lock) {
		    xbuf = (struct input_buffer*) delist_head(&xkBufferPool);
		    if (xbuf) {
			xbuf->driverProtl = xd->my_protl;
			xmed_copy_in(buf, len, (ETHhdr*) &xbuf->hdr,
				     (u_int*) xbuf->data);
#if 0
xmed_dump_hdr((ETHhdr*)&xbuf->hdr);
xmed_dump_pkt((u_int*)xbuf->data, 64);
#endif
			msgSetAttr(&ti->buf->upmsg, 0, &xbuf->hdr,
				   sizeof(ETHhdr));
			enlist_tail(&xkIncomingData, (list_entry_t) xbuf);
		    } else {
			xError(
		"xmed_intr: dropped incoming msg (no thread, no buffer)");
		    } /* if */
		} else {
		    xError(
		"xmed_intr: dropped incoming msg (incomingData queue locked)");
		} /* if */
	    } /* if */

	    /* recycle rx buffer: */
	    m->rx_free = buf;
	} /* while */

	/* clear interrupts: */
	m->med.interrupt_event = m->med.interrupt_event;
    } while (m->med.status_buff_fifo & BSR_RX_READY_NON_EMPTY);

    xTrace0(xmedp, TR_FUNCTIONAL_TRACE, "xmed_intr: returning");

    return 0;
} /* xmed_intr */


/*********************** protocol operations ***********************/

/*
 * Initialize driver and hardware:
 */
xkern_return_t
xmed_init(XObj self)
{
    struct modtab *mptr;
    struct modtab *getmod(int type, int noinit);
    xmed_desc_t xd;
    int unit = 0;
    int noinit = 0;
    static bool inited = FALSE;

    xTrace0(xmedp, TR_FUNCTIONAL_TRACE, "xmed_init()");

    if (!inited) {

	inited = TRUE;

	/* look through Foreign I/O modules and attach all Medusa boards: */

	while ((mptr = getmod(IODC_TP_FIO, noinit)) != 0) {
	    if (mptr->m_type.iodc_sv_model == SVMOD_FIO_SGC) {
		xd.board = (xmed_board_t*) mptr->m_hpa;
		if (xmed_probe(&xd)) {
		    /*
		     * It's really a Medusa card, not a graphics card:
		     */
		    if (unit >= NXMED) {
			printf("xmed%d: can't config (NXMED exceeded)\n",
			       unit);
			continue;
		    } /* if */
		    xd.unit = unit;
		    xmed_desc[unit] = xd;
		    xmed_attach(&xmed_desc[unit++]);
		} /* if */
	    } /* if */
	    noinit = 1;
	} /* while */

	while (unit < NXMED) {
	    xmed_desc[unit++].seated = FALSE;
	} /* while */
    } /* if */

    unit = self->instName[strlen(self->instName)-1] - '0';
    if (unit < 0 || unit >= NXMED) {
	unit = 0;
	printf("xmed_init: unit number out of range, assuming unit 0 for %s\n",
	       self->instName);
    } /* if */

    if (!xmed_desc[unit].seated) {
	printf("xmed_init: Medusa board with unit number %d not seated\n",
	       unit);
    } /* if */

    xmed_desc[unit].my_protl = self;
    XMED_UNIT(self) = unit;

    /* setup protocol function pointers: */
    self->openenable = xmed_openenable;
    self->push = xmed_push;
    self->control = xmed_control;

    xTrace0(xmedp, TR_FUNCTIONAL_TRACE, "xmed_init: returning successfully");

    return XK_SUCCESS;
} /* xmed_init */


/*
 * Control operations:
 */
int
xmed_control(XObj self, int op, char *buf, int len)
{
    xmed_desc_t *xd;
    int unit;
    int retval;
    int *ival;

    xTrace1(xmedp, TR_FUNCTIONAL_TRACE, "xmed_control(op=%x)", op);

    unit = XMED_UNIT(self);
    if (unit < 0 || unit >= NXMED) {
	printf("xmed_control: unit out of range, assuming unit 0 for %s\n",
	       self->instName);
	unit = 0;
    } /* if */

    xd = &xmed_desc[unit];

    retval = -1;

    switch (op) {
      case GETMYHOST:
        if (len >= XMED_NET_ADDR_LEN) {
	    bcopy(xd->net_addr, buf, XMED_NET_ADDR_LEN);
	    retval = XMED_NET_ADDR_LEN;
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

    xTrace1(xmedp, TR_FUNCTIONAL_TRACE, "xmed_control: returning %d", retval);

    return retval;
} /* xmed_control */


/*
 * The openenable interface allows the driver get the address of the
 * higher level protocol to which it interfaces.
 */
xkern_return_t
xmed_openenable(XObj self, XObj hlp, XObj hlptype, Part *part)
{
    int unit;

    xTrace0(xmedp, TR_FUNCTIONAL_TRACE, "xmed_openenable()");

    unit = XMED_UNIT(self);

    xTrace2(xmedp, TR_DETAILED, "xmed_openenable: unit=%d, NXMED=%d\n",
	    unit, NXMED);

    if (unit < 0 || unit >= NXMED) {
	printf("xmed: unit number out of range, assuming unit 0 for %s\n",
	       self->instName);
	unit = 0;
    } /* if */

    /* store high-level protocol in up pointer: */
    xSetUp(self, hlp);

    xTrace0(xmedp, TR_FUNCTIONAL_TRACE,
	    "xmed_openenable: returning XK_SUCCESS");

    return XK_SUCCESS;
} /* xmed_openenable */


/*
 * Send a message out to ether.
 */
xmsg_handle_t
xmed_push(XObj s, Msg *downMsg)
{
    xmed_desc_t *xd;
    xmed_board_t *m;
    u_int *pkt, *next;
    ETHhdr *hp = (ETHhdr*) msgGetAttr(downMsg, 0);
    int len;
    xmed_copy_state_t state;

    xTrace0(xmedp, TR_FUNCTIONAL_TRACE, "xmed_push()");

    xd = &xmed_desc[XMED_UNIT(s)];
    m = xd->board;

    /* make sure ring is operational: */
    if (!(m->mac.tx_status & MAC_RING_OPERATIONAL)) {
	xmed_restart_fddi(xd);
    } /* if */

    pkt = next = m->tx_free;

#if 0
    if (!((u_int)pkt & FIFO_NON_EMPTY)) {
	/* wouldn't it be better to block until a buffer becomes available? */
	xError("xmed_push: no tx buffer available, dropping packet");
	return XMSG_NULL_HANDLE;
    } /* if */
#else
    if (!((u_int)pkt & FIFO_NON_EMPTY)) {
	xError("xmed_push: no tx buffer available, busy-waiting for one");
	/* it probably won't take long until we a buffer, so do busy-wait: */
	do {
	    pkt = next = m->tx_free;
	} while (!((u_int)pkt & FIFO_NON_EMPTY));
    } /* if */
#endif

    xTrace1(xmedp, TR_DETAILED, "xmed_push: got tx buffer %x", pkt);

    /* put FDDI packet header into xmt buffer: */
    *next++ = FDDI_HEADER;

    /* put destination and source address into xmt buffer: */
    *next++ = (hp->dst.high << 16) | hp->dst.mid;
    *next++ = (hp->dst.low  << 16) | hp->src.high;
    *next++ = (hp->src.mid  << 16) | hp->src.low;
    *next++ = hp->type;	/* use whole word although type is only 16 bits */

    /* header length: */
    len = (u_char*)next - (u_char*)pkt;

    /* add msg data: */
    state.next_word = next;
    state.remaining_space = 0;
    msgForEach(downMsg, (XCharFun) xmed_copy_out, &state);
    if (state.remaining_space) {
	xmed_copy_out((u_char*) &zeros, state.remaining_space, &state);
    } /* if */
    len += msgLen(downMsg);

#if 0
    xmed_dump_pkt(pkt, 64);
#endif

    /* send msg off: */

    xTrace2(xmedp, TR_EVENTS, "xmed_push: sending packet @ %x len=%d",
	    pkt, len);
    m->tx_ready = (u_int*)((u_int) pkt | len);
 
    ledcontrol(0, 0, LED_LANXMT);

    xTrace0(xmedp, TR_FUNCTIONAL_TRACE, "xmed_push: returning");

    return XMSG_NULL_HANDLE;
} /* xmed_push */

#endif /* NXMED > 0 */

			/*** end of xmed.c ***/

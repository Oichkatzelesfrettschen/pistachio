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
 * 28-Sep-92  Ian Dall (ian) at University of Adelaide
 *	Make SDP_UNIT 0 and AIC_UNIT 1.
 *
 * 17-Aug-92  Ian Dall (ian) at University of Adelaide
 *	Map 2 pages of pc532_dma space.
 *
 * 17-Aug-92  Ian Dall (ian) at University of Adelaide
 *	Add sdp unit no to args in call of sdp_buffer_init().
 *
 * 13-May-92  Johannes Helander (jvh) at Helsinki University of Technology
 *	Created.
 *
 * $Log: autoconf.c,v $
 */
/*
 * 	File: ns532/PC532/autoconf.c
 *	Author: Johannes Helander, Jukka Virtanen
 *	Helsinki University of Technology 1992.
 */

#include <platforms.h>
#include <sys/types.h>

#include <mach/ns532/vm_types.h>
#include <mach/vm_param.h>
#include <vm/vm_map.h>
#include <vm/vm_kern.h>
#include <mach/kern_return.h>
#include <chips/busses.h>

#include <sdp.h>
#include <aic.h>
#include <ns532/PC532/board.h>

extern struct bus_driver sdp_driver;
extern struct bus_driver aic_driver;
extern int sdp_intr();
extern int aic_intr();

#define SDP_UNIT 0
#define AIC_UNIT 1
int sdp_unit = SDP_UNIT;	/* PC532 has only one sdp */
int aic_unit = AIC_UNIT;	/* PC532 has only one aic */


struct  bus_ctlr        bus_master_init[] = {

/*driver,      	name, 	unit, intr, address+am, phys, adpt, alive,flags, */
#if NSDP > 0
{ &sdp_driver, 	"sdp", SDP_UNIT, sdp_intr,  0x0,0, 0,   '?', 0,    0, },
#endif /* NSDP > 0 */
#if NAIC > 0
{ &aic_driver, 	"aic", AIC_UNIT, aic_intr,  0x0,0, 0,   '?', 0,    0, },
#endif /* NAIC > 0 */
        0
};

struct  bus_device      bus_device_init[] = {
/* driver,      name, unit,intr,addr+am,phys, adaptor,alive,ctlr,slave,flags,*/
#if NSDP > 0
#if SDP_UNIT == 0
{ &sdp_driver,  "rz",   0,  0,  0x0,0,  0,    '?',     0,   '?',   0,    0, },
{ &sdp_driver,  "rz",   1,  0,  0x0,0,  0,    '?',     0,   '?',   1,    0, },
{ &sdp_driver,  "rz",  	2,  0,  0x0,0,  0,    '?',     0,   '?',   2,    0, },
{ &sdp_driver,  "rz",  	3,  0,  0x0,0,  0,    '?',     0,   '?',   3,    0, },
{ &sdp_driver,  "rz",  	4,  0,  0x0,0,  0,    '?',     0,   '?',   4,    0, },
{ &sdp_driver,  "rz",  	5,  0,  0x0,0,  0,    '?',     0,   '?',   5,    0, },
{ &sdp_driver,  "rz",  	6,  0,  0x0,0,  0,    '?',     0,   '?',   6,    0, },
{ &sdp_driver,  "rz",  	7,  0,  0x0,0,  0,    '?',     0,   '?',   7,    0, },

{ &sdp_driver,  "tz",   0,  0,  0x0,0,  0,    '?',     0,   '?',   0,    0, },
{ &sdp_driver,  "tz",   1,  0,  0x0,0,  0,    '?',     0,   '?',   1,    0, },
{ &sdp_driver,  "tz",   2,  0,  0x0,0,  0,    '?',     0,   '?',   2,    0, },
{ &sdp_driver,  "tz",   3,  0,  0x0,0,  0,    '?',     0,   '?',   3,    0, },
{ &sdp_driver,  "tz",   4,  0,  0x0,0,  0,    '?',     0,   '?',   4,    0, },
{ &sdp_driver,  "tz",   5,  0,  0x0,0,  0,    '?',     0,   '?',   5,    0, },
{ &sdp_driver,  "tz",   6,  0,  0x0,0,  0,    '?',     0,   '?',   6,    0, },
{ &sdp_driver,  "tz",   7,  0,  0x0,0,  0,    '?',     0,   '?',   7,    0, },

{ &sdp_driver,  "sc",   0,  0,  0x0,0,  0,    '?',     0,   '?',   0,    0, },
{ &sdp_driver,  "sc",   1,  0,  0x0,0,  0,    '?',     0,   '?',   1,    0, },
{ &sdp_driver,  "sc",   2,  0,  0x0,0,  0,    '?',     0,   '?',   2,    0, },
{ &sdp_driver,  "sc",   3,  0,  0x0,0,  0,    '?',     0,   '?',   3,    0, },
{ &sdp_driver,  "sc",   4,  0,  0x0,0,  0,    '?',     0,   '?',   4,    0, },
{ &sdp_driver,  "sc",   5,  0,  0x0,0,  0,    '?',     0,   '?',   5,    0, },
{ &sdp_driver,  "sc",   6,  0,  0x0,0,  0,    '?',     0,   '?',   6,    0, },
{ &sdp_driver,  "sc",   7,  0,  0x0,0,  0,    '?',     0,   '?',   7,    0, },
#else
{ &sdp_driver,  "rz",   8,  0,  0x0,0,  0,    '?',     0,   '?',   0,    0, },
{ &sdp_driver,  "rz",   9,  0,  0x0,0,  0,    '?',     0,   '?',   1,    0, },
{ &sdp_driver,  "rz",  10,  0,  0x0,0,  0,    '?',     0,   '?',   2,    0, },
{ &sdp_driver,  "rz",  11,  0,  0x0,0,  0,    '?',     0,   '?',   3,    0, },
{ &sdp_driver,  "rz",  12,  0,  0x0,0,  0,    '?',     0,   '?',   4,    0, },
{ &sdp_driver,  "rz",  13,  0,  0x0,0,  0,    '?',     0,   '?',   5,    0, },
{ &sdp_driver,  "rz",  14,  0,  0x0,0,  0,    '?',     0,   '?',   6,    0, },
{ &sdp_driver,  "rz",  15,  0,  0x0,0,  0,    '?',     0,   '?',   7,    0, },

{ &sdp_driver,  "tz",   8,  0,  0x0,0,  0,    '?',     0,   '?',   0,    0, },
{ &sdp_driver,  "tz",   9,  0,  0x0,0,  0,    '?',     0,   '?',   1,    0, },
{ &sdp_driver,  "tz",  10,  0,  0x0,0,  0,    '?',     0,   '?',   2,    0, },
{ &sdp_driver,  "tz",  11,  0,  0x0,0,  0,    '?',     0,   '?',   3,    0, },
{ &sdp_driver,  "tz",  12,  0,  0x0,0,  0,    '?',     0,   '?',   4,    0, },
{ &sdp_driver,  "tz",  13,  0,  0x0,0,  0,    '?',     0,   '?',   5,    0, },
{ &sdp_driver,  "tz",  14,  0,  0x0,0,  0,    '?',     0,   '?',   6,    0, },
{ &sdp_driver,  "tz",  15,  0,  0x0,0,  0,    '?',     0,   '?',   7,    0, },

{ &sdp_driver,  "sc",   8,  0,  0x0,0,  0,    '?',     0,   '?',   0,    0, },
{ &sdp_driver,  "sc",   9,  0,  0x0,0,  0,    '?',     0,   '?',   1,    0, },
{ &sdp_driver,  "sc",  10,  0,  0x0,0,  0,    '?',     0,   '?',   2,    0, },
{ &sdp_driver,  "sc",  11,  0,  0x0,0,  0,    '?',     0,   '?',   3,    0, },
{ &sdp_driver,  "sc",  12,  0,  0x0,0,  0,    '?',     0,   '?',   4,    0, },
{ &sdp_driver,  "sc",  13,  0,  0x0,0,  0,    '?',     0,   '?',   5,    0, },
{ &sdp_driver,  "sc",  14,  0,  0x0,0,  0,    '?',     0,   '?',   6,    0, },
{ &sdp_driver,  "sc",  15,  0,  0x0,0,  0,    '?',     0,   '?',   7,    0, },
#endif
#endif /* NSDP > 0 */

#if NAIC > 0
#if AIC_UNIT == 0
{ &aic_driver,  "rz",   0,  0,  0x0,0,  0,    '?',     0,   '?',   0,    0, },
{ &aic_driver,  "rz",   1,  0,  0x0,0,  0,    '?',     0,   '?',   1,    0, },
{ &aic_driver,  "rz",  	2,  0,  0x0,0,  0,    '?',     0,   '?',   2,    0, },
{ &aic_driver,  "rz",  	3,  0,  0x0,0,  0,    '?',     0,   '?',   3,    0, },
{ &aic_driver,  "rz",  	4,  0,  0x0,0,  0,    '?',     0,   '?',   4,    0, },
{ &aic_driver,  "rz",  	5,  0,  0x0,0,  0,    '?',     0,   '?',   5,    0, },
{ &aic_driver,  "rz",  	6,  0,  0x0,0,  0,    '?',     0,   '?',   6,    0, },
{ &aic_driver,  "rz",  	7,  0,  0x0,0,  0,    '?',     0,   '?',   7,    0, },

{ &aic_driver,  "tz",   0,  0,  0x0,0,  0,    '?',     0,   '?',   0,    0, },
{ &aic_driver,  "tz",   1,  0,  0x0,0,  0,    '?',     0,   '?',   1,    0, },
{ &aic_driver,  "tz",   2,  0,  0x0,0,  0,    '?',     0,   '?',   2,    0, },
{ &aic_driver,  "tz",   3,  0,  0x0,0,  0,    '?',     0,   '?',   3,    0, },
{ &aic_driver,  "tz",   4,  0,  0x0,0,  0,    '?',     0,   '?',   4,    0, },
{ &aic_driver,  "tz",   5,  0,  0x0,0,  0,    '?',     0,   '?',   5,    0, },
{ &aic_driver,  "tz",   6,  0,  0x0,0,  0,    '?',     0,   '?',   6,    0, },
{ &aic_driver,  "tz",   7,  0,  0x0,0,  0,    '?',     0,   '?',   7,    0, },

{ &aic_driver,  "sc",   0,  0,  0x0,0,  0,    '?',     0,   '?',   0,    0, },
{ &aic_driver,  "sc",   1,  0,  0x0,0,  0,    '?',     0,   '?',   1,    0, },
{ &aic_driver,  "sc",   2,  0,  0x0,0,  0,    '?',     0,   '?',   2,    0, },
{ &aic_driver,  "sc",   3,  0,  0x0,0,  0,    '?',     0,   '?',   3,    0, },
{ &aic_driver,  "sc",   4,  0,  0x0,0,  0,    '?',     0,   '?',   4,    0, },
{ &aic_driver,  "sc",   5,  0,  0x0,0,  0,    '?',     0,   '?',   5,    0, },
{ &aic_driver,  "sc",   6,  0,  0x0,0,  0,    '?',     0,   '?',   6,    0, },
{ &aic_driver,  "sc",   7,  0,  0x0,0,  0,    '?',     0,   '?',   7,    0, },
#else
{ &aic_driver,  "rz",   8,  0,  0x0,0,  0,    '?',     0,   '?',   0,    0, },
{ &aic_driver,  "rz",   9,  0,  0x0,0,  0,    '?',     0,   '?',   1,    0, },
{ &aic_driver,  "rz",  10,  0,  0x0,0,  0,    '?',     0,   '?',   2,    0, },
{ &aic_driver,  "rz",  11,  0,  0x0,0,  0,    '?',     0,   '?',   3,    0, },
{ &aic_driver,  "rz",  12,  0,  0x0,0,  0,    '?',     0,   '?',   4,    0, },
{ &aic_driver,  "rz",  13,  0,  0x0,0,  0,    '?',     0,   '?',   5,    0, },
{ &aic_driver,  "rz",  14,  0,  0x0,0,  0,    '?',     0,   '?',   6,    0, },
{ &aic_driver,  "rz",  15,  0,  0x0,0,  0,    '?',     0,   '?',   7,    0, },

{ &aic_driver,  "tz",   8,  0,  0x0,0,  0,    '?',     0,   '?',   0,    0, },
{ &aic_driver,  "tz",   9,  0,  0x0,0,  0,    '?',     0,   '?',   1,    0, },
{ &aic_driver,  "tz",  10,  0,  0x0,0,  0,    '?',     0,   '?',   2,    0, },
{ &aic_driver,  "tz",  11,  0,  0x0,0,  0,    '?',     0,   '?',   3,    0, },
{ &aic_driver,  "tz",  12,  0,  0x0,0,  0,    '?',     0,   '?',   4,    0, },
{ &aic_driver,  "tz",  13,  0,  0x0,0,  0,    '?',     0,   '?',   5,    0, },
{ &aic_driver,  "tz",  14,  0,  0x0,0,  0,    '?',     0,   '?',   6,    0, },
{ &aic_driver,  "tz",  15,  0,  0x0,0,  0,    '?',     0,   '?',   7,    0, },

{ &aic_driver,  "sc",   8,  0,  0x0,0,  0,    '?',     0,   '?',   0,    0, },
{ &aic_driver,  "sc",   9,  0,  0x0,0,  0,    '?',     0,   '?',   1,    0, },
{ &aic_driver,  "sc",  10,  0,  0x0,0,  0,    '?',     0,   '?',   2,    0, },
{ &aic_driver,  "sc",  11,  0,  0x0,0,  0,    '?',     0,   '?',   3,    0, },
{ &aic_driver,  "sc",  12,  0,  0x0,0,  0,    '?',     0,   '?',   4,    0, },
{ &aic_driver,  "sc",  13,  0,  0x0,0,  0,    '?',     0,   '?',   5,    0, },
{ &aic_driver,  "sc",  14,  0,  0x0,0,  0,    '?',     0,   '?',   6,    0, },
{ &aic_driver,  "sc",  15,  0,  0x0,0,  0,    '?',     0,   '?',   7,    0, },
#endif
#endif /* NAIC > 0 */
0
};

volatile u_char 	*sdp;
volatile u_char		*sdpbuf;

volatile u_char		*aic;
volatile u_char		*aicbuf;

volatile u_char		*pc532_dma;
volatile u_char 	*pc532_dmaeop;
volatile xxx_curr_select = -42;
volatile xxx_prev_select = -42;

extern vm_offset_t io_map();
extern int iunit[];

initialize_devices()
{
  /* The dp8490 depends on the pc532_dmaeop space being contiguous
   * withthe pc532_dma space
   */
	pc532_dma = (u_char *) io_map((vm_offset_t) PC532_SCSI_DMABASE,
				PAGE_SIZE * 2);
  
	pc532_dmaeop = (u_char *) io_map((vm_offset_t) PC532_SCSI_DMAEOP,
				   PAGE_SIZE);
  
#if NSDP > 0
	/* scsi chip registers. Which scsi depends on ICU (SCSI_SELECT) */
	/* wired & uncached */
	sdp = (u_char *) io_map((vm_offset_t) PC532_SCSI_BASE, PAGE_SIZE);

	/* command buffer. doesn't need to be uncacheable */
	if (kmem_alloc_wired(kernel_map, &sdpbuf, SDP_RAM_SIZE) !=KERN_SUCCESS)
	    panic("initialize_devices");

	iunit[4] = sdp_unit;	/* 4XXX */
	sdp_buffer_init(sdp_unit, sdpbuf); /* initialize command buffer */
	configure_bus_master("sdp", sdp, 0, sdp_unit, "slot");
#endif /* NSDP > 0 */

#if NAIC > 0
	aic = (u_char *)io_map((vm_offset_t)PC532_SCSI_BASE, PAGE_SIZE);

	/* command buffer. doesn't need to be uncacheable */
	if (kmem_alloc_wired(kernel_map, &aicbuf, AIC_RAM_SIZE) !=KERN_SUCCESS)
	    panic("initialize_devices (aic buffer)");

	/* aic_buffer_init()
	 *
	 * The AIC  parameter is IN/OUT, it will be changed to a pointer
	 * to a software control register for AIC_UNIT'th controller.
	 *
	 * In addition, initialize command buffer.
	 */
	iunit[5] = aic_unit;	/* 5XXX */
  	aic_buffer_init(&aic, aic_unit, aicbuf);
	configure_bus_master("aic", aic, 0, aic_unit, "slot");

#endif /* NAIC > 0 */

#if 0	/* not yet */
#define DUART_BASE 0x28000000
	configure_bus_master("com", 0, DUART_BASE, 1, "uarts");
#endif 0
	cominit();

}

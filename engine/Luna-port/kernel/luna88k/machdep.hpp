/*
 * Mach Operating System
 * Copyright (c) 1993 Carnegie Mellon University
 * All Rights Reserved.
 *
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 *
 * CARNEGIE MELLON AND OMRON ALLOW FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON AND OMRON DISCLAIM ANY LIABILITY OF ANY KIND
 * FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
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
 * HISTORY
 * $Log:	machdep.h,v $
 * Revision 2.5  93/11/17  17:36:44  dbg
 * 	Removed prototypes for functions that are now exported by other
 * 	(mostly machine-independent) header files.  Added 'extern' to
 * 	remaining prototypes.
 * 	[93/11/04            dbg]
 * 
 * Revision 2.4  93/05/17  10:24:31  rvb
 * 	Type casts, etc to quiet gcc 2.3.3 warnings
 * 
 * Revision 2.3  93/03/09  17:58:00  danner
 * 	Added prototype for bootarg()
 * 	[93/02/25            jfriedl]
 * 
 * Revision 2.2  93/01/26  18:02:11  danner
 * 	Created.
 * 	[93/01/24            jfriedl]
 * 
 */

#ifndef __LUNA88K_MACHDEP_H__
#define __LUNA88K_MACHDEP_H__

/* 
 ** This file contains prototypes and other decs. for
 ** "luna88k/machdep.c", the assembley files in "luna88k/",
 ** as well as a few misc C files in "luna88k/".
  */

#include <mach_kdb.h>           	/* MACH_KDB */
#include <mach/m88k/vm_types.h>	  	/* vm_offset_t */
#include <kern/lock.h>			/* simple_lock_t */
#include <motorola/m88k/cpu_number.h>	/* cpu_number */
#include <m88k/asm_macro.h>        	/* disable_interrupts(), etc. */
#include <m88k/setjmp.h>		/* jump_buf_t */
#include <m88k/board.h>			/* MAX_CPUS */
#include <kern/kern_types.h>		/* thread_t, task_t, etc. */
#include <mach/m88k/kern_return.h>	/* kern_return_t */

/* from "luna88k/machdep.c" */
extern volatile unsigned int *int_mask_reg[MAX_CPUS];
extern volatile int scpus;

#ifndef __IS_MACHDEP_C__
  #define MACHDEP_STATIC static
#else
  #define MACHDEP_STATIC /* make a copy for everyone */
#endif

MACHDEP_STATIC inline unsigned spl(void)
{
    return *int_mask_reg[cpu_number()]<<8;
}

/* from "luna88k/machdep.c" */
unsigned spln(int level);

MACHDEP_STATIC inline unsigned spl0(void)
{
    int i = spln(0);
    enable_interrupt();
    return i;
}

#define spl1()		spln(1)
#define spl2()		spln(2)
#define spl3()		spln(3)
#define spl4()		spln(4)
#define spl5()		spln(5)
#define spl6()		spln(6)
#define spl7()		spln(7)
#define splsoftclock()	spln(1)
#define splbio()	spln(3)
#define splimp()	spln(4)
#define splvm()		spln(5)
#define splclock()	spln(6)
#define spltty()	spln(5)
#define splhigh()	spln(6)
#define splsched()	spln(6)

/*
 * Prototypes from "luna88k/machdep.h"
 */
extern void (*scb_level3[])();
extern void level3_intr(void);
extern unsigned spl_mask(int level);
extern unsigned splx(unsigned mask);
extern unsigned check_memory(void *addr, unsigned flag);
extern void start_clock(void);
extern void pre_ext_int(unsigned vec, unsigned *eframe);
extern void softint_on(int cpu);
extern void init_ast_check(processor_t processor);
extern void cause_ast_check(processor_t processor);
extern vm_offset_t get_slave_stack(void);
extern void load_context(thread_t thread);
extern vm_offset_t phystokv(vm_offset_t phys);
extern int wprobe(void *addr, unsigned int write);
extern void bm_set_direct_framebuffer_access(void);
extern void bm_align_screen(void);
extern void bm_set_palette_colors(
    unsigned char old[2/*fg and bg*/][3/*r g b*/],
    unsigned fg_R,
    unsigned fg_G,
    unsigned fg_B,
    unsigned bg_R,
    unsigned bg_G,
    unsigned bg_B);
extern void nvram_read(char *buf, vm_offset_t address, unsigned size);
extern void nvram_write(char *buf, vm_offset_t address, unsigned int size);

#if MACH_KDB
 extern unsigned db_spl(void);
 extern unsigned db_splx(unsigned mask);
 extern unsigned db_splhigh(void);
#endif

/*
 * Prototypes from "luna88k/locore.s"
 */
extern volatile unsigned initialized_cpus;
extern simple_lock_data_t inter_processor_lock;
extern int intstack[];
extern int boothowto;

extern void thread_bootstrap(void);
extern unsigned measure_pause(volatile int *flag);
extern void delay_in_microseconds(unsigned count);
extern void delay(unsigned count);


/*
 * Prototypes from "luna88k/locore_asm_routines.s"
 */
extern unsigned do_load_word(
	vm_offset_t address,
	boolean_t supervisor_mode,
	boolean_t little_endian);
extern unsigned do_load_half(
	vm_offset_t address,
	boolean_t supervisor_mode,
	boolean_t little_endian);
extern unsigned do_load_byte(
	vm_offset_t address,
	boolean_t supervisor_mode,
	boolean_t little_endian);
extern unsigned do_store_word(
	vm_offset_t address,
	unsigned data,
	boolean_t supervisor_mode,
	boolean_t little_endian);
extern unsigned do_store_half(
	vm_offset_t address,
	unsigned data,
	boolean_t supervisor_mode,
	boolean_t little_endian);
extern unsigned do_store_byte(
	vm_offset_t address,
	unsigned data,
	boolean_t supervisor_mode,
	boolean_t little_endian);
extern unsigned do_xmem_word(
	vm_offset_t address,
	unsigned data,
	boolean_t supervisor_mode,
	boolean_t little_endian);
extern unsigned do_xmem_byte(
	vm_offset_t address,
	unsigned data,
	boolean_t supervisor_mode,
	boolean_t little_endian);
extern void set_cpu_number(unsigned number);
extern void set_current_thread(thread_t thread);

/*	These cause too many conflicts
 *
 * Prototypes from "luna88k/eh.s"

boolean_t badaddr(vm_offset_t addr, unsigned len);
boolean_t badwordaddr(vm_offset_t addr);
*/
extern void ovbcopy(void *source, void *dest, unsigned count);
extern void blkclr(void *addr, unsigned size);
extern volatile void longjmp_int_enable(jmp_buf_t *buf, int value);
extern unsigned read_processor_identification_register(void);
extern void call_rom_abort(unsigned level, unsigned *);


/*
 * Prototypes from "luna88k/rawprint_asm_routines.s"
 */
#ifdef RAW_PRINTF
extern void raw_putstr(char *str);
extern void raw_putchar(char c);
#endif

/*
 * Prototypes from "luna88k/rawprint_c_routines.c"
 */
extern int raw_vsnprintf();
extern void raw_printf(const char *fmt, ...)
    __attribute__ ((format (printf,1,2)));

#ifdef MACH_KDB
extern void db_simple_unlock(simple_lock_t lock);
extern void db_simple_lock(simple_lock_t lock);
extern unsigned db_simple_lock_try(simple_lock_t lock);
extern unsigned db_simple_lock_held(simple_lock_t lock);
extern unsigned db_are_interrupts_disabled(void);
#endif


/*
 * Prototypes from "luna88k/locore_c_routines.c"
 */
extern void data_access_emulation(unsigned *eframe);

/*
 * Prototypes from "luna88k/locore_c_routines.c"
 */
extern char *bootarg(char *name);

#endif /* __LUNA88K_MACHDEP_H__ */



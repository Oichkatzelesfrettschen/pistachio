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
 * $Log:	stdarg.h,v $
 * Revision 2.2  93/11/17  19:08:52  dbg
 * 	Added to kernel.
 * 	[93/11/04            dbg]
 * 
 * Revision 2.2  92/08/03  17:52:48  jfriedl
 * 	Installed from gcc 2.1 release [danner].
 * 	[92/07/24            jfriedl]
 * 
 */

#ifndef	_M88K_STDARG_H_
#define	_M88K_STDARG_H_

/* This file contains changes made by Data General, December 1989.  */
/* GNU C varargs support for the Motorola 88100  */

typedef struct
{
  int  __va_arg;		/* argument number */
  int *__va_stk;		/* start of args passed on stack */
  int *__va_reg;		/* start of args passed in regs */
} va_list;


/* stdarg.h support */

#if __GNUC__ > 1 /* GCC 2.0 and beyond */
#define va_start(AP,LASTARG) ((AP) = *(va_list *)__builtin_saveregs())
#else
#define va_start(AP,LASTARG) \
  ( (AP).__va_reg = (int *) __builtin_saveregs2(0), \
    (AP).__va_stk = (int *) __builtin_argptr(), \
    (AP).__va_arg = (int) (__builtin_argsize() + 3) / 4 )
#endif

#define __va_reg_p(TYPE) \
  (__builtin_classify_type(*(TYPE *)0) < 12 \
   ? sizeof(TYPE) <= 8 : sizeof(TYPE) == 4 && __alignof__(TYPE) == 4)

#define	__va_size(TYPE) ((sizeof(TYPE) + 3) >> 2)

#define va_arg(AP,TYPE) \
  ( (AP).__va_arg = (((AP).__va_arg + (1 << (__alignof__(TYPE) >> 3)) - 1) \
		     & ~((1 << (__alignof__(TYPE) >> 3)) - 1)) \
    + __va_size(TYPE), \
    *((TYPE *) ((__va_reg_p(TYPE) && (AP).__va_arg < 8 + __va_size(TYPE) \
		 ? (AP).__va_reg : (AP).__va_stk) \
		+ ((AP).__va_arg - __va_size(TYPE)))))

#define va_end(AP)

#endif	/* _M88K_STDARG_H_ */

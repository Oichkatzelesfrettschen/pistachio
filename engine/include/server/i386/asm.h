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
 * $Log:	asm.h,v $
 * Revision 2.1  92/04/21  17:18:59  rwd
 * BSDSS
 * 
 *
 */

#define S_ARG0	 4(%esp)
#define S_ARG1	 8(%esp)
#define S_ARG2	12(%esp)
#define S_ARG3	16(%esp)

#define FRAME	pushl %ebp; movl %esp, %ebp
#define EMARF	leave

#define B_ARG0	 8(%ebp)
#define B_ARG1	12(%ebp)
#define B_ARG2	16(%ebp)
#define B_ARG3	20(%ebp)

#ifdef	wheeze

#define ALIGN 4
#define EXT(x) x
#define LCL(x) ./**/x

#define LB(x,n) ./**/x
#define LBb(x,n) ./**/x
#define LBf(x,n) ./**/x

#define	SVC lcall $7,$0

#define String .string
#define Value  .value
#define Times(a,b) [a\*b]
#define Divide(a,b) [a\\b]

#define INB	inb	(%dx)
#define OUTB	outb	(%dx)
#define INL	inl	(%dx)
#define OUTL	outl	(%dx)

#else	wheeze

#define ALIGN 2
#define EXT(x) _/**/x
#define	LCL(x)	x

#define LB(x,n) n
#define LBb(x,n) n/**/b
#define LBf(x,n) n/**/f

#define SVC .byte 0x9a; .long 0; .word 0x7

#define String	.ascii
#define Value	.word
#define Times(a,b) (a*b)
#define Divide(a,b) (a/b)

#define INB	inb	%dx, %al
#define OUTB	outb	%al, %dx
#define INL	inl	%dx, %eax
#define OUTL	outl	%eax, %dx

#endif	wheeze

#define data16	.byte 0x66
#define addr16	.byte 0x67



#ifdef GPROF
#define MCOUNT		.data; LB(x, 9): .long 0; .text; lea LBb(x, 9),%edx; call mcount
#define	ENTRY(x)	.globl EXT(x); .align ALIGN; EXT(x): ; \
			pushl %ebp; movl %esp, %ebp; MCOUNT; popl %ebp;
#define	ASENTRY(x) 	.globl x; .align ALIGN; x: ; \
			pushl %ebp; movl %esp, %ebp; MCOUNT; popl %ebp;
#else	GPROF
#define MCOUNT
#define	ENTRY(x)	.globl EXT(x); .align ALIGN; EXT(x):
#define	ASENTRY(x)	.globl x; .align ALIGN; x:
#endif	GPROF

#define	Entry(x)	.globl EXT(x); .align ALIGN; EXT(x):
#define	DATA(x)		.globl EXT(x); .align ALIGN; EXT(x):

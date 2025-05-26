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
 * 11-May-92  Tatu Ylonen (ylo) at Helsinki University of Technology
 *	Created from libmach/i386/crt0.c.
 * $Log:$
 */
/*
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * William Jolitz.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that: (1) source distributions retain this entire copyright
 * notice and comment, and (2) distributions including binaries display
 * the following acknowledgement:  ``This product includes software
 * developed by the University of California, Berkeley and its contributors''
 * in the documentation or other materials provided with the distribution
 * and in all advertising materials mentioning features or use of this
 * software. Neither the name of the University nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint
static char sccsid[] = "@(#)crt0.c	5.2 (Berkeley) 5/14/90";
#endif /* not lint */

/*
 *	C start up routine.
 *	Robert Henry, UCB, 20 Oct 81
 *
 *	We make the following (true) assumptions:
 *	1) when the kernel calls start, it does a jump to location 2,
 *	and thus avoids the register save mask.  We are NOT called
 *	with a calls!  see sys1.c:setregs().
 *	2) The only register variable that we can trust is sp,
 *	which points to the base of the kernel calling frame.
 *	Do NOT believe the documentation in exec(2) regarding the
 *	values of fp and ap.
 *	3) We can allocate as many register variables as we want,
 *	and don't have to save them for anybody.
 *	4) Because of the ways that asm's work, we can't have
 *	any automatic variables allocated on the stack, because
 *	we must catch the value of sp before any automatics are
 *	allocated.
 */

#include <machine/asm.h>

char **environ = (char **)0;
#ifdef paranoid
static int fd;
#endif paranoid

#if	MACH
int	(*mach_init_routine)();
int	(*_cthread_init_routine)();
void	(*_cthread_exit_routine)();
int	(*_StrongBox_init_routine)();
int	errno = 0;
void	exit();
#endif	MACH

extern	unsigned char	etext;
extern	unsigned char	_eprol;

char **_start_sp;

asm("   .text");
asm("	.globl	__start");
#if 0
asm("bpt");
#endif
asm("__start:	sprd	sp,tos");
asm("	bsr	__start2");
_start2(sp)
char **sp;
{
	char **argv;
	int argc;

	_start_sp=sp;
	argv=sp+1;
	argc=(int)*sp;
	environ=(char **)&sp[argc+2];
	
#if	MACH
	if (mach_init_routine)
		(void) mach_init_routine();
#endif	MACH
asm("__eprol:");

#ifdef paranoid
	/*
	 * The standard I/O library assumes that file descriptors 0, 1, and 2
	 * are open. If one of these descriptors is closed prior to the start 
	 * of the process, I/O gets very confused. To avoid this problem, we
	 * insure that the first three file descriptors are open before calling
	 * main(). Normally this is undefined, as it adds two unnecessary
	 * system calls.
	 */
	do	{
		fd = open("/dev/null", 2);
	} while (fd >= 0 && fd < 3);
	close(fd);
#endif paranoid

#ifdef MCRT0
	monstartup(&_eprol, &etext);
#endif MCRT0
#if	MACH
	if (_cthread_init_routine) {
	    int new_sp;
	    new_sp = (*_cthread_init_routine)();
	    if (new_sp) {
	      asm volatile("lprd sp,%0" : : "g" (new_sp) );
	    }
	}
	if (_StrongBox_init_routine) (*_StrongBox_init_routine)();
	(* (_cthread_exit_routine ? _cthread_exit_routine : exit))
		(main(argc,argv,environ));
#else	MACH
	exit(main(argc,argv,environ));
#endif	MACH
}

#ifdef MCRT0
/*ARGSUSED*/
exit(code)
	register int code;
{
	monitor(0);
	_cleanup();
	_exit(code);
}
#endif MCRT0

#ifdef CRT0
/*
 * null mcount and moncontrol,
 * just in case some routine is compiled for profiling
 */
moncontrol(val)
	int val;
{

}
asm("	.globl	_mcount");
asm("_mcount:	ret 0");
#endif CRT0

/*-
 * Copyright (c) 1982, 1986, 1993
 *	The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)tty.h	8.6 (Berkeley) 1/21/94
 */

#pragma once

#ifdef LITES
#include <serv/import_mach.h>
#include <device/tty_status.h>
#endif

#include <sys/termios.h>
#include <sys/select.h>		/* For struct selinfo. */

/*
 * Clists are character lists, which is a variable length linked list
 * of cblocks, with a count of the number of characters in the list.
 */
struct clist {
	int	c_cc;		/* Number of characters in the clist. */
	char	*c_cf;		/* Pointer to the first cblock. */
	char	*c_cl;		/* Pointer to the last cblock. */
};

/*
 * Per-tty structure.
 *
 * Should be split in two, into device and tty drivers.
 * Glue could be masks of what to echo and circular buffer
 * (low, high, timeout).
 */
struct tty {
	struct	clist t_rawq;		/* Device raw input queue. */
	long	t_rawcc;		/* Raw input queue statistics. */
	struct	clist t_canq;		/* Device canonical queue. */
	long	t_cancc;		/* Canonical queue statistics. */
	struct	clist t_outq;		/* Device output queue. */
	long	t_outcc;		/* Output queue statistics. */
	char	t_line;			/* Interface to device drivers. */
	dev_t	t_dev;			/* Device. */
	int	t_state;		/* Device and driver (TS*) state. */
	int	t_flags;		/* Tty flags. */
	struct	pgrp *t_pgrp;		/* Foreground process group. */
	struct	session *t_session;	/* Enclosing session. */
	struct	selinfo t_rsel;		/* Tty read/oob select. */
	struct	selinfo t_wsel;		/* Tty write select. */
	struct	termios t_termios;	/* Termios state. */
	struct	winsize t_winsize;	/* Window size. */
					/* Start output. */
	void	(*t_oproc) (struct tty *);
					/* Stop output. */
	void	(*t_stop) (struct tty *, int);
					/* Set hardware state. */
	int	(*t_param) (struct tty *, struct termios *);
	void	*t_sc;			/* XXX: net/if_sl.c:sl_softc. */
	short	t_column;		/* Tty output column. */
	short	t_rocount, t_rocol;	/* Tty. */
	short	t_hiwat;		/* High water mark. */
	short	t_lowat;		/* Low water mark. */
	short	t_gen;			/* Generation number. */

	/* Lites stuff */
	mach_port_t	t_device_port;
	mach_port_t	t_reply_port;
};

#define	t_cc		t_termios.c_cc
#define	t_cflag		t_termios.c_cflag
#define	t_iflag		t_termios.c_iflag
#define	t_ispeed	t_termios.c_ispeed
#define	t_lflag		t_termios.c_lflag
#define	t_min		t_termios.c_min
#define	t_oflag		t_termios.c_oflag
#define	t_ospeed	t_termios.c_ospeed
#define	t_time		t_termios.c_time

#define	TTIPRI	25			/* Sleep priority for tty reads. */
#define	TTOPRI	26			/* Sleep priority for tty writes. */

#define	TTMASK	15
#define	OBUFSIZ	100
#define	TTYHOG	1024

#ifdef KERNEL
#define	TTMAXHIWAT	roundup(2048, CBSIZE)
#define	TTMINHIWAT	roundup(100, CBSIZE)
#define	TTMAXLOWAT	256
#define	TTMINLOWAT	32
#endif

/* These flags are kept in t_state. */
#define	TS_ASLEEP	0x00001		/* Process waiting for tty. */
#define	TS_ASYNC	0x00002		/* Tty in async I/O mode. */
#define	TS_BUSY		0x00004		/* Draining output. */
#define	TS_CARR_ON	0x00008		/* Carrier is present. */
#define	TS_FLUSH	0x00010		/* Outq has been flushed during DMA. */
#define	TS_ISOPEN	0x00020		/* Open has completed. */
#define	TS_TBLOCK	0x00040		/* Further input blocked. */
#define	TS_TIMEOUT	0x00080		/* Wait for output char processing. */
#define	TS_TTSTOP	0x00100		/* Output paused. */
#define	TS_WOPEN	0x00200		/* Open in progress. */
#define	TS_XCLUDE	0x00400		/* Tty requires exclusivity. */

/* State for intra-line fancy editing work. */
#define	TS_BKSL		0x00800		/* State for lowercase \ work. */
#define	TS_CNTTB	0x01000		/* Counting tab width, ignore FLUSHO. */
#define	TS_ERASE	0x02000		/* Within a \.../ for PRTRUB. */
#define	TS_LNCH		0x04000		/* Next character is literal. */
#define	TS_TYPEN	0x08000		/* Retyping suspended input (PENDIN).*/

#define TS_ONDELAY	0x10000
#define TS_RQUEUED	0x20000

#define	TS_LOCAL	(TS_BKSL | TS_CNTTB | TS_ERASE | TS_LNCH | TS_TYPEN)

/* Character type information. */
#define	ORDINARY	0
#define	CONTROL		1
#define	BACKSPACE	2
#define	NEWLINE		3
#define	TAB		4
#define	VTAB		5
#define	RETURN		6

struct speedtab {
	int sp_speed;			/* Speed. */
	int sp_code;			/* Code. */
};

/* Modem control commands (driver). */
#define	DMSET		0
#define	DMBIS		1
#define	DMBIC		2
#define	DMGET		3

/* Flags on a character passed to ttyinput. */
#define	TTY_CHARMASK	0x000000ff	/* Character mask */
#define	TTY_QUOTE	0x00000100	/* Character quoted */
#define	TTY_ERRORMASK	0xff000000	/* Error mask */
#define	TTY_FE		0x01000000	/* Framing error or BREAK condition */
#define	TTY_PE		0x02000000	/* Parity error */

/* Is tp controlling terminal for p? */
#define	isctty(p, tp)							\
	((p)->p_session == (tp)->t_session && (p)->p_flag & P_CONTROLT)

/* Is p in background of tp? */
#define	isbackground(p, tp)						\
	(isctty((p), (tp)) && (p)->p_pgrp != (tp)->t_pgrp)

#ifdef KERNEL
extern	struct ttychars ttydefaults;

/* Symbolic sleep message strings. */
extern	 char ttyin[], ttyout[], ttopen[], ttclos[], ttybg[], ttybuf[];

int	 b_to_q (char *cp, int cc, struct clist *q);
void	 catq (struct clist *from, struct clist *to);
void	 clist_init (void);
int	 getc (struct clist *q);
void	 ndflush (struct clist *q, int cc);
int	 ndqb (struct clist *q, int flag);
char	*nextc (struct clist *q, char *cp, int *c);
int	 putc (int c, struct clist *q);
int	 q_to_b (struct clist *q, char *cp, int cc);
int	 unputc (struct clist *q);

int	 nullmodem (struct tty *tp, int flag);
int	 tputchar (int c, struct tty *tp);
int	 ttioctl (struct tty *tp, ioctl_cmd_t com, void *data, int flag);
int	 ttread (struct tty *tp, struct uio *uio, int flag);
void	 ttrstrt (void *tp);
int	 ttselect (dev_t device, int rw, struct proc *p);
void	 ttsetwater (struct tty *tp);
int	 ttspeedtab (int speed, struct speedtab *table);
int	 ttstart (struct tty *tp);
void	 ttwakeup (struct tty *tp);
int	 ttwrite (struct tty *tp, struct uio *uio, int flag);
void	 ttychars (struct tty *tp);
int	 ttycheckoutq (struct tty *tp, int wait);
int	 ttyclose (struct tty *tp);
void	 ttyflush (struct tty *tp, int rw);
void	 ttyinfo (struct tty *tp);
int	 ttyinput (int c, struct tty *tp);
int	 ttylclose (struct tty *tp, int flag);
int	 ttymodem (struct tty *tp, int flag);
int	 ttyopen (dev_t device, struct tty *tp);
int	 ttyoutput (int c, struct tty *tp);
void	 ttypend (struct tty *tp);
void	 ttyretype (struct tty *tp);
void	 ttyrub (int c, struct tty *tp);
int	 ttysleep (struct tty *tp,
	    void *chan, int pri, char *wmesg, int timeout);
int	 ttywait (struct tty *tp);
int	 ttywflush (struct tty *tp);
#endif


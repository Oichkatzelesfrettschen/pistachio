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
 * 11-May-92  Johannes Helander (jvh) at Helsinki University of Technology
 *	Created from i386 BSDSS.
 *
 * $Log:	conf.c,v $
 */
 
/*
 * Simplified configuration.
 */
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/buf.h>	/* for B_TAPE */
#include <sys/conf.h>
#include <sys/errno.h>

#include <uxkern/device.h>
#include <uxkern/device_utils.h>

#include <sys/ioctl.h>	/* for tty */
#include <sys/tty.h>

extern int	nulldev();
extern int	enodev();
extern struct tty *nulltty();


/*
 * Block devices all use the same open/close/strategy routines.
 */
extern int	bdev_open(), bdev_close(), bio_strategy(), bdev_ioctl();
extern int	bdev_dump(), bdev_size();

#define	bdev_ops	bdev_open, bdev_close, bio_strategy, bdev_ioctl,\
			bdev_dump, bdev_size

struct bdevsw	bdevsw[] =
{
/*0*/	{ "",		0,		bdev_ops },
/*1*/   { "",           0,              bdev_ops },
/*2*/	{ "",		0,		bdev_ops },
/*3*/	{ "",		0,		bdev_ops },
/*4*/   { "",           0,              bdev_ops },
/*5*/	{ "sd",		C_BLOCK(16),	bdev_ops },

};
int	nblkdev = sizeof(bdevsw)/sizeof(bdevsw[0]);

extern int	char_open(), char_close(), char_read(), char_write();
extern int	char_ioctl(), char_select();
#define	char_ops \
	char_open, char_close, char_read, char_write, char_ioctl, \
	char_select, nulldev, nulltty

extern int	disk_open(), disk_close(), disk_read(), disk_write();
extern int	disk_ioctl();
#define	disk_ops \
	disk_open, disk_close, disk_read, disk_write, disk_ioctl, \
	seltrue, nulldev, nulltty

extern int	tty_open(), tty_close(), tty_read(), tty_write();
extern int	tty_ioctl(), ttselect(), tty_stop();
extern struct tty *tty_find_tty();
#define	tty_ops	\
	tty_open, tty_close, tty_read, tty_write, tty_ioctl, ttselect, \
	tty_stop, tty_find_tty

extern int	cons_open(), cons_write(), cons_ioctl();
extern mach_port_t	cons_port();

#define	console_ops	\
	cons_open, tty_close, tty_read, cons_write, cons_ioctl, ttselect, \
	tty_stop, tty_find_tty, cons_port

extern int	cttyopen(), cttyread(), cttywrite(), cttyioctl(), cttyselect();
#define	ctty_ops \
 	cttyopen, nulldev, cttyread, cttywrite, cttyioctl, cttyselect, \
 	nulldev, nulltty

extern int	logopen(), logclose(), logread(), logioctl(), logselect();
#define	log_ops \
	logopen,  logclose,  logread,  nulldev,   logioctl,  logselect, \
	nulldev, nulltty

extern int	mmopen(), mmread(), mmwrite();
#define	mm_ops \
	mmopen,   nulldev,   mmread,   mmwrite,   enodev,     seltrue,  \
	nulldev, nulltty

extern int	ptsopen(), ptsclose(), ptsread(), ptswrite();
extern int	ptyioctl(), ptsstop();
#define	pts_ops \
	ptsopen,  ptsclose,  ptsread,  ptswrite,  ptyioctl,  ttselect, \
	ptsstop, nulltty

extern int	ptcopen(), ptcclose(), ptcread(), ptcwrite();
extern int	ptcselect();
#define	ptc_ops \
	ptcopen,  ptcclose,  ptcread,  ptcwrite,  ptyioctl,  ptcselect, \
	nulldev, nulltty

#define	no_ops \
	enodev, enodev, enodev, enodev, enodev, enodev, nulldev, nulltty

struct cdevsw	cdevsw[] =
{
/*0*/	{ "console",	0,		console_ops,	},
/*1*/	{ "",		0,		ctty_ops,	},	/* tty */
/*2*/	{ "",		0,		mm_ops,		},	/* kmem,null */
/*3*/	{ "",		0,		no_ops,		},	/* isa */
/*4*/	{ "",		0,		no_ops,		},      /* raw */
/*5*/	{ "",		0,		pts_ops,	},
/*6*/	{ "",		0,		ptc_ops,	},
/*7*/	{ "",		0,		log_ops,	},      /* log */
/*8*/	{ "com",	0,		tty_ops,	},	/* isa */
/*9*/	{ "",		0,		no_ops,		},	/* isa */
/*10*/	{ "",		0,		no_ops,		},	/* isa */
/*11*/	{ "",		0,		no_ops,		},      /* xd */
/*12*/	{ "",		0,		no_ops,		},      /* pc */
/*13*/	{ "sd",		C_BLOCK(16),	disk_ops,	},	/* isa */
};
int	nchrdev = sizeof(cdevsw)/sizeof(cdevsw[0]);

dev_t	cttydev = makedev(1, 0);
int	mem_no = 2;

/*
 * Conjure up a name string for funny devices (not all minors have
 * the same name).
 */
int
check_dev(dev, str)
	dev_t	dev;
	char	*str;
{
}


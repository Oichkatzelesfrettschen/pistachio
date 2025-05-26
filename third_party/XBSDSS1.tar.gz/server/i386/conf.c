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
 * $Log:	conf.c,v $
 * Revision 2.4  92/07/08  16:17:04  mrt
 * 	Remove incorrect declaration of seltrue. It is declared
 * 	in sys/systm.h
 * 	[92/07/02            mrt]
 * 
 * Revision 2.3  92/05/28  17:41:41  rwd
 * 	Fix bdevsw "sd" inconsistancy.
 * 	[92/05/28            rwd]
 * 
 * Revision 2.2  92/05/25  14:42:07  rwd
 * 	Added scsi disk "sd".
 * 	[92/05/21            rwd]
 * 
 * Revision 2.1  92/04/21  17:19:07  rwd
 * BSDSS
 * 
 *
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
/*0*/	{ "hd",		C_BLOCK(16),	bdev_ops },	/* isa */
/*1*/   { "",           0,              bdev_ops },
/*2*/	{ "fd",		C_BLOCK(64),	bdev_ops },	/* isa */
/*3*/	{ "wt",		B_TAPE,		bdev_ops },	/* isa */
/*4*/   { "",           0,              bdev_ops },
/*5*/	{ "sd",		C_BLOCK(16),	bdev_ops },	/* ipsc, isa */

};
int	nblkdev = sizeof(bdevsw)/sizeof(bdevsw[0]);

extern int	char_open(), char_close(), char_read(), char_write();
extern int	char_ioctl(), char_select();
#define	char_ops \
	char_open, char_close, char_read, char_write, char_ioctl, \
	char_select, nulldev, nulltty

extern int	disk_open(), disk_close(), disk_read(), disk_write();
extern int	disk_ioctl();
int		isa_disk_ioctl();	/* forward */
#define	disk_ops \
	disk_open, disk_close, disk_read, disk_write, disk_ioctl, \
	seltrue, nulldev, nulltty

#define	isa_disk_ops \
	disk_open, disk_close, disk_read, disk_write, isa_disk_ioctl, \
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
/*1*/	{ "",		0,		ctty_ops,		},	/* tty */
/*2*/	{ "",		0,		mm_ops,		},	/* kmem,null */
/*3*/	{ "hd",		C_BLOCK(16),	isa_disk_ops,	},	/* isa */
/*4*/	{ "",		0,		no_ops,		},      /* raw */
/*5*/	{ "",		0,		pts_ops,	},
/*6*/	{ "",		0,		ptc_ops,	},
/*7*/	{ "",		0,		log_ops,	},      /* log */
/*8*/	{ "com",	0,		tty_ops,	},	/* isa */
/*9*/	{ "fd",		C_BLOCK(64),	isa_disk_ops,	},	/* isa */
/*10*/	{ "wt",		0,		char_ops,	},	/* isa */
/*11*/	{ "",		0,		no_ops,		},      /* xd */
/*12*/	{ "",		0,		no_ops,		},      /* pc */
/*13*/	{ "sd",		C_BLOCK(16),	isa_disk_ops,	},	/* isa */
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

/*
 * ISA disk IOCTL
 */
#undef	p_flag			/* conflict from sys/{proc,user}.h */
#include <i386at/disk.h>

extern mach_port_t	disk_port();

int
isa_disk_ioctl(dev, cmd, data, flag)
	dev_t	dev;
	int	cmd;
	caddr_t	data;
	int	flag;
{
	mach_port_t	device_port = disk_port(dev);
	unsigned int	count;
	register int	error;

	switch (cmd) {
	    case V_RDABS:
	    {
		char buf[512];

		error = device_set_status(device_port,
					  V_ABS,
					  &((struct absio *)data)->abs_sec,
					  1);
		if (error)
		    return (dev_error_to_errno(error));
		count = 512/sizeof(int);
		error = device_get_status(device_port,
					  cmd,
					  buf,
					  &count);

		if (error)
		    return (dev_error_to_errno(error));
		if (copyout(buf, ((struct absio *)data)->abs_buf, 512))
		    return (EFAULT);
		break;
	    }

	    case V_VERIFY:
	    {
		union vfy_io *vfy_io = (union vfy_io *) data;
		int vfy[2] = {	vfy_io->vfy_in.abs_sec,
				vfy_io->vfy_in.num_sec};

		error = device_set_status(device_port,
					  V_ABS,
					  vfy,
					  2);
		if (error)
		    return (dev_error_to_errno(error));
		count = sizeof (int)/sizeof(int);
		error = device_get_status(device_port,
					  cmd,
					  vfy,
					  &count);

		vfy_io->vfy_out.err_code = vfy[0];
		if (error)
		    return (dev_error_to_errno(error));
		break;
	    }

	    case V_WRABS:
	    {
		char buf[512];

		error = device_set_status(device_port,
					  V_ABS,
					  &((struct absio *)data)->abs_sec,
					  1);
		if (error)
		    return (dev_error_to_errno(error));
		if (copyin(((struct absio *)data)->abs_buf, buf, 512))
		    return (EFAULT);
		count = 512/sizeof(int);
		error = device_set_status(device_port,
					  cmd,
					  buf,
					  count);
		if (error)
		    return (dev_error_to_errno(error));
		break;
	    }

	    default:
	    {
		return (disk_ioctl(dev, cmd, data, flag));
	    }
	}
	return (0);
}

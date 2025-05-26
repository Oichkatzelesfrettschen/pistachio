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
 * 15-Apr-93  Ian Dall (ian) at University of Adelaide
 *	Added include of vm_types.h. conf.h needs it.
 *
 * 11-May-92  Johannes Helander (jvh) at Helsinki University of Technology
 *	Created.
 *
 * $Log: conf.c,v $
 */
/*
 * 	File: ns532/PC532/conf.c
 *	Author: Tatu Ylonen, Helsinki University of Technology 1992.
 */

#include <mach/machine/vm_types.h>
#include <device/conf.h>

extern int block_io_mmap();

extern int	timeopen(), timeclose(), timemmap();
#define	timename "time"

#include <com.h>
#if	NCOM > 0
extern int	comopen(), comclose(), comread(), comwrite();
extern int	comgetstat(), comsetstat(), comportdeath();
#define	comname "com"
#endif	NCOM > 0

#include <scsi.h>
#if	NSCSI > 0
int	rz_open(), rz_close(), rz_read(), rz_write();
int	rz_get_status(), rz_set_status(), rz_devinfo();
#define rzname "sd"
#define tzname "st"
#endif	NSCSI > 0

int bad_info_panic() {
	panic ("ns532/PC532/conf.c device info unimplemented!");
}

/*
 * List of devices - console must be at slot 0
 */
struct dev_ops	dev_name_list[] =
{
	/*name,		open,		close,		read,
	  write,	getstat,	setstat,	mmap,
	  async_in,	reset,		port_death,	subdev 
	  dev_info */

#if	NCOM > 0
	{ comname,	comopen,	comclose,	comread,
	  comwrite,	comgetstat,	comsetstat,	nulldev,
	  nodev,	nulldev,	comportdeath,	0,
      	  bad_info_panic },
#endif	NCOM > 0

	{ timename,	timeopen,	timeclose,	nulldev,
	  nulldev,	nulldev,	nulldev,	timemmap,
	  nodev,	nulldev,	nulldev,	0,
	  bad_info_panic },

#if	NSCSI > 0
	{ rzname,       rz_open,        rz_close,       rz_read,
	  rz_write,     rz_get_status,  rz_set_status,  nodev,
	  nodev,        nulldev,        nulldev,        8,
	  rz_devinfo },

	{ tzname,       rz_open,        rz_close,       rz_read,
	  rz_write,     rz_get_status,  rz_set_status,  nodev,
	  nodev,        nulldev,        nulldev,        8,
	  rz_devinfo },
#endif	NSCSI > 0
};
int	dev_name_count = sizeof(dev_name_list)/sizeof(dev_name_list[0]);

/*
 * Indirect list.
 */
struct dev_indirect dev_indirect_list[] = {

#if	NCOM > 0
	/* console */
	{ "console",	&dev_name_list[0],	0 }
#endif	NCOM > 0
};
int	dev_indirect_count = 
#if	NCOM > 0
    sizeof(dev_indirect_list) / sizeof(dev_indirect_list[0]);
#else	NCOM > 0
    0);
#endif	NCOM > 0

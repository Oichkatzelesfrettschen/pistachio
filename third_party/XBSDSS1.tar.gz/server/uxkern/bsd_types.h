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
 * 14-Nov-92  Johannes Helander (jvh) at Helsinki University of Technology
 *	Define NO_TIME_H before including sys/time.h to avoid getting time.h.
 *
 * $Log:	bsd_types.h,v $
 * Revision 2.1  92/04/21  17:10:48  rwd
 * BSDSS
 * 
 *
 */

#ifndef	_UXKERN_BSD_TYPES_H_
#define	_UXKERN_BSD_TYPES_H_

/*
 * Types for BSD kernel interface.
 */

#include <sys/types.h>
#include <sys/param.h>
#define NO_TIME_H
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/signal.h>

#include <uxkern/bsd_types_gen.h>

typedef	char		*char_array;
typedef char		small_char_array[SMALL_ARRAY_LIMIT];
typedef	char		path_name_t[PATH_LENGTH];
typedef struct timeval	timeval_t;
typedef	struct timeval	timeval_2_t[2];
typedef	struct timeval	timeval_3_t[3];
typedef
struct statb_t {
	int	s_dev;
	int	s_ino;
	int	s_mode;
	int	s_nlink;
	int	s_uid;
	int	s_gid;
	int	s_rdev;
	int	s_size;
	int	s_atime;
	int	s_mtime;
	int	s_ctime;
	int	s_blksize;
	int	s_blocks;
	int	s_flags;
	int	s_gen;
} statb_t;

typedef	struct rusage	rusage_t;
typedef	char		sockarg_t[128];
typedef	int		entry_array[16];
typedef	int		gidset_t[NGROUPS];
typedef	struct rlimit	rlimit_t[1];
typedef	struct sigvec	sigvec_t;
typedef struct sigaction sigaction_t;
typedef	struct sigstack	sigstack_t;
typedef struct timezone	timezone_t;
typedef	struct itimerval itimerval_t;
typedef	char		hostname_t[MAXHOSTNAMELEN];
typedef	char		cfname_t[64];

#endif	_UXKERN_BSD_TYPES_H_

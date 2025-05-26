#
# Mach Operating System
# Copyright (c) 1991 Carnegie Mellon University
# All Rights Reserved.
# 
# Permission to use, copy, modify and distribute this software and its
# documentation is hereby granted, provided that both the copyright
# notice and this permission notice appear in all copies of the
# software, derivative works or modified versions, and any portions
# thereof, and that both notices appear in supporting documentation.
#
# CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
# CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
# ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
#
# Carnegie Mellon requests users of this software to return to
#
#  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
#  School of Computer Science
#  Carnegie Mellon University
#  Pittsburgh PA 15213-3890
#
# any improvements or extensions that they make and grant Carnegie Mellon
# the rights to redistribute these changes.
#
#
# HISTORY
# $Log:	Makefile,v $
# Revision 2.2  92/06/25  17:28:42  mrt
# 	Added new .h files
# 	[92/06/16            mrt]
# 	Created for ODE make
# 
# 	$EndLog$
# 	[92/06/16  15:09:30  mrt]
# 

# This Makefile copies the header files from the bsdss source
# i386 directory to the export directory where they can be found
# in the directory "machine". This eliminates the need for 
# machine links in the sources.

VPATH			= ${MAKETOP}server/ns532

DATAFILES		= ansi.h endian.h limits.h param.h \
			  second_SYS.h stdarg.h types.h \
			  pcb.h proc.h trap.h reg.h

#			  asm.h psl.h 

INCLUDES		= ${DATAFILES}

EXPDIR			= /bsdss/ns532/

.include <${RULES_MK}>

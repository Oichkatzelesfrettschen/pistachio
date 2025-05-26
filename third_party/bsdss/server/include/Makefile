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
# Revision 2.2  92/06/25  17:28:34  mrt
# 	Created for ODE make
# 	[92/06/16  15:09:11  mrt]
# 
#
# $EndLog$

#  This makefile creates a symlink named machine to the
#  directory ${KERN_MACHINE_DIR} in the exports/usr/include
#  area. ${KERN_MACHINE_DIR} is usually the same as ${target_machine}
#  but is given a variable of its own, for machines where it might be
#  different.
#

VPATH		= /usr/include

EXPINC_SUBDIRS	= ${KERN_MACHINE_DIR}

OTHERS		= machine

INCLUDES	= ${OTHERS}

EXPDIR		= /bsdss/

.include <${RULES_MK}>

machine: ${ALWAYS}
	${RM} ${_RMFLAGS_} machine
	${LN} -s ${KERN_MACHINE_DIR} machine

# 
# Mach Operating System
# Copyright (c) 1992 Carnegie Mellon University
# Copyright (c) 1992 Helsinki University of Technology
# All Rights Reserved.
# 
# Permission to use, copy, modify and distribute this software and its
# documentation is hereby granted, provided that both the copyright
# notice and this permission notice appear in all copies of the
# software, derivative works or modified versions, and any portions
# thereof, and that both notices appear in supporting documentation.
# 
# CARNEGIE MELLON AND HELSINKI UNIVERSITY OF TECHNOLOGY ALLOW FREE USE
# OF THIS SOFTWARE IN ITS "AS IS" CONDITION.  CARNEGIE MELLON AND
# HELSINKI UNIVERSITY OF TECHNOLOGY DISCLAIM ANY LIABILITY OF ANY KIND
# FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
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
# 20-Mar-92  Johannes Helander (jvh) at Helsinki University of Technology
#	Created.
#
# $Log: Makefile.ns532,v $
#

###############################################################################
#BEGIN	Machine dependent Makefile fragment for the pc532
###############################################################################

BROKEN_COMPILER_SWITCH = -O0

FUSSY= -Wall -Wstrict-prototypes -Wmissing-prototypes

LDOBJS_PREFIX = ${ORDERED}

MK_LDFLAGS= -e _pstart -T ${TEXTORG}

ioconf.c:
	@echo The hack below should get fixed sometime.
	cp /dev/null ioconf.c

start.o:	${start.o_SOURCE} assym.s
locore.o:	${locore.o_SOURCE} assym.s
cswitch.o:	${cswitch.o_SOURCE} assym.s

###############################################################################
#END	Machine dependent Makefile fragment for the pc532
###############################################################################

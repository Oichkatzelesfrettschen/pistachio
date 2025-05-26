# 
# Mach Operating System
# Copyright (c) 1992 Carnegie Mellon University
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
# $Log:	template.mk,v $
# Revision 2.2  92/06/25  17:28:14  mrt
# 	Derived from conf/Makefile.i386, changed for ODE make.
# 	[92/06/16            mrt]
# 
# Revision 2.2  92/04/22  14:01:03  rwd
# 	LIBS and -L of MKLIB is not machine specific.
# 	[92/04/22            rwd]
# 
# Revision 2.1  92/04/21  17:11:47  rwd
# BSDSS
# 
#
#

###############################################################################
#BEGIN	Machine dependent Makefile fragment for the i386
###############################################################################

#tmp

#LIBS_P=-lc_p

LDOBJS_PREFIX=
LDFLAGS= -x -e __start

###############################################################################
#END	Machine dependent Makefile fragment for the i386
###############################################################################

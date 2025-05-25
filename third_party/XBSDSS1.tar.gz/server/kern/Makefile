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
# 12-Nov-92  Johannes Helander (jvh) at Helsinki University of Technology
#	Added pc532.
#
# $Log:	Makefile,v $
# Revision 2.1  92/04/21  17:13:24  rwd
# BSDSS
# 
#
#

#	@(#)Makefile	7.13 (Berkeley) 5/9/91

# Makefile for kernel tags files, init_systent, etc.

ARCH=	vax tahoe hp300 i386 ns532

all:
	@echo "make tags, make links or init_sysent.c only"

init_sysent.c syscalls.c ../sys/syscall.h: makesyscalls.sh syscalls.master
	-mv -f init_sysent.c init_sysent.c.bak
	-mv -f syscalls.c syscalls.c.bak
	-mv -f ../sys/syscall.h ../sys/syscall.h.bak
	sh makesyscalls.sh syscalls.master

# Kernel tags:
# We need to have links to tags files from the generic directories
# that are relative to the machine type, even via remote mounts;
# therefore we use symlinks to $SYSTAGS, which points at
# ${SYSDIR}/${MACHINE}/tags.

SYSTAGS=/var/db/sys_tags
SYSDIR=/sys

# Put the ../sys stuff near the end so that subroutine definitions win when
# there is a struct tag with the same name (eg., vmmeter).  The real
# solution would probably be for ctags to generate "struct vmmeter" tags.

COMM=	../net/*.[ch] ../netimp/*.[ch] ../netinet/*.[ch] ../netns/*.[ch] \
	../netiso/*.[ch] ../netccitt/*.[ch] \
	../kern/*.c ../ufs/*.[ch] ../nfs/*.[ch] ../vm/*.[ch] ../sys/*.h

# Directories in which to place tags links (other than machine-dependent)
DGEN=	kern sys net netimp netinet netns netccitt nfs ufs vm

tags: FRC
	-for i in ${ARCH}; do \
	    cd ../$$i && make ${MFLAGS} tags; done

links: ${SYSTAGS}
	-for i in ${DGEN}; do \
	    cd $$i && rm -f tags; ln -s ${SYSTAGS} tags; done
	-for i in ${ARCH}; do \
	    cd ../$$i && make ${MFLAGS} links; done

${SYSTAGS}:
	ln -s ${SYSDIR}/${MACHINE}/tags ${SYSTAGS}

FRC:

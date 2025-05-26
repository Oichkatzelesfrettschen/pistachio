#
#  Mach Operating System
#  Copyright (c) 1991,1990,1989 Carnegie Mellon University
#  All Rights Reserved.
#  
#  Permission to use, copy, modify and distribute this software and its
#  documentation is hereby granted, provided that both the copyright
#  notice and this permission notice appear in all copies of the
#  software, derivative works or modified versions, and any portions
#  thereof, and that both notices appear in supporting documentation.
#  
#  CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
#  CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
#  ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
#  
#  Carnegie Mellon requests users of this software to return to
#  
#   Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
#   School of Computer Science
#   Carnegie Mellon University
#   Pittsburgh PA 15213-3890
#  
#  any improvements or extensions that they make and grant Carnegie the
#  rights to redistribute these changes.
# 
# Makefile for bsd emulator.
#
# HISTORY
# $Log:	Makefile,v $
# Revision 2.4  92/07/08  16:11:25  mrt
# 	Used ASCPP for preprocessing .s files as vanilla gcc does not
# 	understand the -ES switch.
# 	[92/07/06            mrt]
# 
# Revision 2.3  92/06/25  17:19:05  mrt
# 	Modified for ODE make
# 	[92/06/16            mrt]
# 

# all directories that are in INCFLAGS or INCDIRS must be here so
# that make can find the relative dependencies that md will generate
VPATH	= ${target_machine}:../server:${EXPORTBASE}/bsdss${INCDIRS:S/-I/:/g}

OTHERS		= emulator
DEPENDENCIES	=
MIG_HDRS	=
PROGRAMS	= 

INCFLAGS	= -I. -I../server -I${EXPORTBASE}/bsdss
#INCDIRS	= -I${EXPORTBASE}/usr/include  (set in setvar.csh)

CC_OPT_LEVEL	= -g
DEFINES		= -DCMUCS=1 -DTypeCheck=0 -DMACH_IPC_COMPAT=0 -DCOMPAT_43
CFLAGS		= ${DEFINES}

PMAXASFLAGS	= -nocpp
PMAXLDFLAGS	= -e __start
AT386LDFLAGS	= -x -e __start ${LIBDIRS}
I386LDFLAGS	= -x -e __start ${LIBDIRS}

MIGFLAGS 	= ${DEFINES}
ASFLAGS		= ${${TARGET_MACHINE}ASFLAGS}
LDFLAGS		= $(${TARGET_MACHINE}LDFLAGS:U-x -e start)

LIBS		= -lthreads -lmach_sa 
#CRT0		= ${EXPORTBASE}/usr/lib/crt0.o
CRT0		!= wh -q -L crt0.o
#LIBDIRS	= -L${EXPORTBASE}/usr/lib (set in setvar.csh)


OFILES= bsd_user_side.o emul_init.o emul_stack_alloc.o emul_generic.o \
	allocator.o mach_user.o emul_sysent.o\
	emul_machdep.o emul_vector.o bsd_1_user.o emul_cache.o \
	emul_mapped.o

.include <${RULES_MK}>

emulator: ${OFILES} emulator_base
	${_LD_} -z -o emulator -T `./emulator_base` ${_LDFLAGS_} ${OFILES} \
		${CRT0} ${_LIBS_}
	-xstrip emulator

#  Leave out the optimzation flags as gcc-cpp doesn't like  them

_CPPFLAGS_=\
	${_CC_CFLAGS_}\
	${${.TARGET}_CENV:U${CENV}}\
	${${.TARGET}_CFLAGS:U${CFLAGS}} ${TARGET_FLAGS}\
	${${.TARGET}_CARGS:U${CARGS}}\
	${_CC_NOSTDINC_} ${_GENINC_} ${_CC_INCDIRS_} ${_CC_PICLIB_}

.s.o:
	$(ASCPP) ${_CPPFLAGS_} ${.IMPSRC} > ${.PREFIX:T}.as
	$(AS) ${ASFLAGS} -o ${.TARGET} ${.PREFIX:T}.as
#	rm -f ${.PREFIX}.as

emulator_base: emulator_base.c
	$(_CC_) $(_CCFLAGS_) -o emulator_base ${emulator_base.c:P}

bsd_1_user.o: uxkern/bsd_1_user.c
	$(_CC_) $(_CCFLAGS_) -c -o bsd_1_user.o ${uxkern/bsd_1_user.c:P}

bsd_user_side.o: uxkern/bsd_1.h


#
#  The sysent tables, mapping trap numbers to actions, are generated
#  automatically on both the emulator and the server side:
#
bsd_user_side.o : sys/syscall.h

sys/syscall.h: emul_sysent.c

emul_sysent.c: ../server/kern/makesyscalls.sh ../server/kern/syscalls.master
	-rm -rf tmp sys
	-mkdir tmp sys
	cd tmp;/bin/sh ../${../server/kern/makesyscalls.sh:P} \
			../${../server/kern/syscalls.master:P}

#
#  Mach IPC-based interfaces
#

MAKE_UXKERN = [ -d uxkern ] || mkdir uxkern


BSD_FILES = uxkern/bsd_1.h uxkern/bsd_1_user.c

$(BSD_FILES): uxkern/bsd_types_gen.h uxkern/bsd_1.defs
	-$(MAKE_UXKERN)
	$(MIG) $(_MIGFLAGS_) \
		-header uxkern/bsd_1.h \
		-user uxkern/bsd_1_user.c \
		-server /dev/null \
		${uxkern/bsd_1.defs:P}

#
# We have to tell Mig to generate a header file, even though we don't really
# want it, because the user file includes the header file from the current
# directory.
#

mach_user.c: mach/mach.defs
	$(MIG) $(_MIGFLAGS_) -DEMULATOR \
		-header mach_interface.h \
		-user mach_user.c \
		-server /dev/null \
		${mach/mach.defs:P}

uxkern/bsd_types_gen.h : bsd_types_gen
	-$(MAKE_UXKERN)
	./bsd_types_gen > uxkern/bsd_types_gen.h

bsd_types_gen : bsd_types_gen.o
	${_host_CC_} -o bsd_types_gen bsd_types_gen.o

bsd_types_gen.o : uxkern/bsd_types_gen.c
	${_host_CC_} -c ${_CCFLAGS_} ${uxkern/bsd_types_gen.c:P}

.if exists(depend.mk)
.include "depend.mk"
.endif

printenv:
	@echo VPATH ${VPATH}
	printenv _MAKEPSD

#
# Mach Operating System
# Copyright (c) 1992,1991 Carnegie Mellon University
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
# Revision 2.3  92/07/08  16:16:55  mrt
# 	Dropped the -MD from MIGFLAGS. If it is appropriate, it will be
# 	defined in osf.mach3.mk
# 	[92/07/06            mrt]
# 
# 	Changed the SOBJS rule to use all the CCFLAGS except the
# 	optimization level as gcc-cpp won't accept them.
# 	[92/07/02            mrt]
# 
# Revision 2.2  92/06/25  17:28:23  mrt
# 	New Server makefile template for new Reno style make.
# 	This file used to be named Makefile.template. Much
# 	of the reformating was done by Glenn Marcy of the
# 	Open Software Foundation.
# 	[92/04/28            mrt]
# 
# 	$EndLog$
# 
#
#


# ${EXPORTBASE}/bsdss/machine has copies of the ${KERN_MACHINE_DIR} files
# All the directories in which md generated relative dependencies must be
# on VPATH
VPATH=..:${EXPORTBASE}/bsdss${INCDIRS:S/-I/:/g}
OTHERS=${LOAD}

DEPENDENCIES=
MIG_HDRS = 

DEFINES		= -DCOMPAT_43 -DBSD=44 -DMACH_IPC_COMPAT=0 $(VOLATILE)
ALLOPTS		= ${IDENT} -DKERNEL ${DEFINES}
LINTOPTS	= ${ALLOPTS}

CFLAGS		= ${ALLOPTS} ${PROFILING:D-DGPROF}
DRIVER_CFLAGS	=${CFLAGS}
PROFILING_CFLAGS=${CFLAGS}
CC_OPT_LEVEL	=-g

#INCFLAGS are procesed by genpath and expanded relative to all the 
# sourcedirs, _CC_GENINC is not. _CC_GENINC is also the additional list
# of directories that are seached for th #include "foo.h" files.
INCFLAGS	=-I.. -I${EXPORTBASE}/bsdss 
#INCDIRS	= -I${EXPORTBASE}/usr/include (set in setvar.csh)
_CC_GENINC_	= -I. 

LIBS		= -lthreads -lmach_sa
#CRT0		= ${EXPORTBASE}/usr/lib/crt0.o
CRT0		!= wh -q -L crt0.o
#LIBDIRS	= -L${EXPORTBASE}/usr/lib (set in setvar.csh)

MIGFLAGS	= ${IDENT} ${DEFINES}

#
#  LDOBJS is the set of object files which comprise the server.
#  It is used both in the dependency list for each *vmunix.swap
#  rule emitted by config and also in the .sys.swap rule
#  below which links the kernel from these objects.
#  
#  LDOBJS_PREFIX and LDOBJS_SUFFIX are defined in the machine
#  dependent Makefile (if necessary).
#
LDOBJS=${LDOBJS_PREFIX} ${OBJS} ${LDOBJS_SUFFIX}

#
#  LDDEPS is the set of extra dependencies associated with each
#  *vmunix.swap rule (in addition to $LDOBJS and the swap*.o file).
#
#  LDDEPS_PREFIX is defined in the machine dependent Makefile
#  (if necessary).
#

LDDEPS=${LDDEPS_PREFIX} conf/newvers.sh conf/copyright \
	conf/version.major conf/version.minor conf/version.variant \
	conf/version.edit conf/version.patch symbols.sort

#
#  PRELDDEPS is another set of extra dependencies associated with each
#  *vmunix.swap rule (in addition to $LDOBJS and the swap*.o file).
#  It is defined in the machine dependent Makefile (if necessary).
#
#  The generated rule looks like
#	vmunix.sys : ${PRELDDEPS} ${LDOBJS} ${LDDEPS}
#

#
#  SWAPDEPS is the set of extra dependencies associated with each
#  swap*.o rule emitted by config (in addition to the
#  ../machine/swap*.c file).
#
#  SWAPDEPS_PREFIX is defined in the machine dependent Makefile
#  (if necessary).
#
SWAPDEPS=${SWAPDEPS_PREFIX} 

#
#  SYSDEPS is the set of extra dependencies associated with each
#  *vmunix rule (in addition to *vmunix.sys).
#
#  SYSDEPS_PREFIX is defined in the machine dependent Makefile
#  (if necessary).
#
SYSDEPS=${SYSDEPS_PREFIX}


#
#  These macros are filled in by the config program depending on the
#  current configuration.  The MACHDEP macro is replaced by the
#  contents of the machine dependent makefile template and the others
#  are replaced by the corresponding symbol definitions for the
#  configuration.
#

%OBJS

%CFILES

%SFILES

%BFILES

%ORDERED

%MACHDEP


#
#  This macro is replaced by definitions for the name ${LOAD} and 
#  ${LOAD}.swap. It also generates the dependency for ${LOAD}.swap
#
 
%LOAD

.include <${RULES_MK}>


.SUFFIXES: .swap .sys

.PRECIOUS: Makefile

#
#  Default rule used to build a *vmunix.sys configuration from the
#  object list and a particular *vmunix.swap module.  The *vmunix.swap
#  module is a normal object file compiled from the appropriate swap*.c
#  file and then copied to the *vmunix.swap name to trigger the full
#  kernel link using this default rule.
#

.swap.sys:
	/bin/sh ${conf/newvers.sh:P} \
		`cat vers.config` \
		${conf/copyright:P} \
		`cat ${conf/version.major:P} \
		     ${conf/version.minor:P} \
		     ${conf/version.variant:P} \
		     ${conf/version.edit:P} \
		     ${conf/version.patch:P}`
	${_CC_} -c ${_CCFLAGS_} vers.c
	rm -f ${.TARGET:.sys=} ${.TARGET}
	@echo loading ${.TARGET}
	${_LD_} ${_LDFLAGS_} ${LDOBJS} vers.o ${.PREFIX}.swap \
		 ${CRT0} ${_LIBS_}
	@echo stripping ${.TARGET}
	-xstrip a.out
	-size a.out
	chmod 755 a.out
	-mv a.out ${.TARGET}

relink: ${LOAD:=.relink}

${LOAD:=.relink}: ${LDDEPS}
	/bin/sh ${conf/newvers.sh:P} \
		${conf/copyright:P} \
		`cat ${conf/version.major:P} \
		     ${conf/version.minor:P} \
		     ${conf/version.variant:P} \
		     ${conf/version.edit:P} \
		     ${conf/version.patch:P}`
	${_CC_} -c ${_CCFLAGS_} vers.c
	rm -f ${.TARGET:.relink=} ${.TARGET:.relink=.sys}
	@echo loading ${.TARGET:.relink=.sys}
	${_LD_} ${_LDFLAGS_} ${LDOBJS} vers.o \
		${.PREFIX}.swap ${CRT0} ${_LIBS_}
	chmod 755 a.out
	-${SWAPSYS}
	-mv a.out ${.TARGET:.relink=.sys}
	eval `awk 'NR==1{S=$$1;next;};\
	 END {\
	   C = "ln ${.TARGET:.relink=.sys} ${.TARGET:.relink=}";\
	   if (S != "") {\
	     C = C "; ln ${.TARGET:.relink=} ${.TARGET:.relink=}" S; \
	     printf "rm -f ${.TARGET:.relink=}%s; ", S; \
	   }\
	   printf "echo \"%s\"; %s\n", C, C; \
	 }' vers.suffix`

${LOAD}: $${.TARGET:=.sys} ${SYSDEPS}
	eval `awk 'NR==1{S=$$1;next;}\
	END {\
	  C = "ln ${.TARGET}.sys ${.TARGET}";\
	  if (S != "") {\
	    C = C "; ln ${.TARGET} ${.TARGET}" S; \
	    printf "rm -f ${.TARGET}%s; ", S; \
	  }\
	  printf "echo \"%s\"; %s\n", C, C; \
	}' vers.suffix`

${LOAD:=.sys}: ${PRELDDEPS} ${LDOBJS} ${LDDEPS}

${LOAD:=.swap}: $${$${.TARGET}_SWAPSPEC}
	@${CP} ${${.TARGET}_SWAPSPEC} ${.TARGET}

${LOAD:@.F.@${${.F.}_SWAPSPEC}@}: ${SWAPDEPS}


#
#  OBJSDEPS is the set of files (defined in the machine dependent
#  template if necessary) which all objects depend on (such as an
#  in-line assembler expansion filter
#

${OBJS}: ${OBJSDEPS}


.if !defined(MACHINEDEP_RULES)
${COBJS}: $${$${.TARGET}_SOURCE}
	${_CC_} -c ${_CCFLAGS_} ${${${.TARGET}_SOURCE}:P}

#  Leave out the optimzation flags as gcc-cpp doesn't like  them

_CPPFLAGS_=\
	${_CC_CFLAGS_}\
	${${.TARGET}_CENV:U${CENV}}\
	${${.TARGET}_CFLAGS:U${CFLAGS}} ${TARGET_FLAGS}\
	${${.TARGET}_CARGS:U${CARGS}}\
	${_CC_NOSTDINC_} ${_GENINC_} ${_CC_INCDIRS_} ${_CC_PICLIB_}

${SOBJS}: $${$${.TARGET}_SOURCE}
	${ASCPP} ${_CPPFLAGS_} ${${${.TARGET}_SOURCE}:P} > ${.PREFIX}.i 
	${AS} ${ASFLAGS} -o ${.TARGET} ${.PREFIX}.i 
	${RM} ${_RMFLAGS_} ${.PREFIX}.i
.endif

${DECODE_OFILES}: $${$${.TARGET}_SOURCE}
	${RM} ${_RMFLAGS_} ${.TARGET}
	${UUDECODE} ${${${.TARGET}_SOURCE}:P}

#
#  Rules for components which are not part of the kernel proper or that
#  need to be built in a special manner.
#

MKODIRS = uxkern

#  The Mig-generated files go into subdirectories.
#  This target makes sure they exist

.BEGIN : 
	@-for dir in ${MKODIRS}; do \
	[ -d $$dir ] || \
	  { echo "mkdir $$dir"; mkdir $$dir; } \
	done


#
#  Mach IPC-based interfaces
#

#  Explicit dependencies on generated files,
#  to ensure that Mig has been run by the time
#  these files are compiled.

bsd_server.o: uxkern/bsd_1_server.c uxkern/bsd_types_gen.h 


bsd_server_side.o: uxkern/bsd_types_gen.h


BSD_1_FILES = uxkern/bsd_1_server.c

$(BSD_1_FILES): uxkern/bsd_types_gen.h uxkern/bsd_1.defs
	 $(MIG) $(_MIGFLAGS_)\
		-header /dev/null \
		-user /dev/null \
		-server uxkern/bsd_1_server.c \
		 ${uxkern/bsd_1.defs:P}

uxkern/bsd_types_gen.h : bsd_types_gen
	./bsd_types_gen > uxkern/bsd_types_gen.h

bsd_types_gen : bsd_types_gen.o
	${_host_CC_} -o bsd_types_gen bsd_types_gen.o

bsd_types_gen.o : uxkern/bsd_types_gen.c
	${_host_CC_} -c ${_CCFLAGS_} ${uxkern/bsd_types_gen.c:P}

gprof: kgmon_on all kgmon_off

kgmon_on:
	kgmon -r -b

kgmon_off:
	kgmon -h -p
	gprof /mach >GPROF

kern/init_sysent.c kern/syscalls.c sys/syscall.h: kern/makesyscalls.sh kern/syscalls.master
	-rm -rf kern sys
	-mkdir sys kern
	cd kern;/bin/sh ../${kern/makesyscalls.sh:P} ../${kern/syscalls.master:P}


#
#  Run "lint" on the current build directory.  This will often be done
#  with a special configuration that defines the union of all options
#  used in all configurations so that all combinations are checked.
#  The LINTFILES variable allows lint to be easily run on an optional
#  sub-set of files specified on the make command line when this is
#  desired.
#

LINTFILES= ${CFILES} ioconf.c

lint:	ALWAYS 
	@lint -n -hbxn -DGENERIC ${LINTOPTS} ${LINTFILES} \
	    ${machine/swapgeneric.c:P} | \
	    egrep -v 'struct/union .* never defined' | \
	    egrep -v 'possible pointer alignment problem'

ALWAYS:


symbols.sort: ${KERN_MACHINE_DIR}/symbols.raw
	-grep -v '^#' ${${KERN_MACHINE_DIR}/symbols.raw:P} \
	    | sed 's/^	//' | sort -u > symbols.tmp
	-mv -f symbols.tmp symbols.sort

printenv:
	@echo VPATH=${VPATH}
	@echo INCDIRS=${INCDIRS}

.if exists(depend.mk)
.include "depend.mk"
.endif


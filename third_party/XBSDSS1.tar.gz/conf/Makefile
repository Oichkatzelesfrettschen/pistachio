#
# Mach Operating System
# Copyright (c) 1992,1991,1990,1989 Carnegie Mellon University
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
# Revision 2.2  92/06/25  17:27:22  mrt
# 	New for ODE make.
# 	[92/06/16            mrt]
# 
# 	$EndLog$
# 

MASTER		= MASTER
MASTER_TM	= ${target_machine}/MASTER
MASTER_LOCAL	= MASTER.local
MASTER_TM_LOCAL	= ${target_machine}/MASTER.local

CONFIG		?=${BSDSS_${TARGET_MACHINE}_CONFIG:U${BSDSS_CONFIG:UDEFAULT}}

OTHERS		= ../${CONFIG}/Makefile

.include <${RULES_MK}>

${CONFIG}:	${MASTER} ${MASTER_TM} \
		${MASTER_LOCAL} ${MASTER_TM_LOCAL} \
		mkconfig.csh
	@echo "[ generating $@ from {,${target_machine}/}MASTER{,.local} ]"
	csh -f ${mkconfig.csh:P} ${CONFIG}\
		${${MASTER}:P}\
		${${MASTER_TM}:P}\
		${${MASTER_LOCAL}:P}\
		${${MASTER_TM_LOCAL}:P} >${CONFIG}.tmp
	@-if [ -s ${CONFIG}.tmp ]; then \
	    if [ -f ${CONFIG} ]; then \
		diff ${CONFIG} ${CONFIG}.tmp; \
		mv ${CONFIG} ${CONFIG}.old; \
	    fi; \
	    mv ${CONFIG}.tmp ${CONFIG}; \
	else \
	    rm -f ${CONFIG}.tmp; \
	fi

../${CONFIG}/Makefile:	${CONFIG} \
			files \
			${target_machine}/files \
			template.mk \
			${target_machine}/template.mk \
			../src/config/config 
	@echo "[ configuring ${CONFIG} ]"
	@-if [ -d ../${CONFIG} ]; then true; else mkdir ../${CONFIG}; fi
	../src/config/config `genpath -I.` -c . ${CONFIG_FLAGS} ${CONFIG}
	@echo ${CONFIG} >../${CONFIG}/vers.config

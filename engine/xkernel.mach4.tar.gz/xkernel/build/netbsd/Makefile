#
# $RCSfile: Makefile.sunos,v $
#
# x-kernel v3.2
#
# Copyright (c) 1993,1991,1990  Arizona Board of Regents
#
# $Revision: 1.44.1.1 $
# $Date: 1993/11/24 23:16:06 $
#
#
# Build-directory Makefile for the sunos/sparc platform.
#
# There are many variables defined here that may vary from site to
# site.  If you are the first person at your site to build  an
# x-kernel, look over the definitions carefully and determine what the
# appropriate values should be for your site. 

XPLATFORM = netbsd
MACHINE = intelx86

#
# HOWTOCOMPILE controls optimization flags and tracing routines.  
# It should be set to either DEBUG or OPTIMIZE
#
HOWTOCOMPILE = DEBUG
#HOWTOCOMPILE = OPTIMIZE

#
# If XK_BYPASS_RCS is defined (to anything), RCS consistency checks
# will be disabled.  This can make a build run significantly faster.
#
XK_BYPASS_RCS = blurp	

#
# Compiler-related flags.  If you decide to change compilers, some of
# these flags may also have to change.
#
# We have experienced some problems using gcc2 on the sunos platform.
#	
CC = gcc
DEBUGWARNINGS = 
OPTIMIZEWARNINGS = 
WARNINGS = $($(HOWTOCOMPILE)WARNINGS)
CC_SPECIFIC_FLAGS = -fwritable-strings
OPTIMIZECFLAGS = -O 
DEBUGCFLAGS = -g

#
# Libraries.  Some of these libraries are labeled 'optional',
# indicating that they are only used by some protocols.  Names and
# locations of libraries may be site-specific.
#
LIBPATH = #-L/cs/x32/lib -L/cs/x32/lib/$(MACHINE)
sunos_LIBS = #-llwp	# required 
LIB_GNUMP = -lgmp	# GNU multi-precision library, optional


SED = sed
MAKE = make
REALCC = /bin/cc
RANLIB = ranlib
AR = ar

#
# XRT names a path to the top of the x-kernel source tree
#
XRT = ../..

HOW = $(HOWTOCOMPILE)$(XPLATFORM)-$(MACHINE)

#
# TMP_CPPFLAGS and TMP_CFLAGS apply to the build directory only, and
# won't be propagated to compilation in other x-kernel directories. 
#
# TMP_CPPFLAGS = 
# TMP_CFLAGS = 

#
# When compiling private versions of x-kernel utilities in your build
# area, you may need to add include paths.  Add these to the
# TMP_INCLUDES variable.
#
TMP_INCLUDES = $(COMPOSE_INCLUDES)


MAKEFILE=Makefile
MAKEFLAGS += --no-print-directory

OPTIMIZECPPFLAGS = 
DEBUGCPPFLAGS = -DXK_DEBUG
BUILD_CPPFLAGS := $($(HOWTOCOMPILE)CPPFLAGS) -DX_NETBSD
BUILD_CFLAGS = $($(HOWTOCOMPILE)CFLAGS)
LDFLAGS = $(LIBPATH)

all:	xkernel

#
# The COMPOSE_MAKEFILE is generated from graph.comp when 'make compose'
# is run. 
#
COMPOSE_MAKEFILE = Makefile.local
COMPOSEFLAGS = -f -m $(COMPOSE_MAKEFILE)
include $(COMPOSE_MAKEFILE)

$(COMPOSE_MAKEFILE):
	@touch $(COMPOSE_MAKEFILE)

#
# If you compile source code in your build area which isn't associated
# with a particular protocol, add the source names to PRIVSRC and the
# object names (e.g., ./$(HOW)/foo.o) to PRIVOBJ. 
#
#PRIVSRC = \
#
#PRIVOBJ = \

include $(XRT)/build/Template/Makefile.common

#
# Include Makefile.nolibs to compile without libraries
#
include $(XRT)/build/Template/Makefile.nolibs


xkernel:	$(OBJ) $(XLIBS)
	@echo linking xkernel ...
	@$(CC) $(LDFLAGS) -o xkernel $(OBJ) $(XLIBS) $(COMPOSE_LIBS) \
		$($(XPLATFORM)_LIBS) 

include Makedep.$(HOW)

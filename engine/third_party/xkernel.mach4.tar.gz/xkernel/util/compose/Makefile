#
# $RCSfile: Makefile,v $
#
# x-kernel v3.2
#
# Copyright (c) 1993,1991,1990  Arizona Board of Regents
#
# $Revision: 1.17 $
# $Date: 1993/11/16 02:56:17 $
#
# object for different architectures are kept in subdirectories of
# this directory.  Make variable $(ARCH) defines the architecture type.


ifeq ($(ARCH),) 

depend .default:
	@echo "Make variable ARCH must be defined"

endif


ifeq ($(ARCH),hp700)

# always include -O in CFLAGS to avoid bug in gcc 1.3x!

TMP_CPPFLAGS := -L$(ROOT)/usr/ccs/lib \
		-I$(ROOT)/usr/include -nostdinc \
		-DHPOSFCOMPAT -DOSF 
TMP_CFLAGS =	-O

endif

XK_BYPASS_RCS = true
OBJS = 	parse.o compose.o util.o error.o
CC=gcc
WARNINGS := -Wall
CC_SPECIFIC_FLAGS := -g -fwritable-strings
TMP_CPPFLAGS += -DARCH_INCLUDE=\"compose_$(ARCH).h\"

MAKE_INCLUDE = ../../makefiles

ARCH_PFX = $(ARCH)/
#
# We define "HOW" as "ARCH" so we can use the normal xkernel depend
# rule which rewrites the objects with a $(HOW) prefix
#
HOW = $(ARCH)


OBJS := $(addprefix $(ARCH_PFX), $(OBJS))
SRC = compose.c error.c parse.c util.c

.PHONY : compose
compose: $(ARCH)/compose

setup:
	@-if [ ! -d $(ARCH) ] ; then umask 2; mkdir $(ARCH) ; else true; fi

$(ARCH)/compose: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(ARCH)/%.o : %.c
	$(CC) -c $(CFLAGS) -o $@ $<
	
include $(MAKE_INCLUDE)/Makefile.local

CFLAGS := $(TMP_CPPFLAGS) $(CC_SPECIFIC_FLAGS) 

clean:  localclean
depend: localdepend

tags:
	etags -tw -f TAGS *.c *.h

include Makedep.$(HOW)


#
# prottbldump makefile
#
# x-kernel v3.2
#
# Copyright (c) 1993,1991,1990  Arizona Board of Regents
#
# $Revision: 1.12 $
# $Date: 1993/11/17 17:59:55 $
#
# object for different architectures are kept in subdirectories of
# this directory.  Make variable $(ARCH) defines the architecture type.

#Ugly, ugly

#ARCH = intelx86 
#XPLATFORM = netbsd 
#XRT = /usr/src/mach/xkernel

#end ugly

CC = gcc
TMP_CFLAGS = -g -fwritable-strings 

SRC = ptbldump.c
OBJS = ptbldump.o

ifeq ($(ARCH),) 
default:
	@echo "Environment variable ARCH must be defined"
else
ifeq ($(XPLATFORM),) 
default:
	@echo "Environment variable XPLATFORM must be defined"
else


XK_BYPASS_RCS = true
% :: RCS/%,v

#
# We define "HOW" as "ARCH" so we can use the normal xkernel depend
# rule which rewrites the objects with a $(HOW) prefix
#
HOW = $(ARCH)

TMP_INCLUDES = -DX_NETBSD -I$(XRT)/$(XPLATFORM)/include -I$(XRT)/pi


OBJS := $(addprefix $(ARCH)/, $(OBJS))
EXT_HOW := DEBUG$(XPLATFORM)-$(ARCH)
EXT_OBJS := \
	$(XRT)/pi/idmap/$(EXT_HOW)/idmap.o \
	$(XRT)/pi/$(EXT_HOW)/prottbl.o \
	$(XRT)/pi/$(EXT_HOW)/prottbl_parse.o \
	$(XRT)/pi/$(EXT_HOW)/xk_debug.o \

ptbldump:	$(ARCH)/ptbldump 

setup:
	@-if [ ! -d $(ARCH) ] ; then umask 2; mkdir $(ARCH) ; fi

$(ARCH)/ptbldump:	$(OBJS) $(EXT_OBJS) 
	$(CC) $(CFLAGS) -o $@ $^

$(HOW)/%.o : %.c
	$(CC) -c $(CFLAGS) -o $@ $<

include $(XRT)/makefiles/Makefile.local

ptblData.c: $(XRT)/etc/prottbl
	ptbldump $(XRT)/etc/prottbl > ptblData.c

clean:  localclean
depend: localdepend

include Makedep.$(HOW)

endif
endif

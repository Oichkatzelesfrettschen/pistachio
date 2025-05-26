# 
# Makefile for utility programs
#
# x-kernel v3.2
#
# Copyright (c) 1993,1991,1990  Arizona Board of Regents
#
# $Revision: 1.21 $
# $Date: 1993/11/15 00:50:50 $

#
# Utility programs which are contained in a single file are built at
# the top level (these are in make variable PGMS.)  Programs which are
# composed of several files are in their own directories.  In each case,
# object files and executables are built in subdirectories with the name
# $(ARCH).  
#


% :: RCS/%,v

XRT = ..
DIRS = compose 
DIRS_MACHKERNEL = ptbldump
PGMS = fperm dirname random_int promfile phostnumber pnetnum pfakeether
SRC = fperm.c dirname.c random_int.c promfile.c phostnumber.c \
	pnetnum.c pfakeether.c
OBJS = $(addprefix $(ARCH)/, $(PGMS))
ALL = $(DIRS) $(DIRS_MACHKERNEL) $(PGMS) 
TMP_CFLAGS = -g -fwritable-strings 
CC = gcc
MAKEFLAGS += --no-print-directory

DEST_BIN_DIR = ../bin

#
# We define "HOW" as "ARCH" so we can use the normal xkernel depend
# rule which rewrites the objects with a $(HOW) prefix
#
HOW = $(ARCH)

ifeq ($(ARCH),) 
def2:
	@echo "Make variable ARCH must be defined"
BOTCHED = def2
endif


ifneq ($(BOTCHED),)
.DEFAULT:	
	@make $(BOTCHED)
else

XK_BYPASS_RCS = true

default:	$(DIRS) $(PGMS)

all:	$(ALL)

#
# For utilities which have their own directories, descend there and make
#
.PHONY : $(DIRS) $(PGMS) $(DIRS_MACHKERNEL)
$(DIRS) $(DIRS_MACHKERNEL):
	cd $@ ; make XRT:=../$(XRT); 

$(PGMS): 
	@if [ ! -d $(ARCH) ] ; then mkdir $(ARCH) ; else true; fi ;
	@make $(ARCH)/$@

$(OBJS): $(ARCH)/% : %.c
	$(CC) $(CFLAGS) -o $@ $^

install: 
	@if [ "$(ARCH)" = "" ] ; then \
	    echo "ARCH environment variable must be defined" ; \
	else \
	    if [ ! -d $(DEST_BIN_DIR)/$(ARCH) ] ; then mkdir $(DEST_BIN_DIR)/$(ARCH) ; else true; fi ; \
	    make $(PGM_INSTALL_TARGETS) ; \
	    make $(DIR_INSTALL_TARGETS) ; \
	fi ; 

install_machkernel: 
	@if [ "$(ARCH)" = "" ] ; then \
	    echo "ARCH environment variable must be defined" ; \
	else \
	    make $(DIR_MACHKERNEL_INSTALL_TARGETS) ; \
	fi ; 

install_all: install install_machkernel

DIR_MACHKERNEL_INSTALL_TARGETS = $(addprefix ../bin/$(ARCH)/, $(DIRS_MACHKERNEL))
DIR_INSTALL_TARGETS = $(addprefix ../bin/$(ARCH)/, $(DIRS))
PGM_INSTALL_TARGETS = $(addprefix ../bin/$(ARCH)/, $(PGMS))

$(DIR_INSTALL_TARGETS) $(DIR_MACHKERNEL_INSTALL_TARGETS) : $(DEST_BIN_DIR)/$(ARCH)/% : %
	@cmp $*/$(ARCH)/$* $@ > /dev/null 2>&1 || echo installing $* ; cp $*/$(ARCH)/$* $@

$(PGM_INSTALL_TARGETS) : $(DEST_BIN_DIR)/$(ARCH)/% : %
	@cmp $(ARCH)/$* $@ > /dev/null 2>&1 || echo installing $* ; cp $(ARCH)/$* $@

setup: localdepend
	@-if [ ! -d $(ARCH) ] ; then umask 2; mkdir $(ARCH) ; else true; fi
	@for dir in $(DIRS) ; do \
		echo $$dir ; \
		cd $$dir ; \
		make depend ; \
		make setup ; \
		cd .. ; \
	done ; \
	true


include $(XRT)/makefiles/Makefile.local


endif

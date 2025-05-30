# Copyright (C) 1988, 1989 Free Software Foundation, Inc.
# This file is part of GNU Make.
# 
# GNU Make is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 1, or (at your option)
# any later version.
#
# GNU Make is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with GNU Make; see the file COPYING.  If not, write to
# the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

#
#	Makefile for GNU Make
#

CFLAGS = $(defines) -O2
LDFLAGS = 

# Define nothing for BSD, USG for System V, and USGr3 (as well as USG) for
# SVR3, HPUX for HP-UX (as well as USG or USGr3 as appropriate).  If you have a
# USG hybrid with <sys/wait.h> and wait3, define HAVE_SYS_WAIT.  If you are USG
# but have sys_siglist, define HAVE_SIGLIST.  If you are USG, and not USGr3,
# but have dup2, define HAVE_DUP2.  If your `cc' command doesn't grok -o
# options with -c (true for many 4.2 BSD derivatives), define
# NO_MINUS_C_MINUS_O.  If you don't want archive support, define NO_ARCHIVES.
# If you want to avoid use of floating-point numbers, define NO_FLOAT.  If your
# compiler can handle `enum' bitfields (and it's not GCC), define
# ENUM_BITFIELDS.
defines =

# Define UMAX here to use Encore's inq_stats call.  If the load average is in a
# symbol in /dev/kmem, define KERNEL_FILE_NAME if not "/vmunix", LDAV_SYMBOL if
# not "_avenrun", LDAV_TYPE if not `long int', and LDAV_CVT to convert the
# LDAV_TYPE value from LDAV_SYMBOL (in `load') to a double if this is not
# "(double) load".  If a `struct nlist' (as defined in <nlist.h>) has a `n_un'
# union, rather than a simple `n_name' member, define NLIST_NAME_UNION.  If the
# `n_name' member of a `struct nlist' is an array that must be copied into,
# define NLIST_NAME_ARRAY.  Otherwise, define NO_LDAV.
LOAD_AVG =

# If you don't want archive support, comment these out.
ARCHIVES = arscan.o ar.o
ARCHIVES_SRC = arscan.c ar.c

# If your system needs extra libraries loaded in, define them here.
# System V probably need -lPW for alloca.
LOADLIBES =

# If your system doesn't have alloca, or the one provided is bad,
# get it from the Emacs distribution and define these.
#ALLOCA = alloca.o
#ALLOCASRC = alloca.c

# If there are remote execution facilities defined,
# enable them with switches here (see remote-*.c).
REMOTE =

# Any extra object files your system needs.
extras =

# Directory to install `make' in.
bindir = $(prefix)/usr/local/bin
# Directory to install the man page in.
mandir = $(prefix)/usr/local/man/man1
# Number to put on the man page filename.
manext = 1

objs = glob.o commands.o job.o dir.o file.o load.o misc.o main.o read.o \
       remake.o remote.o rule.o implicit.o default.o variable.o expand.o \
       function.o vpath.o version.o $(ARCHIVES) $(ALLOCA) $(extras)
srcs = glob.c commands.c job.c dir.c file.c load.c misc.c main.c read.c \
       remake.c remote.c rule.c implicit.c default.c variable.c expand.c \
       function.c vpath.c version.c $(ALLOCASRC) $(ARCHIVES_SRC) \
       commands.h dep.h file.h job.h make.h rule.h variable.h


.SUFFIXES:
.SUFFIXES: .o .c .h .ps .dvi .texinfo

.PHONY: all doc
all: make
doc: make-info make.dvi


# Take your pick.
#makeinfo = emacs -batch make.texinfo -f texinfo-format-buffer -f save-buffer
makeinfo = makeinfo make.texinfo

make-info: make.texinfo
	$(makeinfo)


make.dvi: make.texinfo
	-tex make.texinfo
	texindex make.cp make.fn make.ky make.pg make.tp make.vr
	-tex make.texinfo

make.ps: make.dvi
	dvi2ps make.dvi > make.ps

make: $(objs)
	$(CC) $(LDFLAGS) $(objs) $(LOADLIBES) -o make.new
	mv -f make.new make

load.o: load.c
	$(CC) $(CFLAGS) $(LOAD_AVG) -c load.c
remote.o: remote.c
	$(CC) $(CFLAGS) $(REMOTE) -c remote.c

TAGS: $(srcs)
	etags -tw $(srcs)
tags: $(srcs)
	ctags -tw $(srcs)

.PHONY: install
install: $(bindir)/make $(mandir)/make.$(manext)

$(bindir)/make: make
	cp make $@.new
	mv $@.new $@

$(mandir)/make.$(manext): make.man
	cp make.man $@

.PHONY: clean realclean
clean:
	-rm -f make *.o core
realclean: clean
	-rm -f TAGS tags make.info* make-* make.dvi
	-rm -f make.?? make.??s make.log make.toc make.*aux
# Automatically generated dependencies.
glob.o : glob.c 
commands.o : commands.c make.h dep.h commands.h file.h variable.h job.h 
job.o : job.c make.h commands.h job.h file.h variable.h 
dir.o : dir.c make.h 
file.o : file.c make.h commands.h dep.h file.h variable.h 
load.o : load.c make.h commands.h job.h 
misc.o : misc.c make.h dep.h 
main.o : main.c make.h commands.h dep.h file.h variable.h job.h 
read.o : read.c make.h commands.h dep.h file.h variable.h 
remake.o : remake.c make.h commands.h job.h dep.h file.h 
remote.o : remote.c remote-stub.c make.h commands.h 
rule.o : rule.c make.h commands.h dep.h file.h variable.h rule.h 
implicit.o : implicit.c make.h rule.h dep.h file.h 
default.o : default.c make.h rule.h dep.h file.h commands.h variable.h 
variable.o : variable.c make.h commands.h variable.h dep.h file.h 
expand.o : expand.c make.h commands.h file.h variable.h 
function.o : function.c make.h variable.h dep.h commands.h job.h 
vpath.o : vpath.c make.h file.h variable.h 
version.o : version.c 
arscan.o : arscan.c 
ar.o : ar.c make.h file.h 

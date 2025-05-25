#
# $RCSfile: Makefile,v $
#
# x-kernel v3.2
#
# Copyright (c) 1991  Arizona Board of Regents
#
# $Revision: 1.2 $
# $Date: 1993/02/02 00:55:43 $
#

CC = gcc
INCLUDE = -I. -I$(XRT)/mach3/netipc -I/usr/mach/include -I/usr/cs/include -I/usr/include
OBJS = ssr-test.o testmnipc

testmnipc:	ssr-test.o
	ld -o testmnipc -n -L/usr/mach/lib /usr/mach/lib/crt0.o ssr-test.o -lnetname -lthreads -lmach /usr/local/lib/gcc-lib/mips-dec-mach/2.1/libgcc.a -lc

ssr-test.o: ssr-test.c unxk.h
	$(CC) -c -g $(INCLUDE) -DXK_DEBUG -DMACH -Dpmax -Wreturn-type -fwritable-strings -o ssr-test.o ssr-test.c

% :: RCS/%,v

clean:	
	rm -f $(OBJS)

depend:	
	@$(CC) -MM $(INCLUDE) *.c  > Makedep


Makedep:
	touch $@

include Makedep

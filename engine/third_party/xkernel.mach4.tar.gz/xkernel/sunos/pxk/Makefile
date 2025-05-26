#
# $RCSfile: Makefile,v $
#
# x-kernel v3.2
#
# Copyright (c) 1993,1991,1990  Arizona Board of Regents
#
# $Revision: 1.22.1.1 $
# $Date: 1993/11/10 18:39:34 $
#

DIRS = 
FILES = alloc event machine process init redefines time trace utils

default: $(HOW)/machine.o $(HOW)/process.o def

# files which must be compiled with cc instead of gcc
$(HOW)/machine.o $(HOW)/process.o : $(HOW)/%.o : %.c
	@if [ ! -d $(HOW) ] ; then mkdir $(HOW) ; fi ; \
	echo $(REALCC) -c $(BUILD_CFLAGS) $(CPPFLAGS) -o $@ $< ; \
	$(REALCC) -c $(BUILD_CFLAGS) $(CPPFLAGS) -o $@ $<

include $(MAKE_INCLUDE)/Makefile.common







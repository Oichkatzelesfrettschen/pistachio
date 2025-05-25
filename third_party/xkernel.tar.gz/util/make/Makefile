#
# gnumake makefile
#
# x-kernel v3.2
#
# Copyright (c) 1993,1991,1990  Arizona Board of Regents
#
# $Revision: 1.7 $
# $Date: 1993/02/02 00:00:37 $
#

MAKE_D=make-3.57
DEST = ../../../../bin/$(ARCH)

make:
	-@if [ "$(ARCH)" = "" ] ; then \
		echo "ARCH environment variable must be defined" ; \
	else \
		if [ ! -d $(MAKE_D)/$(ARCH) ] ; then \
			echo making new object directory for $(ARCH) ; \
			mkdir $(MAKE_D)/$(ARCH) ; \
			cd $(MAKE_D)/$(ARCH) ; \
			ln -s ../Makefile ../*.c ../*.h . ; \
			cd ../.. ; \
		else \
			true ; \
		fi ; \
		cd $(MAKE_D)/$(ARCH); make;		\
		cmp make ../../../../bin/$(ARCH)/make > /dev/null 2>&1 || \
			echo installing make ; \
			if [ ! -d $(DEST) ] ; then mkdir $(DEST) ; else true ; fi ; \
			cp make $(DEST) ; \
	fi ; 


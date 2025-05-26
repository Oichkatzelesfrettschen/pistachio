#!/bin/sh -
# 
# Mach Operating System
# Copyright (c) 1992 Carnegie Mellon University
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
# $Log:	newvers.sh,v $
# Revision 2.1  92/04/21  17:11:54  rwd
# BSDSS
# 
#
#

CONFIG="$1"; copyright="$2"; major="$3"; min="$4";
variant="$5"; build="$6"; patch="$7";
h=`hostname` t=`date`
v="${major}.${min}.${patch}"
b="${variant}${build}"
if [ -z "$h" -o -z "$t" -o -z "${CONFIG}" ]; then
    exit 1
fi
(
  echo "#ifdef __STDC__" ;
  echo "#define CONST const" ;
  echo "#else" ;
  echo "#define CONST" ;
  echo "#endif" ;
  echo "CONST int  version_major      = ${major};" ;
  echo "CONST int  version_minor      = ${min};" ;
  echo "CONST char version_version[32]  = \"${v}\";" ;
  echo "CONST char version_release[32]  = \"${b}\";" ;
  echo "CONST char version[] = \"Mach 3.0 BSD${v} (${variant}${build}); ${t}; ${CONFIG} ($h)\\n\";" ;
  echo "CONST char copyright[] = \"\\" ;
  sed <$copyright -e '/^#/d' -e 's;[ 	]*$;;' -e '/^$/d' -e 's;$;\\n\\;' ;
  echo "\";";
) > vers.c
if [ -s vers.suffix -o ! -f vers.suffix ]; then
    echo ".${b}.${CONFIG}" >vers.suffix
fi
exit 0

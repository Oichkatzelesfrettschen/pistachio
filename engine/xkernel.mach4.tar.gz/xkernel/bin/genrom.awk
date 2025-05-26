#
# genrom.awk
#
# x-kernel v3.2
#
# Copyright (c) 1993,1991,1990  Arizona Board of Regents
#
# $Revision: 1.5 $
# $Date: 1993/04/19 20:22:06 $
#

#
# generates an 'initRom' routine from an existing ROM file which
# initializes the 'rom' array as if the ROM file had been read at
# runtime.  This is mainly for in-kernel use.


BEGIN {
	fmtString = "\trom[%d][%d] = %s;\n";

#	printf("\n#include \"platform.h\"\n");
#	printf("\n#include \"xk_debug.h\"\n");
	printf("\nvoid\ninitRom()\n{\n");
}

{ 
	for ( i=1; i <= NF; i++ ) {
		if ( substr($i,0,1) == "#" ) {
			NF = i - 1;
			break;
		} else {
			code = code sprintf(fmtString, line, i-1, "\"" $i "\"");
		}
	}
	if ( maxFields < (NF-1) ) maxFields = NF-1;
	if ( NF > 0 ) {
		code = code sprintf(fmtString, line, NF, "0");
		line++;	
	}
}

END {
	printf("\tif ( ROM_MAX_LINES < %d ) {\n", line);
	printf("\t\txError(\"xkernel ROM array too big\");\n");
	printf("\t\treturn;\n");
	printf("\t}\n");

	printf("\tif ( ROM_MAX_FIELDS < %d ) {\n", maxFields);
	printf("\t\txError(\"xkernel ROM -- too many fields\");\n");
	printf("\t\treturn;\n");
	printf("\t}\n");
	printf code;
	printf(fmtString, line, 0, "0");
	printf("}\n");
}

/* 
 * Mach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 * 
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they make and grant Carnegie Mellon 
 * the rights to redistribute these changes.
 */
/*
 * The port of Mach 3.0 to the pc532 was done by
 *
 *     Johannes Helander <jvh@cs.hut.fi>
 *     Tero Kivinen      <kivinen@hut.fi>
 *     Tatu Ylonen       <ylo@cs.hut.fi>
 *
 * at Helsinki University of Technology, Finland 1992.
 */
/*
 * HISTORY
 * Revision 1.0  92/06/06  20:28:35  iwd
 *      Added support for running handling little endian
 *      executables on a big endian machine and visa versa.
 * 	[92/06/06            iwd]
 * 
 * 18-Mar-92  Johannes Helander (jvh) at Helsinki University of Technology
 *	Created from i386 code.
 *
 * $Log:	exec.c,v $
 * 
 */

/*
 * Executable file format for ns532.
 */
#include <sys/types.h>
#include <sys/file.h>
#include <ns532/exec.h>

#include <mach/machine/vm_types.h>
#include <mach/boot_info.h>
#include "cross_endian.h"
void put_num();
int get_num();


int	is_Sequent = 0;		/* Sequent boot format */
#define	SQT_K_MAGIC	0x42eb

off_t
exec_header_size()
{
	return sizeof(struct exec);
}

void
write_exec_header(out_file, kp, file_size)
	int		   out_file;	/* output file */
	struct loader_info *kp;		/* kernel load info */
	off_t		   file_size;	/* size of output file */
{
	struct exec	out_header;

	if (is_Sequent) {
	    PUT_EXTERN(out_header.a_magic, SQT_K_MAGIC);
	    PUT_EXTERN(out_header.a_text, file_size);
	}
	else {
	    PUT_EXTERN(out_header.a_magic, OMAGIC);
	    PUT_EXTERN(out_header.a_text, (int) file_size - sizeof(struct exec));
	}
	PUT_EXTERN(out_header.a_data, 0);
	PUT_EXTERN(out_header.a_syms, 0);
	PUT_EXTERN(out_header.a_bss, 0);
	PUT_EXTERN(out_header.a_trsize, 0);
	PUT_EXTERN(out_header.a_drsize, 0);
	if (is_Sequent)
	    PUT_EXTERN(out_header.a_entry, kp->entry_1 & 0x0fffffff);
	else
	    PUT_EXTERN(out_header.a_entry, kp->entry_1);

	write(out_file, (char *)&out_header, sizeof(out_header));
}

int
ex_get_header(in_file, is_kernel, lp,
		sym_header, sym_header_size)
	int	in_file;
	int	is_kernel;
	struct loader_info *lp;
	char	*sym_header;		/* OUT */
	int	*sym_header_size;	/* OUT */
{
	vm_offset_t	str_size;
	struct exec	x;

	lseek(in_file, (off_t) 0, L_SET);
	read(in_file, (char *)&x, sizeof(x));

	switch (INTERN(x.a_magic)) {
	    case 0407:
		lp->text_start  = 0;
		lp->text_size   = 0;
		lp->text_offset = 0;
		lp->data_start  = 0x10000;
		lp->data_size   = INTERN(x.a_text) + INTERN(x.a_data);
		lp->data_offset = sizeof(struct exec);
		lp->bss_size    = INTERN(x.a_bss);
		break;

	    case 0410:
		if (INTERN(x.a_text) == 0) {
		    return (0);
		}
		lp->text_start  = 0x10000;
		lp->text_size   = INTERN(x.a_text);
		lp->text_offset = sizeof(struct exec);
		lp->data_start  = lp->text_start + lp->text_size;
		lp->data_size   = INTERN(x.a_data);
		lp->data_offset = lp->text_offset + lp->text_size;
		lp->bss_size    = INTERN(x.a_bss);
		break;

	    case 0413:
		if (INTERN(x.a_text) == 0) {
		    return (0);
		}
		lp->text_start  = 0x10000;
		lp->text_size   = sizeof(struct exec) + INTERN(x.a_text);
		lp->text_offset = 0;
		lp->data_start  = lp->text_start + lp->text_size;
		lp->data_size   = INTERN(x.a_data);
		lp->data_offset = lp->text_offset + lp->text_size;
		lp->bss_size    = INTERN(x.a_bss);
		break;

	    default:
		return (0);
	}
	lp->entry_1 = INTERN(x.a_entry);
	lp->entry_2 = 0;

	/*
	 * If loading kernel, look for Sequent bootstrap code
	 * after a.out header.
	 */
	if (is_kernel) {
	    static int sequent_bootstrap[] = {
		0x0000ffff,
		0x00cf9a00,
		0x0000ffff,
		0x00cf9200,
		0x00180017,
		0x00000000,
		0x00000000,
		0x00000000
	    };
	    int	bootstrap_match[sizeof(sequent_bootstrap)/sizeof(int)];
	    register int i;

	    read(in_file, (char *)&bootstrap_match[0],
			  sizeof(bootstrap_match));

	    INTERNALIZE(bootstrap_match[0]);

	    for (i = 0; i < sizeof(sequent_bootstrap)/sizeof(int); i++)
	        if (sequent_bootstrap[i] != bootstrap_match[i])
		    break;
	    if (i == sizeof(sequent_bootstrap)/sizeof(int)) {
		/*
		 * Sequent boot.
		 */
		is_Sequent = 1;
		if (INTERN(x.a_magic) == 0413) {
		    /*
		     * Treat 413 boot file as 410.
		     */
		    lp->text_size -= sizeof(struct exec);
		    lp->text_offset += sizeof(struct exec);
		}
	    }
	}

	lp->sym_offset = lp->data_offset + INTERN(x.a_data);

	/*
	 * Read string table size.
	 */
	lseek(in_file, (off_t) (lp->sym_offset+INTERN(x.a_syms)), L_SET);
	read(in_file, (char *)&str_size, sizeof(str_size));

        /* Don't intern sym_header since we print it back out again but never
         * use it.
         */
	*(int *)sym_header = x.a_syms;
	*sym_header_size = sizeof(int);

	lp->sym_size = INTERN(x.a_syms) + INTERN(str_size);

	return 1;
}


/* The 32k is little endian. */

void put_num(buf,val,n)
     char	*buf;
     long	val;
     char       n;
{ 
  for (; n > 0; n--)
    {
      *buf++ = val & 0xff; val >>= 8;
    }
}

int get_num(buf, n)
     char *buf;
     int n;
{
  int val = 0;
  buf += (n - 1);
  for (; n > 0; n--)
    {
      val = val * 256 + (*buf-- & 0xff);
    }
  return val;
}



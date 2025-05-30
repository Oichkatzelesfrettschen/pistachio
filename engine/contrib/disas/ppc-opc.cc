/* ppc-opc.c -- PowerPC opcode list
   Copyright 1994, 1995, 1996, 1997, 1998, 2000, 2001, 2002, 2003
   Free Software Foundation, Inc.
   Written by Ian Lance Taylor, Cygnus Support

   This file is part of GDB, GAS, and the GNU binutils.

   GDB, GAS, and the GNU binutils are free software; you can redistribute
   them and/or modify them under the terms of the GNU General Public
   License as published by the Free Software Foundation; either version
   2, or (at your option) any later version.

   GDB, GAS, and the GNU binutils are distributed in the hope that they
   will be useful, but WITHOUT ANY WARRANTY; without even the implied
   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
   the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this file; see the file COPYING.  If not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.  */

#include "ppc.h"

/* This file holds the PowerPC opcode table.  The opcode table
   includes almost all of the extended instruction mnemonics.  This
   permits the disassembler to use them, and simplifies the assembler
   logic, at the cost of increasing the table size.  The table is
   strictly constant data, so the compiler should be able to put it in
   the .text section.

   This file also holds the operand table.  All knowledge about
   inserting operands into instructions and vice-versa is kept in this
   file.  */

/* Local insertion and extraction functions.  */

#ifndef _
#define _(string) string
#endif

#ifndef PARAMS
#define PARAMS(a) a
#endif

#ifndef ATTRIBUTE_UNUSED
#define ATTRIBUTE_UNUSED
#endif

static unsigned long insert_bat
  PARAMS ((unsigned long, long, int, const char **));
static long extract_bat
  PARAMS ((unsigned long, int, int *));
static unsigned long insert_bba
  PARAMS ((unsigned long, long, int, const char **));
static long extract_bba
  PARAMS ((unsigned long, int, int *));
static unsigned long insert_bd
  PARAMS ((unsigned long, long, int, const char **));
static long extract_bd
  PARAMS ((unsigned long, int, int *));
static unsigned long insert_bdm
  PARAMS ((unsigned long, long, int, const char **));
static long extract_bdm
  PARAMS ((unsigned long, int, int *));
static unsigned long insert_bdp
  PARAMS ((unsigned long, long, int, const char **));
static long extract_bdp
  PARAMS ((unsigned long, int, int *));
static int valid_bo
  PARAMS ((long, int));
static unsigned long insert_bo
  PARAMS ((unsigned long, long, int, const char **));
static long extract_bo
  PARAMS ((unsigned long, int, int *));
static unsigned long insert_boe
  PARAMS ((unsigned long, long, int, const char **));
static long extract_boe
  PARAMS ((unsigned long, int, int *));
static unsigned long insert_ds
  PARAMS ((unsigned long, long, int, const char **));
static long extract_ds
  PARAMS ((unsigned long, int, int *));
static unsigned long insert_de
  PARAMS ((unsigned long, long, int, const char **));
static long extract_de
  PARAMS ((unsigned long, int, int *));
static unsigned long insert_des
  PARAMS ((unsigned long, long, int, const char **));
static long extract_des
  PARAMS ((unsigned long, int, int *));
static unsigned long insert_li
  PARAMS ((unsigned long, long, int, const char **));
static long extract_li
  PARAMS ((unsigned long, int, int *));
static unsigned long insert_mbe
  PARAMS ((unsigned long, long, int, const char **));
static long extract_mbe
  PARAMS ((unsigned long, int, int *));
static unsigned long insert_mb6
  PARAMS ((unsigned long, long, int, const char **));
static long extract_mb6
  PARAMS ((unsigned long, int, int *));
static unsigned long insert_nb
  PARAMS ((unsigned long, long, int, const char **));
static long extract_nb
  PARAMS ((unsigned long, int, int *));
static unsigned long insert_nsi
  PARAMS ((unsigned long, long, int, const char **));
static long extract_nsi
  PARAMS ((unsigned long, int, int *));
static unsigned long insert_ral
  PARAMS ((unsigned long, long, int, const char **));
static unsigned long insert_ram
  PARAMS ((unsigned long, long, int, const char **));
static unsigned long insert_ras
  PARAMS ((unsigned long, long, int, const char **));
static unsigned long insert_rbs
  PARAMS ((unsigned long, long, int, const char **));
static long extract_rbs
  PARAMS ((unsigned long, int, int *));
static unsigned long insert_sh6
  PARAMS ((unsigned long, long, int, const char **));
static long extract_sh6
  PARAMS ((unsigned long, int, int *));
static unsigned long insert_spr
  PARAMS ((unsigned long, long, int, const char **));
static long extract_spr
  PARAMS ((unsigned long, int, int *));
static unsigned long insert_tbr
  PARAMS ((unsigned long, long, int, const char **));
static long extract_tbr
  PARAMS ((unsigned long, int, int *));
static unsigned long insert_ev2
  PARAMS ((unsigned long, long, int, const char **));
static long extract_ev2
  PARAMS ((unsigned long, int, int *));
static unsigned long insert_ev4
  PARAMS ((unsigned long, long, int, const char **));
static long extract_ev4
  PARAMS ((unsigned long, int, int *));
static unsigned long insert_ev8
  PARAMS ((unsigned long, long, int, const char **));
static long extract_ev8
  PARAMS ((unsigned long, int, int *));

/* The operands table.

   The fields are bits, shift, insert, extract, flags.

   We used to put parens around the various additions, like the one
   for BA just below.  However, that caused trouble with feeble
   compilers with a limit on depth of a parenthesized expression, like
   (reportedly) the compiler in Microsoft Developer Studio 5.  So we
   omit the parens, since the macros are never used in a context where
   the addition will be ambiguous.  */

const struct powerpc_operand powerpc_operands[] =
{
  /* The zero index is used to indicate the end of the list of
     operands.  */
#define UNUSED 0
  { 0, 0, 0, 0, 0 },

  /* The BA field in an XL form instruction.  */
#define BA UNUSED + 1
#define BA_MASK (0x1f << 16)
  { 5, 16, 0, 0, PPC_OPERAND_CR },

  /* The BA field in an XL form instruction when it must be the same
     as the BT field in the same instruction.  */
#define BAT BA + 1
  { 5, 16, insert_bat, extract_bat, PPC_OPERAND_FAKE },

  /* The BB field in an XL form instruction.  */
#define BB BAT + 1
#define BB_MASK (0x1f << 11)
  { 5, 11, 0, 0, PPC_OPERAND_CR },

  /* The BB field in an XL form instruction when it must be the same
     as the BA field in the same instruction.  */
#define BBA BB + 1
  { 5, 11, insert_bba, extract_bba, PPC_OPERAND_FAKE },

  /* The BD field in a B form instruction.  The lower two bits are
     forced to zero.  */
#define BD BBA + 1
  { 16, 0, insert_bd, extract_bd, PPC_OPERAND_RELATIVE | PPC_OPERAND_SIGNED },

  /* The BD field in a B form instruction when absolute addressing is
     used.  */
#define BDA BD + 1
  { 16, 0, insert_bd, extract_bd, PPC_OPERAND_ABSOLUTE | PPC_OPERAND_SIGNED },

  /* The BD field in a B form instruction when the - modifier is used.
     This sets the y bit of the BO field appropriately.  */
#define BDM BDA + 1
  { 16, 0, insert_bdm, extract_bdm,
      PPC_OPERAND_RELATIVE | PPC_OPERAND_SIGNED },

  /* The BD field in a B form instruction when the - modifier is used
     and absolute address is used.  */
#define BDMA BDM + 1
  { 16, 0, insert_bdm, extract_bdm,
      PPC_OPERAND_ABSOLUTE | PPC_OPERAND_SIGNED },

  /* The BD field in a B form instruction when the + modifier is used.
     This sets the y bit of the BO field appropriately.  */
#define BDP BDMA + 1
  { 16, 0, insert_bdp, extract_bdp,
      PPC_OPERAND_RELATIVE | PPC_OPERAND_SIGNED },

  /* The BD field in a B form instruction when the + modifier is used
     and absolute addressing is used.  */
#define BDPA BDP + 1
  { 16, 0, insert_bdp, extract_bdp,
      PPC_OPERAND_ABSOLUTE | PPC_OPERAND_SIGNED },

  /* The BF field in an X or XL form instruction.  */
#define BF BDPA + 1
  { 3, 23, 0, 0, PPC_OPERAND_CR },

  /* An optional BF field.  This is used for comparison instructions,
     in which an omitted BF field is taken as zero.  */
#define OBF BF + 1
  { 3, 23, 0, 0, PPC_OPERAND_CR | PPC_OPERAND_OPTIONAL },

  /* The BFA field in an X or XL form instruction.  */
#define BFA OBF + 1
  { 3, 18, 0, 0, PPC_OPERAND_CR },

  /* The BI field in a B form or XL form instruction.  */
#define BI BFA + 1
#define BI_MASK (0x1f << 16)
  { 5, 16, 0, 0, PPC_OPERAND_CR },

  /* The BO field in a B form instruction.  Certain values are
     illegal.  */
#define BO BI + 1
#define BO_MASK (0x1f << 21)
  { 5, 21, insert_bo, extract_bo, 0 },

  /* The BO field in a B form instruction when the + or - modifier is
     used.  This is like the BO field, but it must be even.  */
#define BOE BO + 1
  { 5, 21, insert_boe, extract_boe, 0 },

  /* The BT field in an X or XL form instruction.  */
#define BT BOE + 1
  { 5, 21, 0, 0, PPC_OPERAND_CR },

  /* The condition register number portion of the BI field in a B form
     or XL form instruction.  This is used for the extended
     conditional branch mnemonics, which set the lower two bits of the
     BI field.  This field is optional.  */
#define CR BT + 1
  { 3, 18, 0, 0, PPC_OPERAND_CR | PPC_OPERAND_OPTIONAL },

  /* The CRB field in an X form instruction.  */
#define CRB CR + 1
  { 5, 6, 0, 0, 0 },

  /* The CRFD field in an X form instruction.  */
#define CRFD CRB + 1
  { 3, 23, 0, 0, PPC_OPERAND_CR },

  /* The CRFS field in an X form instruction.  */
#define CRFS CRFD + 1
  { 3, 0, 0, 0, PPC_OPERAND_CR },

  /* The CT field in an X form instruction.  */
#define CT CRFS + 1
  { 5, 21, 0, 0, PPC_OPERAND_OPTIONAL },

  /* The D field in a D form instruction.  This is a displacement off
     a register, and implies that the next operand is a register in
     parentheses.  */
#define D CT + 1
  { 16, 0, 0, 0, PPC_OPERAND_PARENS | PPC_OPERAND_SIGNED },

  /* The DE field in a DE form instruction.  This is like D, but is 12
     bits only.  */
#define DE D + 1
  { 14, 0, insert_de, extract_de, PPC_OPERAND_PARENS },

  /* The DES field in a DES form instruction.  This is like DS, but is 14
     bits only (12 stored.)  */
#define DES DE + 1
  { 14, 0, insert_des, extract_des, PPC_OPERAND_PARENS | PPC_OPERAND_SIGNED },

  /* The DS field in a DS form instruction.  This is like D, but the
     lower two bits are forced to zero.  */
#define DS DES + 1
  { 16, 0, insert_ds, extract_ds,
      PPC_OPERAND_PARENS | PPC_OPERAND_SIGNED | PPC_OPERAND_DS },

  /* The E field in a wrteei instruction.  */
#define E DS + 1
  { 1, 15, 0, 0, 0 },

  /* The FL1 field in a POWER SC form instruction.  */
#define FL1 E + 1
  { 4, 12, 0, 0, 0 },

  /* The FL2 field in a POWER SC form instruction.  */
#define FL2 FL1 + 1
  { 3, 2, 0, 0, 0 },

  /* The FLM field in an XFL form instruction.  */
#define FLM FL2 + 1
  { 8, 17, 0, 0, 0 },

  /* The FRA field in an X or A form instruction.  */
#define FRA FLM + 1
#define FRA_MASK (0x1f << 16)
  { 5, 16, 0, 0, PPC_OPERAND_FPR },

  /* The FRB field in an X or A form instruction.  */
#define FRB FRA + 1
#define FRB_MASK (0x1f << 11)
  { 5, 11, 0, 0, PPC_OPERAND_FPR },

  /* The FRC field in an A form instruction.  */
#define FRC FRB + 1
#define FRC_MASK (0x1f << 6)
  { 5, 6, 0, 0, PPC_OPERAND_FPR },

  /* The FRS field in an X form instruction or the FRT field in a D, X
     or A form instruction.  */
#define FRS FRC + 1
#define FRT FRS
  { 5, 21, 0, 0, PPC_OPERAND_FPR },

  /* The FXM field in an XFX instruction.  */
#define FXM FRS + 1
#define FXM_MASK (0xff << 12)
  { 8, 12, 0, 0, 0 },

  /* The L field in a D or X form instruction.  */
#define L FXM + 1
  { 1, 21, 0, 0, PPC_OPERAND_OPTIONAL },

  /* The LEV field in a POWER SC form instruction.  */
#define LEV L + 1
  { 7, 5, 0, 0, 0 },

  /* The LI field in an I form instruction.  The lower two bits are
     forced to zero.  */
#define LI LEV + 1
  { 26, 0, insert_li, extract_li, PPC_OPERAND_RELATIVE | PPC_OPERAND_SIGNED },

  /* The LI field in an I form instruction when used as an absolute
     address.  */
#define LIA LI + 1
  { 26, 0, insert_li, extract_li, PPC_OPERAND_ABSOLUTE | PPC_OPERAND_SIGNED },

  /* The LS field in an X (sync) form instruction.  */
#define LS LIA + 1
  { 2, 21, 0, 0, PPC_OPERAND_OPTIONAL },

  /* The MB field in an M form instruction.  */
#define MB LS + 1
#define MB_MASK (0x1f << 6)
  { 5, 6, 0, 0, 0 },

  /* The ME field in an M form instruction.  */
#define ME MB + 1
#define ME_MASK (0x1f << 1)
  { 5, 1, 0, 0, 0 },

  /* The MB and ME fields in an M form instruction expressed a single
     operand which is a bitmask indicating which bits to select.  This
     is a two operand form using PPC_OPERAND_NEXT.  See the
     description in opcode/ppc.h for what this means.  */
#define MBE ME + 1
  { 5, 6, 0, 0, PPC_OPERAND_OPTIONAL | PPC_OPERAND_NEXT },
  { 32, 0, insert_mbe, extract_mbe, 0 },

  /* The MB or ME field in an MD or MDS form instruction.  The high
     bit is wrapped to the low end.  */
#define MB6 MBE + 2
#define ME6 MB6
#define MB6_MASK (0x3f << 5)
  { 6, 5, insert_mb6, extract_mb6, 0 },

  /* The MO field in an mbar instruction.  */
#define MO MB6 + 1
  { 5, 21, 0, 0, 0 },

  /* The NB field in an X form instruction.  The value 32 is stored as
     0.  */
#define NB MO + 1
  { 6, 11, insert_nb, extract_nb, 0 },

  /* The NSI field in a D form instruction.  This is the same as the
     SI field, only negated.  */
#define NSI NB + 1
  { 16, 0, insert_nsi, extract_nsi,
      PPC_OPERAND_NEGATIVE | PPC_OPERAND_SIGNED },

  /* The RA field in an D, DS, X, XO, M, or MDS form instruction.  */
#define RA NSI + 1
#define RA_MASK (0x1f << 16)
  { 5, 16, 0, 0, PPC_OPERAND_GPR },

  /* The RA field in a D or X form instruction which is an updating
     load, which means that the RA field may not be zero and may not
     equal the RT field.  */
#define RAL RA + 1
  { 5, 16, insert_ral, 0, PPC_OPERAND_GPR },

  /* The RA field in an lmw instruction, which has special value
     restrictions.  */
#define RAM RAL + 1
  { 5, 16, insert_ram, 0, PPC_OPERAND_GPR },

  /* The RA field in a D or X form instruction which is an updating
     store or an updating floating point load, which means that the RA
     field may not be zero.  */
#define RAS RAM + 1
  { 5, 16, insert_ras, 0, PPC_OPERAND_GPR },

  /* The RB field in an X, XO, M, or MDS form instruction.  */
#define RB RAS + 1
#define RB_MASK (0x1f << 11)
  { 5, 11, 0, 0, PPC_OPERAND_GPR },

  /* The RB field in an X form instruction when it must be the same as
     the RS field in the instruction.  This is used for extended
     mnemonics like mr.  */
#define RBS RB + 1
  { 5, 1, insert_rbs, extract_rbs, PPC_OPERAND_FAKE },

  /* The RS field in a D, DS, X, XFX, XS, M, MD or MDS form
     instruction or the RT field in a D, DS, X, XFX or XO form
     instruction.  */
#define RS RBS + 1
#define RT RS
#define RT_MASK (0x1f << 21)
  { 5, 21, 0, 0, PPC_OPERAND_GPR },

  /* The SH field in an X or M form instruction.  */
#define SH RS + 1
#define SH_MASK (0x1f << 11)
  { 5, 11, 0, 0, 0 },

  /* The SH field in an MD form instruction.  This is split.  */
#define SH6 SH + 1
#define SH6_MASK ((0x1f << 11) | (1 << 1))
  { 6, 1, insert_sh6, extract_sh6, 0 },

  /* The SI field in a D form instruction.  */
#define SI SH6 + 1
  { 16, 0, 0, 0, PPC_OPERAND_SIGNED },

  /* The SI field in a D form instruction when we accept a wide range
     of positive values.  */
#define SISIGNOPT SI + 1
  { 16, 0, 0, 0, PPC_OPERAND_SIGNED | PPC_OPERAND_SIGNOPT },

  /* The SPR field in an XFX form instruction.  This is flipped--the
     lower 5 bits are stored in the upper 5 and vice- versa.  */
#define SPR SISIGNOPT + 1
#define PMR SPR
#define SPR_MASK (0x3ff << 11)
  { 10, 11, insert_spr, extract_spr, 0 },

  /* The BAT index number in an XFX form m[ft]ibat[lu] instruction.  */
#define SPRBAT SPR + 1
#define SPRBAT_MASK (0x3 << 17)
  { 2, 17, 0, 0, 0 },

  /* The SPRG register number in an XFX form m[ft]sprg instruction.  */
#define SPRG SPRBAT + 1
#define SPRG_MASK (0x3 << 16)
  { 2, 16, 0, 0, 0 },

  /* The SR field in an X form instruction.  */
#define SR SPRG + 1
  { 4, 16, 0, 0, 0 },

  /* The STRM field in an X AltiVec form instruction.  */
#define STRM SR + 1
#define STRM_MASK (0x3 << 21)
  { 2, 21, 0, 0, 0 },

  /* The SV field in a POWER SC form instruction.  */
#define SV STRM + 1
  { 14, 2, 0, 0, 0 },

  /* The TBR field in an XFX form instruction.  This is like the SPR
     field, but it is optional.  */
#define TBR SV + 1
  { 10, 11, insert_tbr, extract_tbr, PPC_OPERAND_OPTIONAL },

  /* The TO field in a D or X form instruction.  */
#define TO TBR + 1
#define TO_MASK (0x1f << 21)
  { 5, 21, 0, 0, 0 },

  /* The U field in an X form instruction.  */
#define U TO + 1
  { 4, 12, 0, 0, 0 },

  /* The UI field in a D form instruction.  */
#define UI U + 1
  { 16, 0, 0, 0, 0 },

  /* The VA field in a VA, VX or VXR form instruction.  */
#define VA UI + 1
#define VA_MASK	(0x1f << 16)
  { 5, 16, 0, 0, PPC_OPERAND_VR },

  /* The VB field in a VA, VX or VXR form instruction.  */
#define VB VA + 1
#define VB_MASK (0x1f << 11)
  { 5, 11, 0, 0, PPC_OPERAND_VR },

  /* The VC field in a VA form instruction.  */
#define VC VB + 1
#define VC_MASK (0x1f << 6)
  { 5, 6, 0, 0, PPC_OPERAND_VR },

  /* The VD or VS field in a VA, VX, VXR or X form instruction.  */
#define VD VC + 1
#define VS VD
#define VD_MASK (0x1f << 21)
  { 5, 21, 0, 0, PPC_OPERAND_VR },

  /* The SIMM field in a VX form instruction.  */
#define SIMM VD + 1
  { 5, 16, 0, 0, PPC_OPERAND_SIGNED},

  /* The UIMM field in a VX form instruction.  */
#define UIMM SIMM + 1
  { 5, 16, 0, 0, 0 },

  /* The SHB field in a VA form instruction.  */
#define SHB UIMM + 1
  { 4, 6, 0, 0, 0 },

  /* The other UIMM field in a EVX form instruction.  */
#define EVUIMM SHB + 1
  { 5, 11, 0, 0, 0 },

  /* The other UIMM field in a half word EVX form instruction.  */
#define EVUIMM_2 EVUIMM + 1
  { 32, 11, insert_ev2, extract_ev2, PPC_OPERAND_PARENS },

  /* The other UIMM field in a word EVX form instruction.  */
#define EVUIMM_4 EVUIMM_2 + 1
  { 32, 11, insert_ev4, extract_ev4, PPC_OPERAND_PARENS },

  /* The other UIMM field in a double EVX form instruction.  */
#define EVUIMM_8 EVUIMM_4 + 1
  { 32, 11, insert_ev8, extract_ev8, PPC_OPERAND_PARENS },

  /* The WS field.  */
#define WS EVUIMM_8 + 1
#define WS_MASK (0x7 << 11)
  if (invalid != (int *) nullptr

  if (invalid != (int *) nullptr
  if (invalid != (int *) nullptr
  { 1, 16, 0, 0, PPC_OPERAND_OPTIONAL },

};

/* The functions used to insert and extract complicated operands.  */

/* The BA field in an XL form instruction when it must be the same as
   the BT field in the same instruction.  This operand is marked FAKE.
   The insertion function just copies the BT field into the BA field,
   and the extraction function just checks that the fields are the
   same.  */

/*ARGSUSED*/
static unsigned long
insert_bat ( unsigned long insn, long value ATTRIBUTE_UNUSED,
    int dialect ATTRIBUTE_UNUSED, const char **errmsg ATTRIBUTE_UNUSED )
{
  return insn | (((insn >> 21) & 0x1f) << 16);
}

static long
extract_bat ( unsigned long insn, int dialect ATTRIBUTE_UNUSED, int *invalid )
{
  if (invalid != (int *) NULL
      && ((insn >> 21) & 0x1f) != ((insn >> 16) & 0x1f))
    *invalid = 1;
  return 0;
}

/* The BB field in an XL form instruction when it must be the same as
   the BA field in the same instruction.  This operand is marked FAKE.
   The insertion function just copies the BA field into the BB field,
   and the extraction function just checks that the fields are the
   same.  */

/*ARGSUSED*/
static unsigned long
insert_bba ( unsigned long insn, long value ATTRIBUTE_UNUSED,
     int dialect ATTRIBUTE_UNUSED, const char **errmsg ATTRIBUTE_UNUSED )
{
  return insn | (((insn >> 16) & 0x1f) << 11);
}

static long
extract_bba ( unsigned long insn, int dialect ATTRIBUTE_UNUSED,
     int *invalid)
{
  if (invalid != (int *) NULL
      && ((insn >> 16) & 0x1f) != ((insn >> 11) & 0x1f))
    *invalid = 1;
  return 0;
}

/* The BD field in a B form instruction.  The lower two bits are
   forced to zero.  */

/*ARGSUSED*/
static unsigned long
insert_bd ( unsigned long insn, long value,
     int dialect ATTRIBUTE_UNUSED, const char **errmsg ATTRIBUTE_UNUSED )
{
  return insn | (value & 0xfffc);
}

/*ARGSUSED*/
static long
extract_bd ( unsigned long insn, int dialect ATTRIBUTE_UNUSED,
     int *invalid ATTRIBUTE_UNUSED)
{
  return ((insn & 0xfffc) ^ 0x8000) - 0x8000;
}

/* The BD field in a B form instruction when the - modifier is used.
   This modifier means that the branch is not expected to be taken.
   For chips built to versions of the architecture prior to version 2
   (ie. not Power4 compatible), we set the y bit of the BO field to 1
   if the offset is negative.  When extracting, we require that the y
   bit be 1 and that the offset be positive, since if the y bit is 0
   we just want to print the normal form of the instruction.
   Power4 compatible targets use two bits, "a", and "t", instead of
   the "y" bit.  "at" == 00 => no hint, "at" == 01 => unpredictable,
  if (invalid != (int *) nullptr)
  if (invalid != (int *) nullptr)
   for branch on CTR.  We only handle the taken/not-taken hint here.  */

/*ARGSUSED*/
static unsigned long
insert_bdm ( unsigned long insn, long value, int dialect,
     const char **errmsg ATTRIBUTE_UNUSED )
{
  if ((dialect & PPC_OPCODE_POWER4) == 0)
    {
      if ((value & 0x8000) != 0)
	insn |= 1 << 21;
    }
  else
    {
      if ((insn & (0x14 << 21)) == (0x04 << 21))
	insn |= 0x02 << 21;
      else if ((insn & (0x14 << 21)) == (0x10 << 21))
	insn |= 0x08 << 21;
    }
  return insn | (value & 0xfffc);
}

static long
extract_bdm ( unsigned long insn, int dialect, int *invalid )
{
  if (invalid != (int *) NULL)
    {
      if ((dialect & PPC_OPCODE_POWER4) == 0)
	{
	  if (((insn & (1 << 21)) == 0) != ((insn & (1 << 15)) == 0))
	    *invalid = 1;
	}
      else
	{
	  if ((insn & (0x17 << 21)) != (0x06 << 21)
	      && (insn & (0x1d << 21)) != (0x18 << 21))
	    *invalid = 1;
	}
    }
  return ((insn & 0xfffc) ^ 0x8000) - 0x8000;
}

/* The BD field in a B form instruction when the + modifier is used.
   This is like BDM, above, except that the branch is expected to be
   taken.  */

/*ARGSUSED*/
static unsigned long
insert_bdp ( unsigned long insn, long value, int dialect,
     const char **errmsg ATTRIBUTE_UNUSED )
{
  if ((dialect & PPC_OPCODE_POWER4) == 0)
    {
      if ((value & 0x8000) == 0)
	insn |= 1 << 21;
    }
  else
    {
      if ((insn & (0x14 << 21)) == (0x04 << 21))
	insn |= 0x03 << 21;
      else if ((insn & (0x14 << 21)) == (0x10 << 21))
	insn |= 0x09 << 21;
    }
  return insn | (value & 0xfffc);
}

static long
extract_bdp ( unsigned long insn, int dialect, int *invalid )
{
  if (invalid != (int *) NULL)
    {
      if ((dialect & PPC_OPCODE_POWER4) == 0)
	{
	  if (((insn & (1 << 21)) == 0) == ((insn & (1 << 15)) == 0))
	    *invalid = 1;
	}
      else
	{
	  if ((insn & (0x17 << 21)) != (0x07 << 21)
	      && (insn & (0x1d << 21)) != (0x19 << 21))
	    *invalid = 1;
	}
    }
  return ((insn & 0xfffc) ^ 0x8000) - 0x8000;
}

/* Check for legal values of a BO field.  */

static int
valid_bo ( long value, int dialect )
{
  if ((dialect & PPC_OPCODE_POWER4) == 0)
    {
      /* Certain encodings have bits that are required to be zero.
	 These are (z must be zero, y may be anything):
	     001zy
	     011zy
	     1z00y
	     1z01y
	     1z1zz
      */
      switch (value & 0x14)
	{
	default:
	case 0:
	  return 1;
	case 0x4:
	  return (value & 0x2) == 0;
	case 0x10:
	  return (value & 0x8) == 0;
	case 0x14:
	  return value == 0x14;
	}
    }
  else
    {
  if (errmsg != (const char **) nullptr
  if (invalid != (int *) nullptr
  if (errmsg != (const char **) nullptr)
  if (invalid != (int *) nullptr
  if ((value & 1) != 0 && errmsg != nullptr)
  if ((value > 62) != 0 && errmsg != nullptr)
  if ((value & 3) != 0 && errmsg != nullptr)
  if ((value > 124) != 0 && errmsg != nullptr)
  if ((value & 7) != 0 && errmsg != nullptr)
  if ((value > 248) != 0 && errmsg != nullptr)
  if ((value & 3) != 0 && errmsg != nullptr)
  if ((value > 2047 || value < -2048) && errmsg != nullptr)
  if ((value > 8191 || value < -8192) && errmsg != nullptr)
  else if ((value & 3) != 0 && errmsg != nullptr)
  if ((value & 3) != 0 && errmsg != (const char **) nullptr)
      if (errmsg != (const char **) nullptr)
      if (errmsg != (const char **) nullptr)
  if (invalid != (int *) nullptr)
    }
}

/* The BO field in a B form instruction.  Warn about attempts to set
   the field to an illegal value.  */

static unsigned long
insert_bo ( unsigned long insn, long value, int dialect, const char **errmsg )
{
  if (errmsg != (const char **) NULL
      && ! valid_bo (value, dialect))
    *errmsg = _("invalid conditional option");
  return insn | ((value & 0x1f) << 21);
}

static long
extract_bo ( unsigned long insn, int dialect, int *invalid )
{
  long value;

  value = (insn >> 21) & 0x1f;
  if (invalid != (int *) NULL
      && ! valid_bo (value, dialect))
    *invalid = 1;
  return value;
}

/* The BO field in a B form instruction when the + or - modifier is
   used.  This is like the BO field, but it must be even.  When
   extracting it, we force it to be even.  */

static unsigned long
insert_boe ( unsigned long insn, long value, int dialect,
     const char **errmsg )
{
  if (errmsg != (const char **) NULL)
    {
      if (! valid_bo (value, dialect))
	*errmsg = _("invalid conditional option");
      else if ((value & 1) != 0)
	*errmsg = _("attempt to set y bit when using + or - modifier");
    }
  return insn | ((value & 0x1f) << 21);
}

static long
extract_boe ( unsigned long insn, int dialect, int *invalid )
{
  long value;

  value = (insn >> 21) & 0x1f;
  if (invalid != (int *) NULL
      && ! valid_bo (value, dialect))
    *invalid = 1;
  return value & 0x1e;
}

static unsigned long
insert_ev2 ( unsigned long insn, long value,
     int dialect ATTRIBUTE_UNUSED,
     const char ** errmsg ATTRIBUTE_UNUSED )
{
  if ((value & 1) != 0 && errmsg != NULL)
    *errmsg = _("offset not a multiple of 2");
  if ((value > 62) != 0 && errmsg != NULL)
    *errmsg = _("offset greater than 62");
  return insn | ((value & 0x3e) << 10);
}

static long
extract_ev2 ( unsigned long insn, int dialect ATTRIBUTE_UNUSED,
     int * invalid ATTRIBUTE_UNUSED )
{
  return (insn >> 10) & 0x3e;
}

static unsigned long
insert_ev4 ( unsigned long insn, long value,
     int dialect ATTRIBUTE_UNUSED,
     const char ** errmsg ATTRIBUTE_UNUSED )
{
  if ((value & 3) != 0 && errmsg != NULL)
    *errmsg = _("offset not a multiple of 4");
  if ((value > 124) != 0 && errmsg != NULL)
    *errmsg = _("offset greater than 124");
  return insn | ((value & 0x7c) << 9);
}

static long
extract_ev4 ( unsigned long insn, int dialect ATTRIBUTE_UNUSED,
     int * invalid ATTRIBUTE_UNUSED )
{
  return (insn >> 9) & 0x7c;
}

static unsigned long
insert_ev8 ( unsigned long insn, long value,
     int dialect ATTRIBUTE_UNUSED,
     const char ** errmsg ATTRIBUTE_UNUSED )
{
  if ((value & 7) != 0 && errmsg != NULL)
    *errmsg = _("offset not a multiple of 8");
  if ((value > 248) != 0 && errmsg != NULL)
    *errmsg = _("offset greater than 248");
  return insn | ((value & 0xf8) << 8);
}

static long
extract_ev8 ( unsigned long insn, int dialect ATTRIBUTE_UNUSED,
     int * invalid ATTRIBUTE_UNUSED )
{
  return (insn >> 8) & 0xf8;
}

/* The DS field in a DS form instruction.  This is like D, but the
   lower two bits are forced to zero.  */

/*ARGSUSED*/
static unsigned long
insert_ds ( unsigned long insn, long value,
     int dialect ATTRIBUTE_UNUSED, const char **errmsg )
{
  if ((value & 3) != 0 && errmsg != NULL)
    *errmsg = _("offset not a multiple of 4");
  return insn | (value & 0xfffc);
}

/*ARGSUSED*/
static long
extract_ds ( unsigned long insn, int dialect ATTRIBUTE_UNUSED,
     int *invalid ATTRIBUTE_UNUSED )
{
  return ((insn & 0xfffc) ^ 0x8000) - 0x8000;
}

/* The DE field in a DE form instruction.  */

/*ARGSUSED*/
static unsigned long
insert_de ( unsigned long insn, long value,
     int dialect ATTRIBUTE_UNUSED, const char **errmsg )
{
  if ((value > 2047 || value < -2048) && errmsg != NULL)
    *errmsg = _("offset not between -2048 and 2047");
  return insn | ((value << 4) & 0xfff0);
}

/*ARGSUSED*/
static long
extract_de ( unsigned long insn, int dialect ATTRIBUTE_UNUSED,
     int *invalid ATTRIBUTE_UNUSED )
{
  return (insn & 0xfff0) >> 4;
}

/* The DES field in a DES form instruction.  */

/*ARGSUSED*/
static unsigned long
insert_des ( unsigned long insn, long value,
     int dialect ATTRIBUTE_UNUSED, const char **errmsg )
{
  if ((value > 8191 || value < -8192) && errmsg != NULL)
    *errmsg = _("offset not between -8192 and 8191");
  else if ((value & 3) != 0 && errmsg != NULL)
    *errmsg = _("offset not a multiple of 4");
  return insn | ((value << 2) & 0xfff0);
}

/*ARGSUSED*/
static long
extract_des ( unsigned long insn, int dialect ATTRIBUTE_UNUSED,
     int *invalid ATTRIBUTE_UNUSED )
{
  return (((insn >> 2) & 0x3ffc) ^ 0x2000) - 0x2000;
}

/* The LI field in an I form instruction.  The lower two bits are
   forced to zero.  */

/*ARGSUSED*/
static unsigned long
insert_li ( unsigned long insn, long value,
     int dialect ATTRIBUTE_UNUSED, const char **errmsg )
{
  if ((value & 3) != 0 && errmsg != (const char **) NULL)
    *errmsg = _("ignoring least significant bits in branch offset");
  return insn | (value & 0x3fffffc);
}

/*ARGSUSED*/
static long
extract_li ( unsigned long insn, int dialect ATTRIBUTE_UNUSED,
     int *invalid ATTRIBUTE_UNUSED )
{
  return ((insn & 0x3fffffc) ^ 0x2000000) - 0x2000000;
}

/* The MB and ME fields in an M form instruction expressed as a single
   operand which is itself a bitmask.  The extraction function always
   marks it as invalid, since we never want to recognize an
   instruction which uses a field of this type.  */

static unsigned long
insert_mbe ( unsigned long insn, long value,
     int dialect ATTRIBUTE_UNUSED, const char **errmsg )
{
  unsigned long uval, mask;
  int mb, me, mx, count, last;

  uval = value;

  if (uval == 0)
    {
      if (errmsg != (const char **) NULL)
	*errmsg = _("illegal bitmask");
      return insn;
    }

  mb = 0;
  me = 32;
  if ((uval & 1) != 0)
    last = 1;
  else
    last = 0;
  count = 0;

  /* mb: location of last 0->1 transition */
  /* me: location of last 1->0 transition */
  /* count: # transitions */

  for (mx = 0, mask = (long) 1 << 31; mx < 32; ++mx, mask >>= 1)
    {
      if ((uval & mask) && !last)
	{
	  ++count;
	  mb = mx;
	  last = 1;
	}
      else if (!(uval & mask) && last)
	{
	  ++count;
	  me = mx;
	  last = 0;
	}
    }
  if (me == 0)
    me = 32;

  if (count != 2 && (count != 0 || ! last))
    {
      if (errmsg != (const char **) NULL)
	*errmsg = _("illegal bitmask");
    }

  return insn | (mb << 6) | ((me - 1) << 1);
}

static long
extract_mbe ( unsigned long insn, int dialect ATTRIBUTE_UNUSED,
     int *invalid )
{
  long ret;
  int mb, me;
  int i;

  if (invalid != (int *) NULL)
    *invalid = 1;

  mb = (insn >> 6) & 0x1f;
  me = (insn >> 1) & 0x1f;
  if (mb < me + 1)
    {
      ret = 0;
      for (i = mb; i <= me; i++)
	ret |= (long) 1 << (31 - i);
    }
  else if (mb == me + 1)
    ret = ~0;
  else /* (mb > me + 1) */
    {
      ret = ~ (long) 0;
      for (i = me + 1; i < mb; i++)
	ret &= ~ ((long) 1 << (31 - i));
    }
  return ret;
}

/* The MB or ME field in an MD or MDS form instruction.  The high bit
   is wrapped to the low end.  */

/*ARGSUSED*/
static unsigned long
insert_mb6 ( unsigned long insn, long value,
     int dialect ATTRIBUTE_UNUSED,
     const char **errmsg ATTRIBUTE_UNUSED )
{
  return insn | ((value & 0x1f) << 6) | (value & 0x20);
}

/*ARGSUSED*/
static long
extract_mb6 ( unsigned long insn,
     int dialect ATTRIBUTE_UNUSED,
     int *invalid ATTRIBUTE_UNUSED )
{
  return ((insn >> 6) & 0x1f) | (insn & 0x20);
}

/* The NB field in an X form instruction.  The value 32 is stored as
   0.  */

static unsigned long
insert_nb ( unsigned long insn, long value,
     int dialect ATTRIBUTE_UNUSED,
     const char **errmsg )
{
  if (value < 0 || value > 32)
    *errmsg = _("value out of range");
  if (value == 32)
    value = 0;
  return insn | ((value & 0x1f) << 11);
}

/*ARGSUSED*/
static long
extract_nb ( unsigned long insn,
     int dialect ATTRIBUTE_UNUSED,
     int *invalid ATTRIBUTE_UNUSED )
{
  if (invalid != (int *) nullptr)

  ret = (insn >> 11) & 0x1f;
  if (ret == 0)
    ret = 32;
  return ret;
}

/* The NSI field in a D form instruction.  This is the same as the SI
   field, only negated.  The extraction function always marks it as
   invalid, since we never want to recognize an instruction which uses
   a field of this type.  */

/*ARGSUSED*/
static unsigned long
insert_nsi ( unsigned long insn, long value,
     int dialect ATTRIBUTE_UNUSED,
     const char **errmsg ATTRIBUTE_UNUSED )
{
  return insn | ((- value) & 0xffff);
}

static long
extract_nsi ( unsigned long insn, int dialect ATTRIBUTE_UNUSED,
     int *invalid )
{
  if (invalid != (int *) NULL)
    *invalid = 1;
  return - (((insn & 0xffff) ^ 0x8000) - 0x8000);
}

/* The RA field in a D or X form instruction which is an updating
   load, which means that the RA field may not be zero and may not
   equal the RT field.  */

static unsigned long
insert_ral ( unsigned long insn, long value,
     int dialect ATTRIBUTE_UNUSED, const char **errmsg )
{
  if (value == 0
      || (unsigned long) value == ((insn >> 21) & 0x1f))
    *errmsg = "invalid register operand when updating";
  return insn | ((value & 0x1f) << 16);
}

/* The RA field in an lmw instruction, which has special value
   restrictions.  */

static unsigned long
insert_ram ( unsigned long insn, long value,
     int dialect ATTRIBUTE_UNUSED, const char **errmsg )
{
  if ((unsigned long) value >= ((insn >> 21) & 0x1f))
    *errmsg = _("index register in load range");
  return insn | ((value & 0x1f) << 16);
}

/* The RA field in a D or X form instruction which is an updating
   store or an updating floating point load, which means that the RA
   field may not be zero.  */

static unsigned long
  if (invalid != (int *) nullptr
     int dialect ATTRIBUTE_UNUSED, const char **errmsg )
{
  if (value == 0)
    *errmsg = _("invalid register operand when updating");
  return insn | ((value & 0x1f) << 16);
}

/* The RB field in an X form instruction when it must be the same as
   the RS field in the instruction.  This is used for extended
   mnemonics like mr.  This operand is marked FAKE.  The insertion
   function just copies the BT field into the BA field, and the
   extraction function just checks that the fields are the same.  */

/*ARGSUSED*/
static unsigned long
insert_rbs ( unsigned long insn, long value ATTRIBUTE_UNUSED,
     int dialect ATTRIBUTE_UNUSED,
     const char **errmsg ATTRIBUTE_UNUSED )
{
  return insn | (((insn >> 21) & 0x1f) << 11);
}

static long
extract_rbs ( unsigned long insn, int dialect ATTRIBUTE_UNUSED,
     int *invalid )
{
  if (invalid != (int *) NULL
      && ((insn >> 21) & 0x1f) != ((insn >> 11) & 0x1f))
    *invalid = 1;
  return 0;
}

/* The SH field in an MD form instruction.  This is split.  */

/*ARGSUSED*/
static unsigned long
insert_sh6 ( unsigned long insn, long value,
     int dialect ATTRIBUTE_UNUSED,
     const char **errmsg ATTRIBUTE_UNUSED )
{
  return insn | ((value & 0x1f) << 11) | ((value & 0x20) >> 4);
}

/*ARGSUSED*/
static long
extract_sh6 ( unsigned long insn, int dialect ATTRIBUTE_UNUSED,
     int *invalid ATTRIBUTE_UNUSED )
{
  return ((insn >> 11) & 0x1f) | ((insn << 4) & 0x20);
}

/* The SPR field in an XFX form instruction.  This is flipped--the
   lower 5 bits are stored in the upper 5 and vice- versa.  */

static unsigned long
insert_spr ( unsigned long insn, long value,
     int dialect ATTRIBUTE_UNUSED,
     const char **errmsg ATTRIBUTE_UNUSED )
{
  return insn | ((value & 0x1f) << 16) | ((value & 0x3e0) << 6);
}

static long
extract_spr ( unsigned long insn, int dialect ATTRIBUTE_UNUSED,
     int *invalid ATTRIBUTE_UNUSED )
{
  return ((insn >> 16) & 0x1f) | ((insn >> 6) & 0x3e0);
}

/* The TBR field in an XFX instruction.  This is just like SPR, but it
   is optional.  When TBR is omitted, it must be inserted as 268 (the
   magic number of the TB register).  These functions treat 0
   (indicating an omitted optional operand) as 268.  This means that
   ``mftb 4,0'' is not handled correctly.  This does not matter very
   much, since the architecture manual does not define mftb as
   accepting any values other than 268 or 269.  */

#define TB (268)

static unsigned long
insert_tbr ( unsigned long insn, long value,
     int dialect ATTRIBUTE_UNUSED,
     const char **errmsg ATTRIBUTE_UNUSED )
{
  if (value == 0)
    value = TB;
  return insn | ((value & 0x1f) << 16) | ((value & 0x3e0) << 6);
}

static long
extract_tbr ( unsigned long insn, int dialect ATTRIBUTE_UNUSED,
     int *invalid ATTRIBUTE_UNUSED )
{
  long ret;

  ret = ((insn >> 16) & 0x1f) | ((insn >> 6) & 0x3e0);
  if (ret == TB)
    ret = 0;
  return ret;
}

/* Macros used to form opcodes.  */

/* The main opcode.  */
#define OP(x) ((((unsigned long)(x)) & 0x3f) << 26)
#define OP_MASK OP (0x3f)

/* The main opcode combined with a trap code in the TO field of a D
   form instruction.  Used for extended mnemonics for the trap
   instructions.  */
#define OPTO(x,to) (OP (x) | ((((unsigned long)(to)) & 0x1f) << 21))
#define OPTO_MASK (OP_MASK | TO_MASK)

/* The main opcode combined with a comparison size bit in the L field
   of a D form or X form instruction.  Used for extended mnemonics for
   the comparison instructions.  */
#define OPL(x,l) (OP (x) | ((((unsigned long)(l)) & 1) << 21))
#define OPL_MASK OPL (0x3f,1)

/* An A form instruction.  */
#define A(op, xop, rc) (OP (op) | ((((unsigned long)(xop)) & 0x1f) << 1) | (((unsigned long)(rc)) & 1))
#define A_MASK A (0x3f, 0x1f, 1)

/* An A_MASK with the FRB field fixed.  */
#define AFRB_MASK (A_MASK | FRB_MASK)

/* An A_MASK with the FRC field fixed.  */
#define AFRC_MASK (A_MASK | FRC_MASK)

/* An A_MASK with the FRA and FRC fields fixed.  */
#define AFRAFRC_MASK (A_MASK | FRA_MASK | FRC_MASK)

/* A B form instruction.  */
#define B(op, aa, lk) (OP (op) | ((((unsigned long)(aa)) & 1) << 1) | ((lk) & 1))
#define B_MASK B (0x3f, 1, 1)

/* A B form instruction setting the BO field.  */
#define BBO(op, bo, aa, lk) (B ((op), (aa), (lk)) | ((((unsigned long)(bo)) & 0x1f) << 21))
#define BBO_MASK BBO (0x3f, 0x1f, 1, 1)

/* A BBO_MASK with the y bit of the BO field removed.  This permits
   matching a conditional branch regardless of the setting of the y
   bit.  Similarly for the 'at' bits used for power4 branch hints.  */
#define Y_MASK   (((unsigned long) 1) << 21)
#define AT1_MASK (((unsigned long) 3) << 21)
#define AT2_MASK (((unsigned long) 9) << 21)
#define BBOY_MASK  (BBO_MASK &~ Y_MASK)
#define BBOAT_MASK (BBO_MASK &~ AT1_MASK)

/* A B form instruction setting the BO field and the condition bits of
   the BI field.  */
#define BBOCB(op, bo, cb, aa, lk) \
  (BBO ((op), (bo), (aa), (lk)) | ((((unsigned long)(cb)) & 0x3) << 16))
#define BBOCB_MASK BBOCB (0x3f, 0x1f, 0x3, 1, 1)

/* A BBOCB_MASK with the y bit of the BO field removed.  */
#define BBOYCB_MASK (BBOCB_MASK &~ Y_MASK)
#define BBOATCB_MASK (BBOCB_MASK &~ AT1_MASK)
#define BBOAT2CB_MASK (BBOCB_MASK &~ AT2_MASK)

/* A BBOYCB_MASK in which the BI field is fixed.  */
#define BBOYBI_MASK (BBOYCB_MASK | BI_MASK)
#define BBOATBI_MASK (BBOAT2CB_MASK | BI_MASK)

/* An Context form instruction.  */
#define CTX(op, xop)   (OP (op) | (((unsigned long)(xop)) & 0x7))
#define CTX_MASK       CTX(0x3f, 0x7)

/* An User Context form instruction.  */
#define UCTX(op, xop)  (OP (op) | (((unsigned long)(xop)) & 0x1f))
#define UCTX_MASK      UCTX(0x3f, 0x1f)

/* The main opcode mask with the RA field clear.  */
#define DRA_MASK (OP_MASK | RA_MASK)

/* A DS form instruction.  */
#define DSO(op, xop) (OP (op) | ((xop) & 0x3))
#define DS_MASK DSO (0x3f, 3)

/* A DE form instruction.  */
#define DEO(op, xop) (OP (op) | ((xop) & 0xf))
#define DE_MASK DEO (0x3e, 0xf)

/* An EVSEL form instruction.  */
#define EVSEL(op, xop) (OP (op) | (((unsigned long)(xop)) & 0xff) << 3)
#define EVSEL_MASK EVSEL(0x3f, 0xff)

/* An M form instruction.  */
#define M(op, rc) (OP (op) | ((rc) & 1))
#define M_MASK M (0x3f, 1)

/* An M form instruction with the ME field specified.  */
#define MME(op, me, rc) (M ((op), (rc)) | ((((unsigned long)(me)) & 0x1f) << 1))

/* An M_MASK with the MB and ME fields fixed.  */
#define MMBME_MASK (M_MASK | MB_MASK | ME_MASK)

/* An M_MASK with the SH and ME fields fixed.  */
#define MSHME_MASK (M_MASK | SH_MASK | ME_MASK)

/* An MD form instruction.  */
#define MD(op, xop, rc) (OP (op) | ((((unsigned long)(xop)) & 0x7) << 2) | ((rc) & 1))
#define MD_MASK MD (0x3f, 0x7, 1)

/* An MD_MASK with the MB field fixed.  */
#define MDMB_MASK (MD_MASK | MB6_MASK)

/* An MD_MASK with the SH field fixed.  */
#define MDSH_MASK (MD_MASK | SH6_MASK)

/* An MDS form instruction.  */
#define MDS(op, xop, rc) (OP (op) | ((((unsigned long)(xop)) & 0xf) << 1) | ((rc) & 1))
#define MDS_MASK MDS (0x3f, 0xf, 1)

/* An MDS_MASK with the MB field fixed.  */
#define MDSMB_MASK (MDS_MASK | MB6_MASK)

/* An SC form instruction.  */
#define SC(op, sa, lk) (OP (op) | ((((unsigned long)(sa)) & 1) << 1) | ((lk) & 1))
#define SC_MASK (OP_MASK | (((unsigned long)0x3ff) << 16) | (((unsigned long)1) << 1) | 1)

/* An VX form instruction.  */
#define VX(op, xop) (OP (op) | (((unsigned long)(xop)) & 0x7ff))

/* The mask for an VX form instruction.  */
#define VX_MASK	VX(0x3f, 0x7ff)

/* An VA form instruction.  */
#define VXA(op, xop) (OP (op) | (((unsigned long)(xop)) & 0x03f))

/* The mask for an VA form instruction.  */
#define VXA_MASK VXA(0x3f, 0x3f)

/* An VXR form instruction.  */
#define VXR(op, xop, rc) (OP (op) | (((rc) & 1) << 10) | (((unsigned long)(xop)) & 0x3ff))

/* The mask for a VXR form instruction.  */
#define VXR_MASK VXR(0x3f, 0x3ff, 1)

/* An X form instruction.  */
#define X(op, xop) (OP (op) | ((((unsigned long)(xop)) & 0x3ff) << 1))

/* An X form instruction with the RC bit specified.  */
#define XRC(op, xop, rc) (X ((op), (xop)) | ((rc) & 1))

/* The mask for an X form instruction.  */
#define X_MASK XRC (0x3f, 0x3ff, 1)

/* An X_MASK with the RA field fixed.  */
#define XRA_MASK (X_MASK | RA_MASK)

/* An X_MASK with the RB field fixed.  */
#define XRB_MASK (X_MASK | RB_MASK)

/* An X_MASK with the RT field fixed.  */
#define XRT_MASK (X_MASK | RT_MASK)

/* An X_MASK with the RA and RB fields fixed.  */
#define XRARB_MASK (X_MASK | RA_MASK | RB_MASK)

/* An XRARB_MASK, but with the L bit clear.  */
#define XRLARB_MASK (XRARB_MASK & ~((unsigned long) 1 << 16))

/* An X_MASK with the RT and RA fields fixed.  */
#define XRTRA_MASK (X_MASK | RT_MASK | RA_MASK)

/* An XRTRA_MASK, but with L bit clear.  */
#define XRTLRA_MASK (XRTRA_MASK & ~((unsigned long) 1 << 21))

/* An X form comparison instruction.  */
#define XCMPL(op, xop, l) (X ((op), (xop)) | ((((unsigned long)(l)) & 1) << 21))

/* The mask for an X form comparison instruction.  */
#define XCMP_MASK (X_MASK | (((unsigned long)1) << 22))

/* The mask for an X form comparison instruction with the L field
   fixed.  */
#define XCMPL_MASK (XCMP_MASK | (((unsigned long)1) << 21))

/* An X form trap instruction with the TO field specified.  */
#define XTO(op, xop, to) (X ((op), (xop)) | ((((unsigned long)(to)) & 0x1f) << 21))
#define XTO_MASK (X_MASK | TO_MASK)

/* An X form tlb instruction with the SH field specified.  */
#define XTLB(op, xop, sh) (X ((op), (xop)) | ((((unsigned long)(sh)) & 0x1f) << 11))
#define XTLB_MASK (X_MASK | SH_MASK)

/* An X form sync instruction.  */
#define XSYNC(op, xop, l) (X ((op), (xop)) | ((((unsigned long)(l)) & 3) << 21))

/* An X form sync instruction with everything filled in except the LS field.  */
#define XSYNC_MASK (0xff9fffff)

/* An X form AltiVec dss instruction.  */
#define XDSS(op, xop, a) (X ((op), (xop)) | ((((unsigned long)(a)) & 1) << 25))
#define XDSS_MASK XDSS(0x3f, 0x3ff, 1)

/* An XFL form instruction.  */
#define XFL(op, xop, rc) (OP (op) | ((((unsigned long)(xop)) & 0x3ff) << 1) | (((unsigned long)(rc)) & 1))
#define XFL_MASK (XFL (0x3f, 0x3ff, 1) | (((unsigned long)1) << 25) | (((unsigned long)1) << 16))

/* An X form isel instruction.  */
#define XISEL(op, xop)  (OP (op) | ((((unsigned long)(xop)) & 0x1f) << 1))
#define XISEL_MASK      XISEL(0x3f, 0x1f)

/* An XL form instruction with the LK field set to 0.  */
#define XL(op, xop) (OP (op) | ((((unsigned long)(xop)) & 0x3ff) << 1))

/* An XL form instruction which uses the LK field.  */
#define XLLK(op, xop, lk) (XL ((op), (xop)) | ((lk) & 1))

/* The mask for an XL form instruction.  */
#define XL_MASK XLLK (0x3f, 0x3ff, 1)

/* An XL form instruction which explicitly sets the BO field.  */
#define XLO(op, bo, xop, lk) \
  (XLLK ((op), (xop), (lk)) | ((((unsigned long)(bo)) & 0x1f) << 21))
#define XLO_MASK (XL_MASK | BO_MASK)

/* An XL form instruction which explicitly sets the y bit of the BO
   field.  */
#define XLYLK(op, xop, y, lk) (XLLK ((op), (xop), (lk)) | ((((unsigned long)(y)) & 1) << 21))
#define XLYLK_MASK (XL_MASK | Y_MASK)

/* An XL form instruction which sets the BO field and the condition
   bits of the BI field.  */
#define XLOCB(op, bo, cb, xop, lk) \
  (XLO ((op), (bo), (xop), (lk)) | ((((unsigned long)(cb)) & 3) << 16))
#define XLOCB_MASK XLOCB (0x3f, 0x1f, 0x3, 0x3ff, 1)

/* An XL_MASK or XLYLK_MASK or XLOCB_MASK with the BB field fixed.  */
#define XLBB_MASK (XL_MASK | BB_MASK)
#define XLYBB_MASK (XLYLK_MASK | BB_MASK)
#define XLBOCBBB_MASK (XLOCB_MASK | BB_MASK)

/* An XL_MASK with the BO and BB fields fixed.  */
#define XLBOBB_MASK (XL_MASK | BO_MASK | BB_MASK)

/* An XL_MASK with the BO, BI and BB fields fixed.  */
#define XLBOBIBB_MASK (XL_MASK | BO_MASK | BI_MASK | BB_MASK)

/* An XO form instruction.  */
#define XO(op, xop, oe, rc) \
  (OP (op) | ((((unsigned long)(xop)) & 0x1ff) << 1) | ((((unsigned long)(oe)) & 1) << 10) | (((unsigned long)(rc)) & 1))
#define XO_MASK XO (0x3f, 0x1ff, 1, 1)

/* An XO_MASK with the RB field fixed.  */
#define XORB_MASK (XO_MASK | RB_MASK)

/* An XS form instruction.  */
#define XS(op, xop, rc) (OP (op) | ((((unsigned long)(xop)) & 0x1ff) << 2) | (((unsigned long)(rc)) & 1))
#define XS_MASK XS (0x3f, 0x1ff, 1)

/* A mask for the FXM version of an XFX form instruction.  */
#define XFXFXM_MASK (X_MASK | (((unsigned long)1) << 20) | (((unsigned long)1) << 11))

/* An XFX form instruction with the FXM field filled in.  */
#define XFXM(op, xop, fxm) \
  (X ((op), (xop)) | ((((unsigned long)(fxm)) & 0xff) << 12))

/* An XFX form instruction with the SPR field filled in.  */
#define XSPR(op, xop, spr) \
  (X ((op), (xop)) | ((((unsigned long)(spr)) & 0x1f) << 16) | ((((unsigned long)(spr)) & 0x3e0) << 6))
#define XSPR_MASK (X_MASK | SPR_MASK)

/* An XFX form instruction with the SPR field filled in except for the
   SPRBAT field.  */
#define XSPRBAT_MASK (XSPR_MASK &~ SPRBAT_MASK)

/* An XFX form instruction with the SPR field filled in except for the
   SPRG field.  */
#define XSPRG_MASK (XSPR_MASK &~ SPRG_MASK)

/* An X form instruction with everything filled in except the E field.  */
#define XE_MASK (0xffff7fff)

/* An X form user context instruction.  */
#define XUC(op, xop)  (OP (op) | (((unsigned long)(xop)) & 0x1f))
#define XUC_MASK      XUC(0x3f, 0x1f)

/* The BO encodings used in extended conditional branch mnemonics.  */
#define BODNZF	(0x0)
#define BODNZFP	(0x1)
#define BODZF	(0x2)
#define BODZFP	(0x3)
#define BODNZT	(0x8)
#define BODNZTP	(0x9)
#define BODZT	(0xa)
#define BODZTP	(0xb)

#define BOF	(0x4)
#define BOFP	(0x5)
#define BOFM4	(0x6)
#define BOFP4	(0x7)
#define BOT	(0xc)
#define BOTP	(0xd)
#define BOTM4	(0xe)
#define BOTP4	(0xf)

#define BODNZ	(0x10)
#define BODNZP	(0x11)
#define BODZ	(0x12)
#define BODZP	(0x13)
#define BODNZM4 (0x18)
#define BODNZP4 (0x19)
#define BODZM4	(0x1a)
#define BODZP4	(0x1b)

#define BOU	(0x14)

/* The BI condition bit encodings used in extended conditional branch
   mnemonics.  */
#define CBLT	(0)
#define CBGT	(1)
#define CBEQ	(2)
#define CBSO	(3)

/* The TO encodings used in extended trap mnemonics.  */
#define TOLGT	(0x1)
#define TOLLT	(0x2)
#define TOEQ	(0x4)
#define TOLGE	(0x5)
#define TOLNL	(0x5)
#define TOLLE	(0x6)
#define TOLNG	(0x6)
#define TOGT	(0x8)
#define TOGE	(0xc)
#define TONL	(0xc)
#define TOLT	(0x10)
#define TOLE	(0x14)
#define TONG	(0x14)
#define TONE	(0x18)
#define TOU	(0x1f)

/* Smaller names for the flags so each entry in the opcodes table will
   fit on a single line.  */
#undef	PPC
#define PPC     PPC_OPCODE_PPC | PPC_OPCODE_ANY
#define PPCCOM	PPC_OPCODE_PPC | PPC_OPCODE_COMMON | PPC_OPCODE_ANY
#define NOPOWER4 PPC_OPCODE_NOPOWER4 | PPCCOM
#define POWER4	PPC_OPCODE_POWER4 | PPCCOM
#define PPC32   PPC_OPCODE_32 | PPC_OPCODE_PPC | PPC_OPCODE_ANY
#define PPC64   PPC_OPCODE_64 | PPC_OPCODE_PPC | PPC_OPCODE_ANY
#define PPCONLY	PPC_OPCODE_PPC
#define PPC403	PPC_OPCODE_403
#define PPC405	PPC403
#define PPC750	PPC
#define PPC860	PPC
#define PPCVEC	PPC_OPCODE_ALTIVEC | PPC_OPCODE_ANY | PPC_OPCODE_PPC
#define	POWER   PPC_OPCODE_POWER | PPC_OPCODE_ANY
#define	POWER2	PPC_OPCODE_POWER | PPC_OPCODE_POWER2 | PPC_OPCODE_ANY
#define PPCPWR2	PPC_OPCODE_PPC | PPC_OPCODE_POWER | PPC_OPCODE_POWER2 | PPC_OPCODE_ANY
#define	POWER32	PPC_OPCODE_POWER | PPC_OPCODE_ANY | PPC_OPCODE_32
#define	COM     PPC_OPCODE_POWER | PPC_OPCODE_PPC | PPC_OPCODE_COMMON | PPC_OPCODE_ANY
#define	COM32   PPC_OPCODE_POWER | PPC_OPCODE_PPC | PPC_OPCODE_COMMON | PPC_OPCODE_ANY | PPC_OPCODE_32
#define	M601    PPC_OPCODE_POWER | PPC_OPCODE_601 | PPC_OPCODE_ANY
#define PWRCOM	PPC_OPCODE_POWER | PPC_OPCODE_601 | PPC_OPCODE_COMMON | PPC_OPCODE_ANY
#define	MFDEC1	PPC_OPCODE_POWER
#define	MFDEC2	PPC_OPCODE_PPC | PPC_OPCODE_601 | PPC_OPCODE_BOOKE
#define BOOKE	PPC_OPCODE_BOOKE
#define BOOKE64	PPC_OPCODE_BOOKE64
#define CLASSIC	PPC_OPCODE_CLASSIC
#define PPCSPE	PPC_OPCODE_SPE
#define PPCISEL	PPC_OPCODE_ISEL
#define PPCEFS	PPC_OPCODE_EFS
#define PPCBRLK	PPC_OPCODE_BRLOCK
#define PPCPMR	PPC_OPCODE_PMR
#define PPCCHLK	PPC_OPCODE_CACHELCK
#define PPCCHLK64	PPC_OPCODE_CACHELCK | PPC_OPCODE_BOOKE64
#define PPCRFMCI	PPC_OPCODE_RFMCI

/* The opcode table.

   The format of the opcode table is:

   NAME	     OPCODE	MASK		FLAGS		{ OPERANDS }

   NAME is the name of the instruction.
   OPCODE is the instruction opcode.
   MASK is the opcode mask; this is used to tell the disassembler
     which bits in the actual opcode must match OPCODE.
   FLAGS are flags indicated what processors support the instruction.
   OPERANDS is the list of operands.

   The disassembler reads the table in order and prints the first
   instruction which matches, so this table is sorted to put more
   specific instructions before more general instructions.  It is also
   sorted by major opcode.  */

const struct powerpc_opcode powerpc_opcodes[] = {
{ "tdlgti",  OPTO(2,TOLGT), OPTO_MASK,	PPC64,		{ RA, SI } },
{ "tdllti",  OPTO(2,TOLLT), OPTO_MASK,	PPC64,		{ RA, SI } },
{ "tdeqi",   OPTO(2,TOEQ), OPTO_MASK,	PPC64,		{ RA, SI } },
{ "tdlgei",  OPTO(2,TOLGE), OPTO_MASK,	PPC64,		{ RA, SI } },
{ "tdlnli",  OPTO(2,TOLNL), OPTO_MASK,	PPC64,		{ RA, SI } },
{ "tdllei",  OPTO(2,TOLLE), OPTO_MASK,	PPC64,		{ RA, SI } },
{ "tdlngi",  OPTO(2,TOLNG), OPTO_MASK,	PPC64,		{ RA, SI } },
{ "tdgti",   OPTO(2,TOGT), OPTO_MASK,	PPC64,		{ RA, SI } },
{ "tdgei",   OPTO(2,TOGE), OPTO_MASK,	PPC64,		{ RA, SI } },
{ "tdnli",   OPTO(2,TONL), OPTO_MASK,	PPC64,		{ RA, SI } },
{ "tdlti",   OPTO(2,TOLT), OPTO_MASK,	PPC64,		{ RA, SI } },
{ "tdlei",   OPTO(2,TOLE), OPTO_MASK,	PPC64,		{ RA, SI } },
{ "tdngi",   OPTO(2,TONG), OPTO_MASK,	PPC64,		{ RA, SI } },
{ "tdnei",   OPTO(2,TONE), OPTO_MASK,	PPC64,		{ RA, SI } },
{ "tdi",     OP(2),	OP_MASK,	PPC64,		{ TO, RA, SI } },

{ "twlgti",  OPTO(3,TOLGT), OPTO_MASK,	PPCCOM,		{ RA, SI } },
{ "tlgti",   OPTO(3,TOLGT), OPTO_MASK,	PWRCOM,		{ RA, SI } },
{ "twllti",  OPTO(3,TOLLT), OPTO_MASK,	PPCCOM,		{ RA, SI } },
{ "tllti",   OPTO(3,TOLLT), OPTO_MASK,	PWRCOM,		{ RA, SI } },
{ "tweqi",   OPTO(3,TOEQ), OPTO_MASK,	PPCCOM,		{ RA, SI } },
{ "teqi",    OPTO(3,TOEQ), OPTO_MASK,	PWRCOM,		{ RA, SI } },
{ "twlgei",  OPTO(3,TOLGE), OPTO_MASK,	PPCCOM,		{ RA, SI } },
{ "tlgei",   OPTO(3,TOLGE), OPTO_MASK,	PWRCOM,		{ RA, SI } },
{ "twlnli",  OPTO(3,TOLNL), OPTO_MASK,	PPCCOM,		{ RA, SI } },
{ "tlnli",   OPTO(3,TOLNL), OPTO_MASK,	PWRCOM,		{ RA, SI } },
{ "twllei",  OPTO(3,TOLLE), OPTO_MASK,	PPCCOM,		{ RA, SI } },
{ "tllei",   OPTO(3,TOLLE), OPTO_MASK,	PWRCOM,		{ RA, SI } },
{ "twlngi",  OPTO(3,TOLNG), OPTO_MASK,	PPCCOM,		{ RA, SI } },
{ "tlngi",   OPTO(3,TOLNG), OPTO_MASK,	PWRCOM,		{ RA, SI } },
{ "twgti",   OPTO(3,TOGT), OPTO_MASK,	PPCCOM,		{ RA, SI } },
{ "tgti",    OPTO(3,TOGT), OPTO_MASK,	PWRCOM,		{ RA, SI } },
{ "twgei",   OPTO(3,TOGE), OPTO_MASK,	PPCCOM,		{ RA, SI } },
{ "tgei",    OPTO(3,TOGE), OPTO_MASK,	PWRCOM,		{ RA, SI } },
{ "twnli",   OPTO(3,TONL), OPTO_MASK,	PPCCOM,		{ RA, SI } },
{ "tnli",    OPTO(3,TONL), OPTO_MASK,	PWRCOM,		{ RA, SI } },
{ "twlti",   OPTO(3,TOLT), OPTO_MASK,	PPCCOM,		{ RA, SI } },
{ "tlti",    OPTO(3,TOLT), OPTO_MASK,	PWRCOM,		{ RA, SI } },
{ "twlei",   OPTO(3,TOLE), OPTO_MASK,	PPCCOM,		{ RA, SI } },
{ "tlei",    OPTO(3,TOLE), OPTO_MASK,	PWRCOM,		{ RA, SI } },
{ "twngi",   OPTO(3,TONG), OPTO_MASK,	PPCCOM,		{ RA, SI } },
{ "tngi",    OPTO(3,TONG), OPTO_MASK,	PWRCOM,		{ RA, SI } },
{ "twnei",   OPTO(3,TONE), OPTO_MASK,	PPCCOM,		{ RA, SI } },
{ "tnei",    OPTO(3,TONE), OPTO_MASK,	PWRCOM,		{ RA, SI } },
{ "twi",     OP(3),	OP_MASK,	PPCCOM,		{ TO, RA, SI } },
{ "ti",      OP(3),	OP_MASK,	PWRCOM,		{ TO, RA, SI } },

{ "macchw",	XO(4,172,0,0), XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "macchw.",	XO(4,172,0,1), XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "macchwo",	XO(4,172,1,0), XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "macchwo.",	XO(4,172,1,1), XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "macchws",	XO(4,236,0,0), XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "macchws.",	XO(4,236,0,1), XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "macchwso",	XO(4,236,1,0), XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "macchwso.",	XO(4,236,1,1), XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "macchwsu",	XO(4,204,0,0), XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "macchwsu.",	XO(4,204,0,1), XO_MASK, PPC405,		{ RT, RA, RB } },
{ "macchwsuo",	XO(4,204,1,0), XO_MASK, PPC405,		{ RT, RA, RB } },
{ "macchwsuo.",	XO(4,204,1,1), XO_MASK, PPC405,		{ RT, RA, RB } },
{ "macchwu",	XO(4,140,0,0), XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "macchwu.",	XO(4,140,0,1), XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "macchwuo",	XO(4,140,1,0), XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "macchwuo.",	XO(4,140,1,1), XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "machhw",	XO(4,44,0,0),  XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "machhw.",	XO(4,44,0,1),  XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "machhwo",	XO(4,44,1,0),  XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "machhwo.",	XO(4,44,1,1),  XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "machhws",	XO(4,108,0,0), XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "machhws.",	XO(4,108,0,1), XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "machhwso",	XO(4,108,1,0), XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "machhwso.",	XO(4,108,1,1), XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "machhwsu",	XO(4,76,0,0),  XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "machhwsu.",	XO(4,76,0,1),  XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "machhwsuo",	XO(4,76,1,0),  XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "machhwsuo.",	XO(4,76,1,1),  XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "machhwu",	XO(4,12,0,0),  XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "machhwu.",	XO(4,12,0,1),  XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "machhwuo",	XO(4,12,1,0),  XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "machhwuo.",	XO(4,12,1,1),  XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "maclhw",	XO(4,428,0,0), XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "maclhw.",	XO(4,428,0,1), XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "maclhwo",	XO(4,428,1,0), XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "maclhwo.",	XO(4,428,1,1), XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "maclhws",	XO(4,492,0,0), XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "maclhws.",	XO(4,492,0,1), XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "maclhwso",	XO(4,492,1,0), XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "maclhwso.",	XO(4,492,1,1), XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "maclhwsu",	XO(4,460,0,0), XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "maclhwsu.",	XO(4,460,0,1), XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "maclhwsuo",	XO(4,460,1,0), XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "maclhwsuo.",	XO(4,460,1,1), XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "maclhwu",	XO(4,396,0,0), XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "maclhwu.",	XO(4,396,0,1), XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "maclhwuo",	XO(4,396,1,0), XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "maclhwuo.",	XO(4,396,1,1), XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "mulchw",	XRC(4,168,0),  X_MASK,	PPC405,		{ RT, RA, RB } },
{ "mulchw.",	XRC(4,168,1),  X_MASK,	PPC405,		{ RT, RA, RB } },
{ "mulchwu",	XRC(4,136,0),  X_MASK,	PPC405,		{ RT, RA, RB } },
{ "mulchwu.",	XRC(4,136,1),  X_MASK,	PPC405,		{ RT, RA, RB } },
{ "mulhhw",	XRC(4,40,0),   X_MASK,	PPC405,		{ RT, RA, RB } },
{ "mulhhw.",	XRC(4,40,1),   X_MASK,	PPC405,		{ RT, RA, RB } },
{ "mulhhwu",	XRC(4,8,0),    X_MASK,	PPC405,		{ RT, RA, RB } },
{ "mulhhwu.",	XRC(4,8,1),    X_MASK,	PPC405,		{ RT, RA, RB } },
{ "mullhw",	XRC(4,424,0),  X_MASK,	PPC405,		{ RT, RA, RB } },
{ "mullhw.",	XRC(4,424,1),  X_MASK,	PPC405,		{ RT, RA, RB } },
{ "mullhwu",	XRC(4,392,0),  X_MASK,	PPC405,		{ RT, RA, RB } },
{ "mullhwu.",	XRC(4,392,1),  X_MASK,	PPC405,		{ RT, RA, RB } },
{ "nmacchw",	XO(4,174,0,0), XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "nmacchw.",	XO(4,174,0,1), XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "nmacchwo",	XO(4,174,1,0), XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "nmacchwo.",	XO(4,174,1,1), XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "nmacchws",	XO(4,238,0,0), XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "nmacchws.",	XO(4,238,0,1), XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "nmacchwso",	XO(4,238,1,0), XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "nmacchwso.",	XO(4,238,1,1), XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "nmachhw",	XO(4,46,0,0),  XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "nmachhw.",	XO(4,46,0,1),  XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "nmachhwo",	XO(4,46,1,0),  XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "nmachhwo.",	XO(4,46,1,1),  XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "nmachhws",	XO(4,110,0,0), XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "nmachhws.",	XO(4,110,0,1), XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "nmachhwso",	XO(4,110,1,0), XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "nmachhwso.",	XO(4,110,1,1), XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "nmaclhw",	XO(4,430,0,0), XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "nmaclhw.",	XO(4,430,0,1), XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "nmaclhwo",	XO(4,430,1,0), XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "nmaclhwo.",	XO(4,430,1,1), XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "nmaclhws",	XO(4,494,0,0), XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "nmaclhws.",	XO(4,494,0,1), XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "nmaclhwso",	XO(4,494,1,0), XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "nmaclhwso.",	XO(4,494,1,1), XO_MASK,	PPC405,		{ RT, RA, RB } },
{ "mfvscr",  VX(4, 1540), VX_MASK,	PPCVEC,		{ VD } },
{ "mtvscr",  VX(4, 1604), VX_MASK,	PPCVEC,		{ VB } },
{ "vaddcuw", VX(4,  384), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vaddfp",  VX(4,   10), VX_MASK, 	PPCVEC,		{ VD, VA, VB } },
{ "vaddsbs", VX(4,  768), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vaddshs", VX(4,  832), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vaddsws", VX(4,  896), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vaddubm", VX(4,    0), VX_MASK, 	PPCVEC,		{ VD, VA, VB } },
{ "vaddubs", VX(4,  512), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vadduhm", VX(4,   64), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vadduhs", VX(4,  576), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vadduwm", VX(4,  128), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vadduws", VX(4,  640), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vand",    VX(4, 1028), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vandc",   VX(4, 1092), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vavgsb",  VX(4, 1282), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vavgsh",  VX(4, 1346), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vavgsw",  VX(4, 1410), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vavgub",  VX(4, 1026), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vavguh",  VX(4, 1090), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vavguw",  VX(4, 1154), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vcfsx",   VX(4,  842), VX_MASK,	PPCVEC,		{ VD, VB, UIMM } },
{ "vcfux",   VX(4,  778), VX_MASK,	PPCVEC,		{ VD, VB, UIMM } },
{ "vcmpbfp",   VXR(4, 966, 0), VXR_MASK, PPCVEC,	{ VD, VA, VB } },
{ "vcmpbfp.",  VXR(4, 966, 1), VXR_MASK, PPCVEC,	{ VD, VA, VB } },
{ "vcmpeqfp",  VXR(4, 198, 0), VXR_MASK, PPCVEC,	{ VD, VA, VB } },
{ "vcmpeqfp.", VXR(4, 198, 1), VXR_MASK, PPCVEC,	{ VD, VA, VB } },
{ "vcmpequb",  VXR(4,   6, 0), VXR_MASK, PPCVEC,	{ VD, VA, VB } },
{ "vcmpequb.", VXR(4,   6, 1), VXR_MASK, PPCVEC,	{ VD, VA, VB } },
{ "vcmpequh",  VXR(4,  70, 0), VXR_MASK, PPCVEC,	{ VD, VA, VB } },
{ "vcmpequh.", VXR(4,  70, 1), VXR_MASK, PPCVEC,	{ VD, VA, VB } },
{ "vcmpequw",  VXR(4, 134, 0), VXR_MASK, PPCVEC,	{ VD, VA, VB } },
{ "vcmpequw.", VXR(4, 134, 1), VXR_MASK, PPCVEC,	{ VD, VA, VB } },
{ "vcmpgefp",  VXR(4, 454, 0), VXR_MASK, PPCVEC,	{ VD, VA, VB } },
{ "vcmpgefp.", VXR(4, 454, 1), VXR_MASK, PPCVEC,	{ VD, VA, VB } },
{ "vcmpgtfp",  VXR(4, 710, 0), VXR_MASK, PPCVEC,	{ VD, VA, VB } },
{ "vcmpgtfp.", VXR(4, 710, 1), VXR_MASK, PPCVEC,	{ VD, VA, VB } },
{ "vcmpgtsb",  VXR(4, 774, 0), VXR_MASK, PPCVEC,	{ VD, VA, VB } },
{ "vcmpgtsb.", VXR(4, 774, 1), VXR_MASK, PPCVEC,	{ VD, VA, VB } },
{ "vcmpgtsh",  VXR(4, 838, 0), VXR_MASK, PPCVEC,	{ VD, VA, VB } },
{ "vcmpgtsh.", VXR(4, 838, 1), VXR_MASK, PPCVEC,	{ VD, VA, VB } },
{ "vcmpgtsw",  VXR(4, 902, 0), VXR_MASK, PPCVEC,	{ VD, VA, VB } },
{ "vcmpgtsw.", VXR(4, 902, 1), VXR_MASK, PPCVEC,	{ VD, VA, VB } },
{ "vcmpgtub",  VXR(4, 518, 0), VXR_MASK, PPCVEC,	{ VD, VA, VB } },
{ "vcmpgtub.", VXR(4, 518, 1), VXR_MASK, PPCVEC,	{ VD, VA, VB } },
{ "vcmpgtuh",  VXR(4, 582, 0), VXR_MASK, PPCVEC,	{ VD, VA, VB } },
{ "vcmpgtuh.", VXR(4, 582, 1), VXR_MASK, PPCVEC,	{ VD, VA, VB } },
{ "vcmpgtuw",  VXR(4, 646, 0), VXR_MASK, PPCVEC,	{ VD, VA, VB } },
{ "vcmpgtuw.", VXR(4, 646, 1), VXR_MASK, PPCVEC,	{ VD, VA, VB } },
{ "vctsxs",    VX(4,  970), VX_MASK,	PPCVEC,		{ VD, VB, UIMM } },
{ "vctuxs",    VX(4,  906), VX_MASK,	PPCVEC,		{ VD, VB, UIMM } },
{ "vexptefp",  VX(4,  394), VX_MASK,	PPCVEC,		{ VD, VB } },
{ "vlogefp",   VX(4,  458), VX_MASK,	PPCVEC,		{ VD, VB } },
{ "vmaddfp",   VXA(4,  46), VXA_MASK,	PPCVEC,		{ VD, VA, VC, VB } },
{ "vmaxfp",    VX(4, 1034), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vmaxsb",    VX(4,  258), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vmaxsh",    VX(4,  322), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vmaxsw",    VX(4,  386), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vmaxub",    VX(4,    2), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vmaxuh",    VX(4,   66), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vmaxuw",    VX(4,  130), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vmhaddshs", VXA(4,  32), VXA_MASK,	PPCVEC,		{ VD, VA, VB, VC } },
{ "vmhraddshs", VXA(4, 33), VXA_MASK,	PPCVEC,		{ VD, VA, VB, VC } },
{ "vminfp",    VX(4, 1098), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vminsb",    VX(4,  770), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vminsh",    VX(4,  834), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vminsw",    VX(4,  898), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vminub",    VX(4,  514), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vminuh",    VX(4,  578), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vminuw",    VX(4,  642), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vmladduhm", VXA(4,  34), VXA_MASK,	PPCVEC,		{ VD, VA, VB, VC } },
{ "vmrghb",    VX(4,   12), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vmrghh",    VX(4,   76), VX_MASK,    PPCVEC,		{ VD, VA, VB } },
{ "vmrghw",    VX(4,  140), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vmrglb",    VX(4,  268), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vmrglh",    VX(4,  332), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vmrglw",    VX(4,  396), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vmsummbm",  VXA(4,  37), VXA_MASK,	PPCVEC,		{ VD, VA, VB, VC } },
{ "vmsumshm",  VXA(4,  40), VXA_MASK,	PPCVEC,		{ VD, VA, VB, VC } },
{ "vmsumshs",  VXA(4,  41), VXA_MASK,	PPCVEC,		{ VD, VA, VB, VC } },
{ "vmsumubm",  VXA(4,  36), VXA_MASK,   PPCVEC,		{ VD, VA, VB, VC } },
{ "vmsumuhm",  VXA(4,  38), VXA_MASK,   PPCVEC,		{ VD, VA, VB, VC } },
{ "vmsumuhs",  VXA(4,  39), VXA_MASK,   PPCVEC,		{ VD, VA, VB, VC } },
{ "vmulesb",   VX(4,  776), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vmulesh",   VX(4,  840), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vmuleub",   VX(4,  520), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vmuleuh",   VX(4,  584), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vmulosb",   VX(4,  264), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vmulosh",   VX(4,  328), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vmuloub",   VX(4,    8), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vmulouh",   VX(4,   72), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vnmsubfp",  VXA(4,  47), VXA_MASK,	PPCVEC,		{ VD, VA, VC, VB } },
{ "vnor",      VX(4, 1284), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vor",       VX(4, 1156), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vperm",     VXA(4,  43), VXA_MASK,	PPCVEC,		{ VD, VA, VB, VC } },
{ "vpkpx",     VX(4,  782), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vpkshss",   VX(4,  398), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vpkshus",   VX(4,  270), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vpkswss",   VX(4,  462), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vpkswus",   VX(4,  334), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vpkuhum",   VX(4,   14), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vpkuhus",   VX(4,  142), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vpkuwum",   VX(4,   78), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vpkuwus",   VX(4,  206), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vrefp",     VX(4,  266), VX_MASK,	PPCVEC,		{ VD, VB } },
{ "vrfim",     VX(4,  714), VX_MASK,	PPCVEC,		{ VD, VB } },
{ "vrfin",     VX(4,  522), VX_MASK,	PPCVEC,		{ VD, VB } },
{ "vrfip",     VX(4,  650), VX_MASK,	PPCVEC,		{ VD, VB } },
{ "vrfiz",     VX(4,  586), VX_MASK,	PPCVEC,		{ VD, VB } },
{ "vrlb",      VX(4,    4), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vrlh",      VX(4,   68), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vrlw",      VX(4,  132), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vrsqrtefp", VX(4,  330), VX_MASK,	PPCVEC,		{ VD, VB } },
{ "vsel",      VXA(4,  42), VXA_MASK,	PPCVEC,		{ VD, VA, VB, VC } },
{ "vsl",       VX(4,  452), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vslb",      VX(4,  260), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vsldoi",    VXA(4,  44), VXA_MASK,	PPCVEC,		{ VD, VA, VB, SHB } },
{ "vslh",      VX(4,  324), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vslo",      VX(4, 1036), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vslw",      VX(4,  388), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vspltb",    VX(4,  524), VX_MASK,	PPCVEC,		{ VD, VB, UIMM } },
{ "vsplth",    VX(4,  588), VX_MASK,	PPCVEC,		{ VD, VB, UIMM } },
{ "vspltisb",  VX(4,  780), VX_MASK,	PPCVEC,		{ VD, SIMM } },
{ "vspltish",  VX(4,  844), VX_MASK,	PPCVEC,		{ VD, SIMM } },
{ "vspltisw",  VX(4,  908), VX_MASK,	PPCVEC,		{ VD, SIMM } },
{ "vspltw",    VX(4,  652), VX_MASK,	PPCVEC,		{ VD, VB, UIMM } },
{ "vsr",       VX(4,  708), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vsrab",     VX(4,  772), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vsrah",     VX(4,  836), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vsraw",     VX(4,  900), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vsrb",      VX(4,  516), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vsrh",      VX(4,  580), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vsro",      VX(4, 1100), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vsrw",      VX(4,  644), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vsubcuw",   VX(4, 1408), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vsubfp",    VX(4,   74), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vsubsbs",   VX(4, 1792), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vsubshs",   VX(4, 1856), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vsubsws",   VX(4, 1920), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vsububm",   VX(4, 1024), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vsububs",   VX(4, 1536), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vsubuhm",   VX(4, 1088), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vsubuhs",   VX(4, 1600), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vsubuwm",   VX(4, 1152), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vsubuws",   VX(4, 1664), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vsumsws",   VX(4, 1928), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vsum2sws",  VX(4, 1672), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vsum4sbs",  VX(4, 1800), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vsum4shs",  VX(4, 1608), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vsum4ubs",  VX(4, 1544), VX_MASK,	PPCVEC,		{ VD, VA, VB } },
{ "vupkhpx",   VX(4,  846), VX_MASK,	PPCVEC,		{ VD, VB } },
{ "vupkhsb",   VX(4,  526), VX_MASK,	PPCVEC,		{ VD, VB } },
{ "vupkhsh",   VX(4,  590), VX_MASK,	PPCVEC,		{ VD, VB } },
{ "vupklpx",   VX(4,  974), VX_MASK,	PPCVEC,		{ VD, VB } },
{ "vupklsb",   VX(4,  654), VX_MASK,	PPCVEC,		{ VD, VB } },
{ "vupklsh",   VX(4,  718), VX_MASK,	PPCVEC,		{ VD, VB } },
{ "vxor",      VX(4, 1220), VX_MASK,	PPCVEC,		{ VD, VA, VB } },

{ "evaddw",    VX(4, 512), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evaddiw",   VX(4, 514), VX_MASK,	PPCSPE,		{ RS, RB, UIMM } },
{ "evsubfw",   VX(4, 516), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evsubw",    VX(4, 516), VX_MASK,	PPCSPE,		{ RS, RB, RA } },
{ "evsubifw",  VX(4, 518), VX_MASK,	PPCSPE,		{ RS, UIMM, RB } },
{ "evsubiw",   VX(4, 518), VX_MASK,	PPCSPE,		{ RS, RB, UIMM } },
{ "evabs",     VX(4, 520), VX_MASK,	PPCSPE,		{ RS, RA } },
{ "evneg",     VX(4, 521), VX_MASK,	PPCSPE,		{ RS, RA } },
{ "evextsb",   VX(4, 522), VX_MASK,	PPCSPE,		{ RS, RA } },
{ "evextsh",   VX(4, 523), VX_MASK,	PPCSPE,		{ RS, RA } },
{ "evrndw",    VX(4, 524), VX_MASK,	PPCSPE,		{ RS, RA } },
{ "evcntlzw",  VX(4, 525), VX_MASK,	PPCSPE,		{ RS, RA } },
{ "evcntlsw",  VX(4, 526), VX_MASK,	PPCSPE,		{ RS, RA } },

{ "brinc",     VX(4, 527), VX_MASK,	PPCSPE,		{ RS, RA, RB } },

{ "evand",     VX(4, 529), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evandc",    VX(4, 530), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmr",      VX(4, 535), VX_MASK,	PPCSPE,		{ RS, RA, BBA } },
{ "evor",      VX(4, 535), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evorc",     VX(4, 539), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evxor",     VX(4, 534), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "eveqv",     VX(4, 537), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evnand",    VX(4, 542), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evnot",     VX(4, 536), VX_MASK,	PPCSPE,		{ RS, RA, BBA } },
{ "evnor",     VX(4, 536), VX_MASK,	PPCSPE,		{ RS, RA, RB } },

{ "evrlw",     VX(4, 552), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evrlwi",    VX(4, 554), VX_MASK,	PPCSPE,		{ RS, RA, EVUIMM } },
{ "evslw",     VX(4, 548), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evslwi",    VX(4, 550), VX_MASK,	PPCSPE,		{ RS, RA, EVUIMM } },
{ "evsrws",    VX(4, 545), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evsrwu",    VX(4, 544), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evsrwis",   VX(4, 547), VX_MASK,	PPCSPE,		{ RS, RA, EVUIMM } },
{ "evsrwiu",   VX(4, 546), VX_MASK,	PPCSPE,		{ RS, RA, EVUIMM } },
{ "evsplati",  VX(4, 553), VX_MASK,	PPCSPE,		{ RS, SIMM } },
{ "evsplatfi", VX(4, 555), VX_MASK,	PPCSPE,		{ RS, SIMM } },
{ "evmergehi", VX(4, 556), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmergelo", VX(4, 557), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmergehilo",VX(4,558), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmergelohi",VX(4,559), VX_MASK,	PPCSPE,		{ RS, RA, RB } },

{ "evcmpgts",  VX(4, 561), VX_MASK,	PPCSPE,		{ CRFD, RA, RB } },
{ "evcmpgtu",  VX(4, 560), VX_MASK,	PPCSPE,		{ CRFD, RA, RB } },
{ "evcmplts",  VX(4, 563), VX_MASK,	PPCSPE,		{ CRFD, RA, RB } },
{ "evcmpltu",  VX(4, 562), VX_MASK,	PPCSPE,		{ CRFD, RA, RB } },
{ "evcmpeq",   VX(4, 564), VX_MASK,	PPCSPE,		{ CRFD, RA, RB } },
{ "evsel",     EVSEL(4,79),EVSEL_MASK,	PPCSPE,		{ RS, RA, RB, CRFS } },

{ "evldd",     VX(4, 769), VX_MASK,	PPCSPE,		{ RS, EVUIMM_8, RA } },
{ "evlddx",    VX(4, 768), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evldw",     VX(4, 771), VX_MASK,	PPCSPE,		{ RS, EVUIMM_8, RA } },
{ "evldwx",    VX(4, 770), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evldh",     VX(4, 773), VX_MASK,	PPCSPE,		{ RS, EVUIMM_8, RA } },
{ "evldhx",    VX(4, 772), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evlwhe",    VX(4, 785), VX_MASK,	PPCSPE,		{ RS, EVUIMM_4, RA } },
{ "evlwhex",   VX(4, 784), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evlwhou",   VX(4, 789), VX_MASK,	PPCSPE,		{ RS, EVUIMM_4, RA } },
{ "evlwhoux",  VX(4, 788), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evlwhos",   VX(4, 791), VX_MASK,	PPCSPE,		{ RS, EVUIMM_4, RA } },
{ "evlwhosx",  VX(4, 790), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evlwwsplat",VX(4, 793), VX_MASK,	PPCSPE,		{ RS, EVUIMM_4, RA } },
{ "evlwwsplatx",VX(4, 792), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evlwhsplat",VX(4, 797), VX_MASK,	PPCSPE,		{ RS, EVUIMM_4, RA } },
{ "evlwhsplatx",VX(4, 796), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evlhhesplat",VX(4, 777), VX_MASK,	PPCSPE,		{ RS, EVUIMM_2, RA } },
{ "evlhhesplatx",VX(4, 776), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evlhhousplat",VX(4, 781), VX_MASK,	PPCSPE,		{ RS, EVUIMM_2, RA } },
{ "evlhhousplatx",VX(4, 780), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evlhhossplat",VX(4, 783), VX_MASK,	PPCSPE,		{ RS, EVUIMM_2, RA } },
{ "evlhhossplatx",VX(4, 782), VX_MASK,	PPCSPE,		{ RS, RA, RB } },

{ "evstdd",    VX(4, 801), VX_MASK,	PPCSPE,		{ RS, EVUIMM_8, RA } },
{ "evstddx",   VX(4, 800), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evstdw",    VX(4, 803), VX_MASK,	PPCSPE,		{ RS, EVUIMM_8, RA } },
{ "evstdwx",   VX(4, 802), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evstdh",    VX(4, 805), VX_MASK,	PPCSPE,		{ RS, EVUIMM_8, RA } },
{ "evstdhx",   VX(4, 804), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evstwwe",   VX(4, 825), VX_MASK,	PPCSPE,		{ RS, EVUIMM_4, RA } },
{ "evstwwex",  VX(4, 824), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evstwwo",   VX(4, 829), VX_MASK,	PPCSPE,		{ RS, EVUIMM_4, RA } },
{ "evstwwox",  VX(4, 828), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evstwhe",   VX(4, 817), VX_MASK,	PPCSPE,		{ RS, EVUIMM_4, RA } },
{ "evstwhex",  VX(4, 816), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evstwho",   VX(4, 821), VX_MASK,	PPCSPE,		{ RS, EVUIMM_4, RA } },
{ "evstwhox",  VX(4, 820), VX_MASK,	PPCSPE,		{ RS, RA, RB } },

{ "evfsabs",   VX(4, 644), VX_MASK,	PPCSPE,		{ RS, RA } },
{ "evfsnabs",  VX(4, 645), VX_MASK,	PPCSPE,		{ RS, RA } },
{ "evfsneg",   VX(4, 646), VX_MASK,	PPCSPE,		{ RS, RA } },
{ "evfsadd",   VX(4, 640), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evfssub",   VX(4, 641), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evfsmul",   VX(4, 648), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evfsdiv",   VX(4, 649), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evfscmpgt", VX(4, 652), VX_MASK,	PPCSPE,		{ CRFD, RA, RB } },
{ "evfscmplt", VX(4, 653), VX_MASK,	PPCSPE,		{ CRFD, RA, RB } },
{ "evfscmpeq", VX(4, 654), VX_MASK,	PPCSPE,		{ CRFD, RA, RB } },
{ "evfststgt", VX(4, 668), VX_MASK,	PPCSPE,		{ CRFD, RA, RB } },
{ "evfststlt", VX(4, 669), VX_MASK,	PPCSPE,		{ CRFD, RA, RB } },
{ "evfststeq", VX(4, 670), VX_MASK,	PPCSPE,		{ CRFD, RA, RB } },
{ "evfscfui",  VX(4, 656), VX_MASK,	PPCSPE,		{ RS, RB } },
{ "evfsctuiz", VX(4, 664), VX_MASK,	PPCSPE,		{ RS, RB } },
{ "evfscfsi",  VX(4, 657), VX_MASK,	PPCSPE,		{ RS, RB } },
{ "evfscfuf",  VX(4, 658), VX_MASK,	PPCSPE,		{ RS, RB } },
{ "evfscfsf",  VX(4, 659), VX_MASK,	PPCSPE,		{ RS, RB } },
{ "evfsctui",  VX(4, 660), VX_MASK,	PPCSPE,		{ RS, RB } },
{ "evfsctsi",  VX(4, 661), VX_MASK,	PPCSPE,		{ RS, RB } },
{ "evfsctsiz", VX(4, 666), VX_MASK,	PPCSPE,		{ RS, RB } },
{ "evfsctuf",  VX(4, 662), VX_MASK,	PPCSPE,		{ RS, RB } },
{ "evfsctsf",  VX(4, 663), VX_MASK,	PPCSPE,		{ RS, RB } },

{ "efsabs",   VX(4, 708), VX_MASK,	PPCEFS,		{ RS, RA } },
{ "efsnabs",  VX(4, 709), VX_MASK,	PPCEFS,		{ RS, RA } },
{ "efsneg",   VX(4, 710), VX_MASK,	PPCEFS,		{ RS, RA } },
{ "efsadd",   VX(4, 704), VX_MASK,	PPCEFS,		{ RS, RA, RB } },
{ "efssub",   VX(4, 705), VX_MASK,	PPCEFS,		{ RS, RA, RB } },
{ "efsmul",   VX(4, 712), VX_MASK,	PPCEFS,		{ RS, RA, RB } },
{ "efsdiv",   VX(4, 713), VX_MASK,	PPCEFS,		{ RS, RA, RB } },
{ "efscmpgt", VX(4, 716), VX_MASK,	PPCEFS,		{ CRFD, RA, RB } },
{ "efscmplt", VX(4, 717), VX_MASK,	PPCEFS,		{ CRFD, RA, RB } },
{ "efscmpeq", VX(4, 718), VX_MASK,	PPCEFS,		{ CRFD, RA, RB } },
{ "efststgt", VX(4, 732), VX_MASK,	PPCEFS,		{ CRFD, RA, RB } },
{ "efststlt", VX(4, 733), VX_MASK,	PPCEFS,		{ CRFD, RA, RB } },
{ "efststeq", VX(4, 734), VX_MASK,	PPCEFS,		{ CRFD, RA, RB } },
{ "efscfui",  VX(4, 720), VX_MASK,	PPCEFS,		{ RS, RB } },
{ "efsctuiz", VX(4, 728), VX_MASK,	PPCEFS,		{ RS, RB } },
{ "efscfsi",  VX(4, 721), VX_MASK,	PPCEFS,		{ RS, RB } },
{ "efscfuf",  VX(4, 722), VX_MASK,	PPCEFS,		{ RS, RB } },
{ "efscfsf",  VX(4, 723), VX_MASK,	PPCEFS,		{ RS, RB } },
{ "efsctui",  VX(4, 724), VX_MASK,	PPCEFS,		{ RS, RB } },
{ "efsctsi",  VX(4, 725), VX_MASK,	PPCEFS,		{ RS, RB } },
{ "efsctsiz", VX(4, 730), VX_MASK,	PPCEFS,		{ RS, RB } },
{ "efsctuf",  VX(4, 726), VX_MASK,	PPCEFS,		{ RS, RB } },
{ "efsctsf",  VX(4, 727), VX_MASK,	PPCEFS,		{ RS, RB } },

{ "evmhossf",  VX(4, 1031), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmhossfa", VX(4, 1063), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmhosmf",  VX(4, 1039), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmhosmfa", VX(4, 1071), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmhosmi",  VX(4, 1037), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmhosmia", VX(4, 1069), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmhoumi",  VX(4, 1036), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmhoumia", VX(4, 1068), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmhessf",  VX(4, 1027), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmhessfa", VX(4, 1059), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmhesmf",  VX(4, 1035), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmhesmfa", VX(4, 1067), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmhesmi",  VX(4, 1033), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmhesmia", VX(4, 1065), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmheumi",  VX(4, 1032), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmheumia", VX(4, 1064), VX_MASK,	PPCSPE,		{ RS, RA, RB } },

{ "evmhossfaaw",VX(4, 1287), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmhossiaaw",VX(4, 1285), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmhosmfaaw",VX(4, 1295), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmhosmiaaw",VX(4, 1293), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmhousiaaw",VX(4, 1284), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmhoumiaaw",VX(4, 1292), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmhessfaaw",VX(4, 1283), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmhessiaaw",VX(4, 1281), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmhesmfaaw",VX(4, 1291), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmhesmiaaw",VX(4, 1289), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmheusiaaw",VX(4, 1280), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmheumiaaw",VX(4, 1288), VX_MASK,	PPCSPE,		{ RS, RA, RB } },

{ "evmhossfanw",VX(4, 1415), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmhossianw",VX(4, 1413), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmhosmfanw",VX(4, 1423), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmhosmianw",VX(4, 1421), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmhousianw",VX(4, 1412), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmhoumianw",VX(4, 1420), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmhessfanw",VX(4, 1411), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmhessianw",VX(4, 1409), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmhesmfanw",VX(4, 1419), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmhesmianw",VX(4, 1417), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmheusianw",VX(4, 1408), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmheumianw",VX(4, 1416), VX_MASK,	PPCSPE,		{ RS, RA, RB } },

{ "evmhogsmfaa",VX(4, 1327), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmhogsmiaa",VX(4, 1325), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmhogumiaa",VX(4, 1324), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmhegsmfaa",VX(4, 1323), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmhegsmiaa",VX(4, 1321), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmhegumiaa",VX(4, 1320), VX_MASK,	PPCSPE,		{ RS, RA, RB } },

{ "evmhogsmfan",VX(4, 1455), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmhogsmian",VX(4, 1453), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmhogumian",VX(4, 1452), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmhegsmfan",VX(4, 1451), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmhegsmian",VX(4, 1449), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmhegumian",VX(4, 1448), VX_MASK,	PPCSPE,		{ RS, RA, RB } },

{ "evmwhssf",  VX(4, 1095), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmwhssfa", VX(4, 1127), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmwhsmf",  VX(4, 1103), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmwhsmfa", VX(4, 1135), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmwhsmi",  VX(4, 1101), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmwhsmia", VX(4, 1133), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmwhumi",  VX(4, 1100), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmwhumia", VX(4, 1132), VX_MASK,	PPCSPE,		{ RS, RA, RB } },

{ "evmwlumi",  VX(4, 1096), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmwlumia", VX(4, 1128), VX_MASK,	PPCSPE,		{ RS, RA, RB } },

{ "evmwlssiaaw",VX(4, 1345), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmwlsmiaaw",VX(4, 1353), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmwlusiaaw",VX(4, 1344), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmwlumiaaw",VX(4, 1352), VX_MASK,	PPCSPE,		{ RS, RA, RB } },

{ "evmwlssianw",VX(4, 1473), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmwlsmianw",VX(4, 1481), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmwlusianw",VX(4, 1472), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmwlumianw",VX(4, 1480), VX_MASK,	PPCSPE,		{ RS, RA, RB } },

{ "evmwssf",   VX(4, 1107), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmwssfa",  VX(4, 1139), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmwsmf",   VX(4, 1115), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmwsmfa",  VX(4, 1147), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmwsmi",   VX(4, 1113), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmwsmia",  VX(4, 1145), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmwumi",   VX(4, 1112), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmwumia",  VX(4, 1144), VX_MASK,	PPCSPE,		{ RS, RA, RB } },

{ "evmwssfaa", VX(4, 1363), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmwsmfaa", VX(4, 1371), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmwsmiaa", VX(4, 1369), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmwumiaa", VX(4, 1368), VX_MASK,	PPCSPE,		{ RS, RA, RB } },

{ "evmwssfan", VX(4, 1491), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmwsmfan", VX(4, 1499), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmwsmian", VX(4, 1497), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evmwumian", VX(4, 1496), VX_MASK,	PPCSPE,		{ RS, RA, RB } },

{ "evaddssiaaw",VX(4, 1217), VX_MASK,	PPCSPE,		{ RS, RA } },
{ "evaddsmiaaw",VX(4, 1225), VX_MASK,	PPCSPE,		{ RS, RA } },
{ "evaddusiaaw",VX(4, 1216), VX_MASK,	PPCSPE,		{ RS, RA } },
{ "evaddumiaaw",VX(4, 1224), VX_MASK,	PPCSPE,		{ RS, RA } },

{ "evsubfssiaaw",VX(4, 1219), VX_MASK,	PPCSPE,		{ RS, RA } },
{ "evsubfsmiaaw",VX(4, 1227), VX_MASK,	PPCSPE,		{ RS, RA } },
{ "evsubfusiaaw",VX(4, 1218), VX_MASK,	PPCSPE,		{ RS, RA } },
{ "evsubfumiaaw",VX(4, 1226), VX_MASK,	PPCSPE,		{ RS, RA } },

{ "evmra",    VX(4, 1220), VX_MASK,	PPCSPE,		{ RS, RA } },

{ "evdivws",  VX(4, 1222), VX_MASK,	PPCSPE,		{ RS, RA, RB } },
{ "evdivwu",  VX(4, 1223), VX_MASK,	PPCSPE,		{ RS, RA, RB } },

{ "mulli",   OP(7),	OP_MASK,	PPCCOM,		{ RT, RA, SI } },
{ "muli",    OP(7),	OP_MASK,	PWRCOM,		{ RT, RA, SI } },

{ "subfic",  OP(8),	OP_MASK,	PPCCOM,		{ RT, RA, SI } },
{ "sfi",     OP(8),	OP_MASK,	PWRCOM,		{ RT, RA, SI } },

{ "dozi",    OP(9),	OP_MASK,	M601,		{ RT, RA, SI } },

{ "bce",     B(9,0,0),	B_MASK,		BOOKE64,	{ BO, BI, BD } },
{ "bcel",    B(9,0,1),	B_MASK,		BOOKE64,	{ BO, BI, BD } },
{ "bcea",    B(9,1,0),	B_MASK,		BOOKE64,	{ BO, BI, BDA } },
{ "bcela",   B(9,1,1),	B_MASK,		BOOKE64,	{ BO, BI, BDA } },

{ "cmplwi",  OPL(10,0),	OPL_MASK,	PPCCOM,		{ OBF, RA, UI } },
{ "cmpldi",  OPL(10,1), OPL_MASK,	PPC64,		{ OBF, RA, UI } },
{ "cmpli",   OP(10),	OP_MASK,	PPCONLY,	{ BF, L, RA, UI } },
{ "cmpli",   OP(10),	OP_MASK,	PWRCOM,		{ BF, RA, UI } },

{ "cmpwi",   OPL(11,0),	OPL_MASK,	PPCCOM,		{ OBF, RA, SI } },
{ "cmpdi",   OPL(11,1),	OPL_MASK,	PPC64,		{ OBF, RA, SI } },
{ "cmpi",    OP(11),	OP_MASK,	PPCONLY,	{ BF, L, RA, SI } },
{ "cmpi",    OP(11),	OP_MASK,	PWRCOM,		{ BF, RA, SI } },

{ "addic",   OP(12),	OP_MASK,	PPCCOM,		{ RT, RA, SI } },
{ "ai",	     OP(12),	OP_MASK,	PWRCOM,		{ RT, RA, SI } },
{ "subic",   OP(12),	OP_MASK,	PPCCOM,		{ RT, RA, NSI } },

{ "addic.",  OP(13),	OP_MASK,	PPCCOM,		{ RT, RA, SI } },
{ "ai.",     OP(13),	OP_MASK,	PWRCOM,		{ RT, RA, SI } },
{ "subic.",  OP(13),	OP_MASK,	PPCCOM,		{ RT, RA, NSI } },

{ "li",	     OP(14),	DRA_MASK,	PPCCOM,		{ RT, SI } },
{ "lil",     OP(14),	DRA_MASK,	PWRCOM,		{ RT, SI } },
{ "addi",    OP(14),	OP_MASK,	PPCCOM,		{ RT, RA, SI } },
{ "cal",     OP(14),	OP_MASK,	PWRCOM,		{ RT, D, RA } },
{ "subi",    OP(14),	OP_MASK,	PPCCOM,		{ RT, RA, NSI } },
{ "la",	     OP(14),	OP_MASK,	PPCCOM,		{ RT, D, RA } },

{ "lis",     OP(15),	DRA_MASK,	PPCCOM,		{ RT, SISIGNOPT } },
{ "liu",     OP(15),	DRA_MASK,	PWRCOM,		{ RT, SISIGNOPT } },
{ "addis",   OP(15),	OP_MASK,	PPCCOM,		{ RT,RA,SISIGNOPT } },
{ "cau",     OP(15),	OP_MASK,	PWRCOM,		{ RT,RA,SISIGNOPT } },
{ "subis",   OP(15),	OP_MASK,	PPCCOM,		{ RT, RA, NSI } },

{ "bdnz-",   BBO(16,BODNZ,0,0),      BBOATBI_MASK, PPCCOM,	{ BDM } },
{ "bdnz+",   BBO(16,BODNZ,0,0),      BBOATBI_MASK, PPCCOM,	{ BDP } },
{ "bdnz",    BBO(16,BODNZ,0,0),      BBOATBI_MASK, PPCCOM,	{ BD } },
{ "bdn",     BBO(16,BODNZ,0,0),      BBOATBI_MASK, PWRCOM,	{ BD } },
{ "bdnzl-",  BBO(16,BODNZ,0,1),      BBOATBI_MASK, PPCCOM,	{ BDM } },
{ "bdnzl+",  BBO(16,BODNZ,0,1),      BBOATBI_MASK, PPCCOM,	{ BDP } },
{ "bdnzl",   BBO(16,BODNZ,0,1),      BBOATBI_MASK, PPCCOM,	{ BD } },
{ "bdnl",    BBO(16,BODNZ,0,1),      BBOATBI_MASK, PWRCOM,	{ BD } },
{ "bdnza-",  BBO(16,BODNZ,1,0),      BBOATBI_MASK, PPCCOM,	{ BDMA } },
{ "bdnza+",  BBO(16,BODNZ,1,0),      BBOATBI_MASK, PPCCOM,	{ BDPA } },
{ "bdnza",   BBO(16,BODNZ,1,0),      BBOATBI_MASK, PPCCOM,	{ BDA } },
{ "bdna",    BBO(16,BODNZ,1,0),      BBOATBI_MASK, PWRCOM,	{ BDA } },
{ "bdnzla-", BBO(16,BODNZ,1,1),      BBOATBI_MASK, PPCCOM,	{ BDMA } },
{ "bdnzla+", BBO(16,BODNZ,1,1),      BBOATBI_MASK, PPCCOM,	{ BDPA } },
{ "bdnzla",  BBO(16,BODNZ,1,1),      BBOATBI_MASK, PPCCOM,	{ BDA } },
{ "bdnla",   BBO(16,BODNZ,1,1),      BBOATBI_MASK, PWRCOM,	{ BDA } },
{ "bdz-",    BBO(16,BODZ,0,0),       BBOATBI_MASK, PPCCOM,	{ BDM } },
{ "bdz+",    BBO(16,BODZ,0,0),       BBOATBI_MASK, PPCCOM,	{ BDP } },
{ "bdz",     BBO(16,BODZ,0,0),       BBOATBI_MASK, COM,		{ BD } },
{ "bdzl-",   BBO(16,BODZ,0,1),       BBOATBI_MASK, PPCCOM,	{ BDM } },
{ "bdzl+",   BBO(16,BODZ,0,1),       BBOATBI_MASK, PPCCOM,	{ BDP } },
{ "bdzl",    BBO(16,BODZ,0,1),       BBOATBI_MASK, COM,		{ BD } },
{ "bdza-",   BBO(16,BODZ,1,0),       BBOATBI_MASK, PPCCOM,	{ BDMA } },
{ "bdza+",   BBO(16,BODZ,1,0),       BBOATBI_MASK, PPCCOM,	{ BDPA } },
{ "bdza",    BBO(16,BODZ,1,0),       BBOATBI_MASK, COM,		{ BDA } },
{ "bdzla-",  BBO(16,BODZ,1,1),       BBOATBI_MASK, PPCCOM,	{ BDMA } },
{ "bdzla+",  BBO(16,BODZ,1,1),       BBOATBI_MASK, PPCCOM,	{ BDPA } },
{ "bdzla",   BBO(16,BODZ,1,1),       BBOATBI_MASK, COM,		{ BDA } },
{ "blt-",    BBOCB(16,BOT,CBLT,0,0), BBOATCB_MASK, PPCCOM,	{ CR, BDM } },
{ "blt+",    BBOCB(16,BOT,CBLT,0,0), BBOATCB_MASK, PPCCOM,	{ CR, BDP } },
{ "blt",     BBOCB(16,BOT,CBLT,0,0), BBOATCB_MASK, COM,		{ CR, BD } },
{ "bltl-",   BBOCB(16,BOT,CBLT,0,1), BBOATCB_MASK, PPCCOM,	{ CR, BDM } },
{ "bltl+",   BBOCB(16,BOT,CBLT,0,1), BBOATCB_MASK, PPCCOM,	{ CR, BDP } },
{ "bltl",    BBOCB(16,BOT,CBLT,0,1), BBOATCB_MASK, COM,		{ CR, BD } },
{ "blta-",   BBOCB(16,BOT,CBLT,1,0), BBOATCB_MASK, PPCCOM,	{ CR, BDMA } },
{ "blta+",   BBOCB(16,BOT,CBLT,1,0), BBOATCB_MASK, PPCCOM,	{ CR, BDPA } },
{ "blta",    BBOCB(16,BOT,CBLT,1,0), BBOATCB_MASK, COM,		{ CR, BDA } },
{ "bltla-",  BBOCB(16,BOT,CBLT,1,1), BBOATCB_MASK, PPCCOM,	{ CR, BDMA } },
{ "bltla+",  BBOCB(16,BOT,CBLT,1,1), BBOATCB_MASK, PPCCOM,	{ CR, BDPA } },
{ "bltla",   BBOCB(16,BOT,CBLT,1,1), BBOATCB_MASK, COM,		{ CR, BDA } },
{ "bgt-",    BBOCB(16,BOT,CBGT,0,0), BBOATCB_MASK, PPCCOM,	{ CR, BDM } },
{ "bgt+",    BBOCB(16,BOT,CBGT,0,0), BBOATCB_MASK, PPCCOM,	{ CR, BDP } },
{ "bgt",     BBOCB(16,BOT,CBGT,0,0), BBOATCB_MASK, COM,		{ CR, BD } },
{ "bgtl-",   BBOCB(16,BOT,CBGT,0,1), BBOATCB_MASK, PPCCOM,	{ CR, BDM } },
{ "bgtl+",   BBOCB(16,BOT,CBGT,0,1), BBOATCB_MASK, PPCCOM,	{ CR, BDP } },
{ "bgtl",    BBOCB(16,BOT,CBGT,0,1), BBOATCB_MASK, COM,		{ CR, BD } },
{ "bgta-",   BBOCB(16,BOT,CBGT,1,0), BBOATCB_MASK, PPCCOM,	{ CR, BDMA } },
{ "bgta+",   BBOCB(16,BOT,CBGT,1,0), BBOATCB_MASK, PPCCOM,	{ CR, BDPA } },
{ "bgta",    BBOCB(16,BOT,CBGT,1,0), BBOATCB_MASK, COM,		{ CR, BDA } },
{ "bgtla-",  BBOCB(16,BOT,CBGT,1,1), BBOATCB_MASK, PPCCOM,	{ CR, BDMA } },
{ "bgtla+",  BBOCB(16,BOT,CBGT,1,1), BBOATCB_MASK, PPCCOM,	{ CR, BDPA } },
{ "bgtla",   BBOCB(16,BOT,CBGT,1,1), BBOATCB_MASK, COM,		{ CR, BDA } },
{ "beq-",    BBOCB(16,BOT,CBEQ,0,0), BBOATCB_MASK, PPCCOM,	{ CR, BDM } },
{ "beq+",    BBOCB(16,BOT,CBEQ,0,0), BBOATCB_MASK, PPCCOM,	{ CR, BDP } },
{ "beq",     BBOCB(16,BOT,CBEQ,0,0), BBOATCB_MASK, COM,		{ CR, BD } },
{ "beql-",   BBOCB(16,BOT,CBEQ,0,1), BBOATCB_MASK, PPCCOM,	{ CR, BDM } },
{ "beql+",   BBOCB(16,BOT,CBEQ,0,1), BBOATCB_MASK, PPCCOM,	{ CR, BDP } },
{ "beql",    BBOCB(16,BOT,CBEQ,0,1), BBOATCB_MASK, COM,		{ CR, BD } },
{ "beqa-",   BBOCB(16,BOT,CBEQ,1,0), BBOATCB_MASK, PPCCOM,	{ CR, BDMA } },
{ "beqa+",   BBOCB(16,BOT,CBEQ,1,0), BBOATCB_MASK, PPCCOM,	{ CR, BDPA } },
{ "beqa",    BBOCB(16,BOT,CBEQ,1,0), BBOATCB_MASK, COM,		{ CR, BDA } },
{ "beqla-",  BBOCB(16,BOT,CBEQ,1,1), BBOATCB_MASK, PPCCOM,	{ CR, BDMA } },
{ "beqla+",  BBOCB(16,BOT,CBEQ,1,1), BBOATCB_MASK, PPCCOM,	{ CR, BDPA } },
{ "beqla",   BBOCB(16,BOT,CBEQ,1,1), BBOATCB_MASK, COM,		{ CR, BDA } },
{ "bso-",    BBOCB(16,BOT,CBSO,0,0), BBOATCB_MASK, PPCCOM,	{ CR, BDM } },
{ "bso+",    BBOCB(16,BOT,CBSO,0,0), BBOATCB_MASK, PPCCOM,	{ CR, BDP } },
{ "bso",     BBOCB(16,BOT,CBSO,0,0), BBOATCB_MASK, COM,		{ CR, BD } },
{ "bsol-",   BBOCB(16,BOT,CBSO,0,1), BBOATCB_MASK, PPCCOM,	{ CR, BDM } },
{ "bsol+",   BBOCB(16,BOT,CBSO,0,1), BBOATCB_MASK, PPCCOM,	{ CR, BDP } },
{ "bsol",    BBOCB(16,BOT,CBSO,0,1), BBOATCB_MASK, COM,		{ CR, BD } },
{ "bsoa-",   BBOCB(16,BOT,CBSO,1,0), BBOATCB_MASK, PPCCOM,	{ CR, BDMA } },
{ "bsoa+",   BBOCB(16,BOT,CBSO,1,0), BBOATCB_MASK, PPCCOM,	{ CR, BDPA } },
{ "bsoa",    BBOCB(16,BOT,CBSO,1,0), BBOATCB_MASK, COM,		{ CR, BDA } },
{ "bsola-",  BBOCB(16,BOT,CBSO,1,1), BBOATCB_MASK, PPCCOM,	{ CR, BDMA } },
{ "bsola+",  BBOCB(16,BOT,CBSO,1,1), BBOATCB_MASK, PPCCOM,	{ CR, BDPA } },
{ "bsola",   BBOCB(16,BOT,CBSO,1,1), BBOATCB_MASK, COM,		{ CR, BDA } },
{ "bun-",    BBOCB(16,BOT,CBSO,0,0), BBOATCB_MASK, PPCCOM,	{ CR, BDM } },
{ "bun+",    BBOCB(16,BOT,CBSO,0,0), BBOATCB_MASK, PPCCOM,	{ CR, BDP } },
{ "bun",     BBOCB(16,BOT,CBSO,0,0), BBOATCB_MASK, PPCCOM,	{ CR, BD } },
{ "bunl-",   BBOCB(16,BOT,CBSO,0,1), BBOATCB_MASK, PPCCOM,	{ CR, BDM } },
{ "bunl+",   BBOCB(16,BOT,CBSO,0,1), BBOATCB_MASK, PPCCOM,	{ CR, BDP } },
{ "bunl",    BBOCB(16,BOT,CBSO,0,1), BBOATCB_MASK, PPCCOM,	{ CR, BD } },
{ "buna-",   BBOCB(16,BOT,CBSO,1,0), BBOATCB_MASK, PPCCOM,	{ CR, BDMA } },
{ "buna+",   BBOCB(16,BOT,CBSO,1,0), BBOATCB_MASK, PPCCOM,	{ CR, BDPA } },
{ "buna",    BBOCB(16,BOT,CBSO,1,0), BBOATCB_MASK, PPCCOM,	{ CR, BDA } },
{ "bunla-",  BBOCB(16,BOT,CBSO,1,1), BBOATCB_MASK, PPCCOM,	{ CR, BDMA } },
{ "bunla+",  BBOCB(16,BOT,CBSO,1,1), BBOATCB_MASK, PPCCOM,	{ CR, BDPA } },
{ "bunla",   BBOCB(16,BOT,CBSO,1,1), BBOATCB_MASK, PPCCOM,	{ CR, BDA } },
{ "bge-",    BBOCB(16,BOF,CBLT,0,0), BBOATCB_MASK, PPCCOM,	{ CR, BDM } },
{ "bge+",    BBOCB(16,BOF,CBLT,0,0), BBOATCB_MASK, PPCCOM,	{ CR, BDP } },
{ "bge",     BBOCB(16,BOF,CBLT,0,0), BBOATCB_MASK, COM,		{ CR, BD } },
{ "bgel-",   BBOCB(16,BOF,CBLT,0,1), BBOATCB_MASK, PPCCOM,	{ CR, BDM } },
{ "bgel+",   BBOCB(16,BOF,CBLT,0,1), BBOATCB_MASK, PPCCOM,	{ CR, BDP } },
{ "bgel",    BBOCB(16,BOF,CBLT,0,1), BBOATCB_MASK, COM,		{ CR, BD } },
{ "bgea-",   BBOCB(16,BOF,CBLT,1,0), BBOATCB_MASK, PPCCOM,	{ CR, BDMA } },
{ "bgea+",   BBOCB(16,BOF,CBLT,1,0), BBOATCB_MASK, PPCCOM,	{ CR, BDPA } },
{ "bgea",    BBOCB(16,BOF,CBLT,1,0), BBOATCB_MASK, COM,		{ CR, BDA } },
{ "bgela-",  BBOCB(16,BOF,CBLT,1,1), BBOATCB_MASK, PPCCOM,	{ CR, BDMA } },
{ "bgela+",  BBOCB(16,BOF,CBLT,1,1), BBOATCB_MASK, PPCCOM,	{ CR, BDPA } },
{ "bgela",   BBOCB(16,BOF,CBLT,1,1), BBOATCB_MASK, COM,		{ CR, BDA } },
{ "bnl-",    BBOCB(16,BOF,CBLT,0,0), BBOATCB_MASK, PPCCOM,	{ CR, BDM } },
{ "bnl+",    BBOCB(16,BOF,CBLT,0,0), BBOATCB_MASK, PPCCOM,	{ CR, BDP } },
{ "bnl",     BBOCB(16,BOF,CBLT,0,0), BBOATCB_MASK, COM,		{ CR, BD } },
{ "bnll-",   BBOCB(16,BOF,CBLT,0,1), BBOATCB_MASK, PPCCOM,	{ CR, BDM } },
{ "bnll+",   BBOCB(16,BOF,CBLT,0,1), BBOATCB_MASK, PPCCOM,	{ CR, BDP } },
{ "bnll",    BBOCB(16,BOF,CBLT,0,1), BBOATCB_MASK, COM,		{ CR, BD } },
{ "bnla-",   BBOCB(16,BOF,CBLT,1,0), BBOATCB_MASK, PPCCOM,	{ CR, BDMA } },
{ "bnla+",   BBOCB(16,BOF,CBLT,1,0), BBOATCB_MASK, PPCCOM,	{ CR, BDPA } },
{ "bnla",    BBOCB(16,BOF,CBLT,1,0), BBOATCB_MASK, COM,		{ CR, BDA } },
{ "bnlla-",  BBOCB(16,BOF,CBLT,1,1), BBOATCB_MASK, PPCCOM,	{ CR, BDMA } },
{ "bnlla+",  BBOCB(16,BOF,CBLT,1,1), BBOATCB_MASK, PPCCOM,	{ CR, BDPA } },
{ "bnlla",   BBOCB(16,BOF,CBLT,1,1), BBOATCB_MASK, COM,		{ CR, BDA } },
{ "ble-",    BBOCB(16,BOF,CBGT,0,0), BBOATCB_MASK, PPCCOM,	{ CR, BDM } },
{ "ble+",    BBOCB(16,BOF,CBGT,0,0), BBOATCB_MASK, PPCCOM,	{ CR, BDP } },
{ "ble",     BBOCB(16,BOF,CBGT,0,0), BBOATCB_MASK, COM,		{ CR, BD } },
{ "blel-",   BBOCB(16,BOF,CBGT,0,1), BBOATCB_MASK, PPCCOM,	{ CR, BDM } },
{ "blel+",   BBOCB(16,BOF,CBGT,0,1), BBOATCB_MASK, PPCCOM,	{ CR, BDP } },
{ "blel",    BBOCB(16,BOF,CBGT,0,1), BBOATCB_MASK, COM,		{ CR, BD } },
{ "blea-",   BBOCB(16,BOF,CBGT,1,0), BBOATCB_MASK, PPCCOM,	{ CR, BDMA } },
{ "blea+",   BBOCB(16,BOF,CBGT,1,0), BBOATCB_MASK, PPCCOM,	{ CR, BDPA } },
{ "blea",    BBOCB(16,BOF,CBGT,1,0), BBOATCB_MASK, COM,		{ CR, BDA } },
{ "blela-",  BBOCB(16,BOF,CBGT,1,1), BBOATCB_MASK, PPCCOM,	{ CR, BDMA } },
{ "blela+",  BBOCB(16,BOF,CBGT,1,1), BBOATCB_MASK, PPCCOM,	{ CR, BDPA } },
{ "blela",   BBOCB(16,BOF,CBGT,1,1), BBOATCB_MASK, COM,		{ CR, BDA } },
{ "bng-",    BBOCB(16,BOF,CBGT,0,0), BBOATCB_MASK, PPCCOM,	{ CR, BDM } },
{ "bng+",    BBOCB(16,BOF,CBGT,0,0), BBOATCB_MASK, PPCCOM,	{ CR, BDP } },
{ "bng",     BBOCB(16,BOF,CBGT,0,0), BBOATCB_MASK, COM,		{ CR, BD } },
{ "bngl-",   BBOCB(16,BOF,CBGT,0,1), BBOATCB_MASK, PPCCOM,	{ CR, BDM } },
{ "bngl+",   BBOCB(16,BOF,CBGT,0,1), BBOATCB_MASK, PPCCOM,	{ CR, BDP } },
{ "bngl",    BBOCB(16,BOF,CBGT,0,1), BBOATCB_MASK, COM,		{ CR, BD } },
{ "bnga-",   BBOCB(16,BOF,CBGT,1,0), BBOATCB_MASK, PPCCOM,	{ CR, BDMA } },
{ "bnga+",   BBOCB(16,BOF,CBGT,1,0), BBOATCB_MASK, PPCCOM,	{ CR, BDPA } },
{ "bnga",    BBOCB(16,BOF,CBGT,1,0), BBOATCB_MASK, COM,		{ CR, BDA } },
{ "bngla-",  BBOCB(16,BOF,CBGT,1,1), BBOATCB_MASK, PPCCOM,	{ CR, BDMA } },
{ "bngla+",  BBOCB(16,BOF,CBGT,1,1), BBOATCB_MASK, PPCCOM,	{ CR, BDPA } },
{ "bngla",   BBOCB(16,BOF,CBGT,1,1), BBOATCB_MASK, COM,		{ CR, BDA } },
{ "bne-",    BBOCB(16,BOF,CBEQ,0,0), BBOATCB_MASK, PPCCOM,	{ CR, BDM } },
{ "bne+",    BBOCB(16,BOF,CBEQ,0,0), BBOATCB_MASK, PPCCOM,	{ CR, BDP } },
{ "bne",     BBOCB(16,BOF,CBEQ,0,0), BBOATCB_MASK, COM,		{ CR, BD } },
{ "bnel-",   BBOCB(16,BOF,CBEQ,0,1), BBOATCB_MASK, PPCCOM,	{ CR, BDM } },
{ "bnel+",   BBOCB(16,BOF,CBEQ,0,1), BBOATCB_MASK, PPCCOM,	{ CR, BDP } },
{ "bnel",    BBOCB(16,BOF,CBEQ,0,1), BBOATCB_MASK, COM,		{ CR, BD } },
{ "bnea-",   BBOCB(16,BOF,CBEQ,1,0), BBOATCB_MASK, PPCCOM,	{ CR, BDMA } },
{ "bnea+",   BBOCB(16,BOF,CBEQ,1,0), BBOATCB_MASK, PPCCOM,	{ CR, BDPA } },
{ "bnea",    BBOCB(16,BOF,CBEQ,1,0), BBOATCB_MASK, COM,		{ CR, BDA } },
{ "bnela-",  BBOCB(16,BOF,CBEQ,1,1), BBOATCB_MASK, PPCCOM,	{ CR, BDMA } },
{ "bnela+",  BBOCB(16,BOF,CBEQ,1,1), BBOATCB_MASK, PPCCOM,	{ CR, BDPA } },
{ "bnela",   BBOCB(16,BOF,CBEQ,1,1), BBOATCB_MASK, COM,		{ CR, BDA } },
{ "bns-",    BBOCB(16,BOF,CBSO,0,0), BBOATCB_MASK, PPCCOM,	{ CR, BDM } },
{ "bns+",    BBOCB(16,BOF,CBSO,0,0), BBOATCB_MASK, PPCCOM,	{ CR, BDP } },
{ "bns",     BBOCB(16,BOF,CBSO,0,0), BBOATCB_MASK, COM,		{ CR, BD } },
{ "bnsl-",   BBOCB(16,BOF,CBSO,0,1), BBOATCB_MASK, PPCCOM,	{ CR, BDM } },
{ "bnsl+",   BBOCB(16,BOF,CBSO,0,1), BBOATCB_MASK, PPCCOM,	{ CR, BDP } },
{ "bnsl",    BBOCB(16,BOF,CBSO,0,1), BBOATCB_MASK, COM,		{ CR, BD } },
{ "bnsa-",   BBOCB(16,BOF,CBSO,1,0), BBOATCB_MASK, PPCCOM,	{ CR, BDMA } },
{ "bnsa+",   BBOCB(16,BOF,CBSO,1,0), BBOATCB_MASK, PPCCOM,	{ CR, BDPA } },
{ "bnsa",    BBOCB(16,BOF,CBSO,1,0), BBOATCB_MASK, COM,		{ CR, BDA } },
{ "bnsla-",  BBOCB(16,BOF,CBSO,1,1), BBOATCB_MASK, PPCCOM,	{ CR, BDMA } },
{ "bnsla+",  BBOCB(16,BOF,CBSO,1,1), BBOATCB_MASK, PPCCOM,	{ CR, BDPA } },
{ "bnsla",   BBOCB(16,BOF,CBSO,1,1), BBOATCB_MASK, COM,		{ CR, BDA } },
{ "bnu-",    BBOCB(16,BOF,CBSO,0,0), BBOATCB_MASK, PPCCOM,	{ CR, BDM } },
{ "bnu+",    BBOCB(16,BOF,CBSO,0,0), BBOATCB_MASK, PPCCOM,	{ CR, BDP } },
{ "bnu",     BBOCB(16,BOF,CBSO,0,0), BBOATCB_MASK, PPCCOM,	{ CR, BD } },
{ "bnul-",   BBOCB(16,BOF,CBSO,0,1), BBOATCB_MASK, PPCCOM,	{ CR, BDM } },
{ "bnul+",   BBOCB(16,BOF,CBSO,0,1), BBOATCB_MASK, PPCCOM,	{ CR, BDP } },
{ "bnul",    BBOCB(16,BOF,CBSO,0,1), BBOATCB_MASK, PPCCOM,	{ CR, BD } },
{ "bnua-",   BBOCB(16,BOF,CBSO,1,0), BBOATCB_MASK, PPCCOM,	{ CR, BDMA } },
{ "bnua+",   BBOCB(16,BOF,CBSO,1,0), BBOATCB_MASK, PPCCOM,	{ CR, BDPA } },
{ "bnua",    BBOCB(16,BOF,CBSO,1,0), BBOATCB_MASK, PPCCOM,	{ CR, BDA } },
{ "bnula-",  BBOCB(16,BOF,CBSO,1,1), BBOATCB_MASK, PPCCOM,	{ CR, BDMA } },
{ "bnula+",  BBOCB(16,BOF,CBSO,1,1), BBOATCB_MASK, PPCCOM,	{ CR, BDPA } },
{ "bnula",   BBOCB(16,BOF,CBSO,1,1), BBOATCB_MASK, PPCCOM,	{ CR, BDA } },
{ "bdnzt-",  BBO(16,BODNZT,0,0), BBOY_MASK, NOPOWER4,	{ BI, BDM } },
{ "bdnzt+",  BBO(16,BODNZT,0,0), BBOY_MASK, NOPOWER4,	{ BI, BDP } },
{ "bdnzt",   BBO(16,BODNZT,0,0), BBOY_MASK, PPCCOM,	{ BI, BD } },
{ "bdnztl-", BBO(16,BODNZT,0,1), BBOY_MASK, NOPOWER4,	{ BI, BDM } },
{ "bdnztl+", BBO(16,BODNZT,0,1), BBOY_MASK, NOPOWER4,	{ BI, BDP } },
{ "bdnztl",  BBO(16,BODNZT,0,1), BBOY_MASK, PPCCOM,	{ BI, BD } },
{ "bdnzta-", BBO(16,BODNZT,1,0), BBOY_MASK, NOPOWER4,	{ BI, BDMA } },
{ "bdnzta+", BBO(16,BODNZT,1,0), BBOY_MASK, NOPOWER4,	{ BI, BDPA } },
{ "bdnzta",  BBO(16,BODNZT,1,0), BBOY_MASK, PPCCOM,	{ BI, BDA } },
{ "bdnztla-",BBO(16,BODNZT,1,1), BBOY_MASK, NOPOWER4,	{ BI, BDMA } },
{ "bdnztla+",BBO(16,BODNZT,1,1), BBOY_MASK, NOPOWER4,	{ BI, BDPA } },
{ "bdnztla", BBO(16,BODNZT,1,1), BBOY_MASK, PPCCOM,	{ BI, BDA } },
{ "bdnzf-",  BBO(16,BODNZF,0,0), BBOY_MASK, NOPOWER4,	{ BI, BDM } },
{ "bdnzf+",  BBO(16,BODNZF,0,0), BBOY_MASK, NOPOWER4,	{ BI, BDP } },
{ "bdnzf",   BBO(16,BODNZF,0,0), BBOY_MASK, PPCCOM,	{ BI, BD } },
{ "bdnzfl-", BBO(16,BODNZF,0,1), BBOY_MASK, NOPOWER4,	{ BI, BDM } },
{ "bdnzfl+", BBO(16,BODNZF,0,1), BBOY_MASK, NOPOWER4,	{ BI, BDP } },
{ "bdnzfl",  BBO(16,BODNZF,0,1), BBOY_MASK, PPCCOM,	{ BI, BD } },
{ "bdnzfa-", BBO(16,BODNZF,1,0), BBOY_MASK, NOPOWER4,	{ BI, BDMA } },
{ "bdnzfa+", BBO(16,BODNZF,1,0), BBOY_MASK, NOPOWER4,	{ BI, BDPA } },
{ "bdnzfa",  BBO(16,BODNZF,1,0), BBOY_MASK, PPCCOM,	{ BI, BDA } },
{ "bdnzfla-",BBO(16,BODNZF,1,1), BBOY_MASK, NOPOWER4,	{ BI, BDMA } },
{ "bdnzfla+",BBO(16,BODNZF,1,1), BBOY_MASK, NOPOWER4,	{ BI, BDPA } },
{ "bdnzfla", BBO(16,BODNZF,1,1), BBOY_MASK, PPCCOM,	{ BI, BDA } },
{ "bt-",     BBO(16,BOT,0,0), BBOAT_MASK, PPCCOM,	{ BI, BDM } },
{ "bt+",     BBO(16,BOT,0,0), BBOAT_MASK, PPCCOM,	{ BI, BDP } },
{ "bt",	     BBO(16,BOT,0,0), BBOAT_MASK, PPCCOM,	{ BI, BD } },
{ "bbt",     BBO(16,BOT,0,0), BBOAT_MASK, PWRCOM,	{ BI, BD } },
{ "btl-",    BBO(16,BOT,0,1), BBOAT_MASK, PPCCOM,	{ BI, BDM } },
{ "btl+",    BBO(16,BOT,0,1), BBOAT_MASK, PPCCOM,	{ BI, BDP } },
{ "btl",     BBO(16,BOT,0,1), BBOAT_MASK, PPCCOM,	{ BI, BD } },
{ "bbtl",    BBO(16,BOT,0,1), BBOAT_MASK, PWRCOM,	{ BI, BD } },
{ "bta-",    BBO(16,BOT,1,0), BBOAT_MASK, PPCCOM,	{ BI, BDMA } },
{ "bta+",    BBO(16,BOT,1,0), BBOAT_MASK, PPCCOM,	{ BI, BDPA } },
{ "bta",     BBO(16,BOT,1,0), BBOAT_MASK, PPCCOM,	{ BI, BDA } },
{ "bbta",    BBO(16,BOT,1,0), BBOAT_MASK, PWRCOM,	{ BI, BDA } },
{ "btla-",   BBO(16,BOT,1,1), BBOAT_MASK, PPCCOM,	{ BI, BDMA } },
{ "btla+",   BBO(16,BOT,1,1), BBOAT_MASK, PPCCOM,	{ BI, BDPA } },
{ "btla",    BBO(16,BOT,1,1), BBOAT_MASK, PPCCOM,	{ BI, BDA } },
{ "bbtla",   BBO(16,BOT,1,1), BBOAT_MASK, PWRCOM,	{ BI, BDA } },
{ "bf-",     BBO(16,BOF,0,0), BBOAT_MASK, PPCCOM,	{ BI, BDM } },
{ "bf+",     BBO(16,BOF,0,0), BBOAT_MASK, PPCCOM,	{ BI, BDP } },
{ "bf",	     BBO(16,BOF,0,0), BBOAT_MASK, PPCCOM,	{ BI, BD } },
{ "bbf",     BBO(16,BOF,0,0), BBOAT_MASK, PWRCOM,	{ BI, BD } },
{ "bfl-",    BBO(16,BOF,0,1), BBOAT_MASK, PPCCOM,	{ BI, BDM } },
{ "bfl+",    BBO(16,BOF,0,1), BBOAT_MASK, PPCCOM,	{ BI, BDP } },
{ "bfl",     BBO(16,BOF,0,1), BBOAT_MASK, PPCCOM,	{ BI, BD } },
{ "bbfl",    BBO(16,BOF,0,1), BBOAT_MASK, PWRCOM,	{ BI, BD } },
{ "bfa-",    BBO(16,BOF,1,0), BBOAT_MASK, PPCCOM,	{ BI, BDMA } },
{ "bfa+",    BBO(16,BOF,1,0), BBOAT_MASK, PPCCOM,	{ BI, BDPA } },
{ "bfa",     BBO(16,BOF,1,0), BBOAT_MASK, PPCCOM,	{ BI, BDA } },
{ "bbfa",    BBO(16,BOF,1,0), BBOAT_MASK, PWRCOM,	{ BI, BDA } },
{ "bfla-",   BBO(16,BOF,1,1), BBOAT_MASK, PPCCOM,	{ BI, BDMA } },
{ "bfla+",   BBO(16,BOF,1,1), BBOAT_MASK, PPCCOM,	{ BI, BDPA } },
{ "bfla",    BBO(16,BOF,1,1), BBOAT_MASK, PPCCOM,	{ BI, BDA } },
{ "bbfla",   BBO(16,BOF,1,1), BBOAT_MASK, PWRCOM,	{ BI, BDA } },
{ "bdzt-",   BBO(16,BODZT,0,0), BBOY_MASK, NOPOWER4,	{ BI, BDM } },
{ "bdzt+",   BBO(16,BODZT,0,0), BBOY_MASK, NOPOWER4,	{ BI, BDP } },
{ "bdzt",    BBO(16,BODZT,0,0), BBOY_MASK, PPCCOM,	{ BI, BD } },
{ "bdztl-",  BBO(16,BODZT,0,1), BBOY_MASK, NOPOWER4,	{ BI, BDM } },
{ "bdztl+",  BBO(16,BODZT,0,1), BBOY_MASK, NOPOWER4,	{ BI, BDP } },
{ "bdztl",   BBO(16,BODZT,0,1), BBOY_MASK, PPCCOM,	{ BI, BD } },
{ "bdzta-",  BBO(16,BODZT,1,0), BBOY_MASK, NOPOWER4,	{ BI, BDMA } },
{ "bdzta+",  BBO(16,BODZT,1,0), BBOY_MASK, NOPOWER4,	{ BI, BDPA } },
{ "bdzta",   BBO(16,BODZT,1,0), BBOY_MASK, PPCCOM,	{ BI, BDA } },
{ "bdztla-", BBO(16,BODZT,1,1), BBOY_MASK, NOPOWER4,	{ BI, BDMA } },
{ "bdztla+", BBO(16,BODZT,1,1), BBOY_MASK, NOPOWER4,	{ BI, BDPA } },
{ "bdztla",  BBO(16,BODZT,1,1), BBOY_MASK, PPCCOM,	{ BI, BDA } },
{ "bdzf-",   BBO(16,BODZF,0,0), BBOY_MASK, NOPOWER4,	{ BI, BDM } },
{ "bdzf+",   BBO(16,BODZF,0,0), BBOY_MASK, NOPOWER4,	{ BI, BDP } },
{ "bdzf",    BBO(16,BODZF,0,0), BBOY_MASK, PPCCOM,	{ BI, BD } },
{ "bdzfl-",  BBO(16,BODZF,0,1), BBOY_MASK, NOPOWER4,	{ BI, BDM } },
{ "bdzfl+",  BBO(16,BODZF,0,1), BBOY_MASK, NOPOWER4,	{ BI, BDP } },
{ "bdzfl",   BBO(16,BODZF,0,1), BBOY_MASK, PPCCOM,	{ BI, BD } },
{ "bdzfa-",  BBO(16,BODZF,1,0), BBOY_MASK, NOPOWER4,	{ BI, BDMA } },
{ "bdzfa+",  BBO(16,BODZF,1,0), BBOY_MASK, NOPOWER4,	{ BI, BDPA } },
{ "bdzfa",   BBO(16,BODZF,1,0), BBOY_MASK, PPCCOM,	{ BI, BDA } },
{ "bdzfla-", BBO(16,BODZF,1,1), BBOY_MASK, NOPOWER4,	{ BI, BDMA } },
{ "bdzfla+", BBO(16,BODZF,1,1), BBOY_MASK, NOPOWER4,	{ BI, BDPA } },
{ "bdzfla",  BBO(16,BODZF,1,1), BBOY_MASK, PPCCOM,	{ BI, BDA } },
{ "bc-",     B(16,0,0),	B_MASK,		PPCCOM,		{ BOE, BI, BDM } },
{ "bc+",     B(16,0,0),	B_MASK,		PPCCOM,		{ BOE, BI, BDP } },
{ "bc",	     B(16,0,0),	B_MASK,		COM,		{ BO, BI, BD } },
{ "bcl-",    B(16,0,1),	B_MASK,		PPCCOM,		{ BOE, BI, BDM } },
{ "bcl+",    B(16,0,1),	B_MASK,		PPCCOM,		{ BOE, BI, BDP } },
{ "bcl",     B(16,0,1),	B_MASK,		COM,		{ BO, BI, BD } },
{ "bca-",    B(16,1,0),	B_MASK,		PPCCOM,		{ BOE, BI, BDMA } },
{ "bca+",    B(16,1,0),	B_MASK,		PPCCOM,		{ BOE, BI, BDPA } },
{ "bca",     B(16,1,0),	B_MASK,		COM,		{ BO, BI, BDA } },
{ "bcla-",   B(16,1,1),	B_MASK,		PPCCOM,		{ BOE, BI, BDMA } },
{ "bcla+",   B(16,1,1),	B_MASK,		PPCCOM,		{ BOE, BI, BDPA } },
{ "bcla",    B(16,1,1),	B_MASK,		COM,		{ BO, BI, BDA } },

{ "sc",      SC(17,1,0), 0xffffffff,	PPC,		{ 0 } },
{ "svc",     SC(17,0,0), SC_MASK,	POWER,		{ LEV, FL1, FL2 } },
{ "svcl",    SC(17,0,1), SC_MASK,	POWER,		{ LEV, FL1, FL2 } },
{ "svca",    SC(17,1,0), SC_MASK,	PWRCOM,		{ SV } },
{ "svcla",   SC(17,1,1), SC_MASK,	POWER,		{ SV } },

{ "b",	     B(18,0,0),	B_MASK,		COM,		{ LI } },
{ "bl",      B(18,0,1),	B_MASK,		COM,		{ LI } },
{ "ba",      B(18,1,0),	B_MASK,		COM,		{ LIA } },
{ "bla",     B(18,1,1),	B_MASK,		COM,		{ LIA } },

{ "mcrf",    XL(19,0),	XLBB_MASK|(3 << 21)|(3 << 16), COM,	{ BF, BFA } },

{ "blr",     XLO(19,BOU,16,0), XLBOBIBB_MASK, PPCCOM,	{ 0 } },
{ "br",      XLO(19,BOU,16,0), XLBOBIBB_MASK, PWRCOM,	{ 0 } },
{ "blrl",    XLO(19,BOU,16,1), XLBOBIBB_MASK, PPCCOM,	{ 0 } },
{ "brl",     XLO(19,BOU,16,1), XLBOBIBB_MASK, PWRCOM,	{ 0 } },
{ "bdnzlr",  XLO(19,BODNZ,16,0), XLBOBIBB_MASK, PPCCOM,	{ 0 } },
{ "bdnzlr-", XLO(19,BODNZ,16,0), XLBOBIBB_MASK, NOPOWER4,	{ 0 } },
{ "bdnzlr+", XLO(19,BODNZP,16,0), XLBOBIBB_MASK, NOPOWER4,	{ 0 } },
{ "bdnzlr-", XLO(19,BODNZM4,16,0), XLBOBIBB_MASK, POWER4,	{ 0 } },
{ "bdnzlr+", XLO(19,BODNZP4,16,0), XLBOBIBB_MASK, POWER4,	{ 0 } },
{ "bdnzlrl", XLO(19,BODNZ,16,1), XLBOBIBB_MASK, PPCCOM,	{ 0 } },
{ "bdnzlrl-",XLO(19,BODNZ,16,1), XLBOBIBB_MASK, NOPOWER4,	{ 0 } },
{ "bdnzlrl+",XLO(19,BODNZP,16,1), XLBOBIBB_MASK, NOPOWER4,	{ 0 } },
{ "bdnzlrl-",XLO(19,BODNZM4,16,1), XLBOBIBB_MASK, POWER4,	{ 0 } },
{ "bdnzlrl+",XLO(19,BODNZP4,16,1), XLBOBIBB_MASK, POWER4,	{ 0 } },
{ "bdzlr",   XLO(19,BODZ,16,0), XLBOBIBB_MASK, PPCCOM,	{ 0 } },
{ "bdzlr-",  XLO(19,BODZ,16,0), XLBOBIBB_MASK, NOPOWER4,	{ 0 } },
{ "bdzlr+",  XLO(19,BODZP,16,0), XLBOBIBB_MASK, NOPOWER4,	{ 0 } },
{ "bdzlr-",  XLO(19,BODZM4,16,0), XLBOBIBB_MASK, POWER4,	{ 0 } },
{ "bdzlr+",  XLO(19,BODZP4,16,0), XLBOBIBB_MASK, POWER4,	{ 0 } },
{ "bdzlrl",  XLO(19,BODZ,16,1), XLBOBIBB_MASK, PPCCOM,	{ 0 } },
{ "bdzlrl-", XLO(19,BODZ,16,1), XLBOBIBB_MASK, NOPOWER4,	{ 0 } },
{ "bdzlrl+", XLO(19,BODZP,16,1), XLBOBIBB_MASK, NOPOWER4,	{ 0 } },
{ "bdzlrl-", XLO(19,BODZM4,16,1), XLBOBIBB_MASK, POWER4,	{ 0 } },
{ "bdzlrl+", XLO(19,BODZP4,16,1), XLBOBIBB_MASK, POWER4,	{ 0 } },
{ "bltlr",   XLOCB(19,BOT,CBLT,16,0), XLBOCBBB_MASK, PPCCOM, { CR } },
{ "bltlr-",  XLOCB(19,BOT,CBLT,16,0), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bltlr+",  XLOCB(19,BOTP,CBLT,16,0), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bltlr-",  XLOCB(19,BOTM4,CBLT,16,0), XLBOCBBB_MASK, POWER4, { CR } },
{ "bltlr+",  XLOCB(19,BOTP4,CBLT,16,0), XLBOCBBB_MASK, POWER4, { CR } },
{ "bltr",    XLOCB(19,BOT,CBLT,16,0), XLBOCBBB_MASK, PWRCOM, { CR } },
{ "bltlrl",  XLOCB(19,BOT,CBLT,16,1), XLBOCBBB_MASK, PPCCOM, { CR } },
{ "bltlrl-", XLOCB(19,BOT,CBLT,16,1), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bltlrl+", XLOCB(19,BOTP,CBLT,16,1), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bltlrl-", XLOCB(19,BOTM4,CBLT,16,1), XLBOCBBB_MASK, POWER4, { CR } },
{ "bltlrl+", XLOCB(19,BOTP4,CBLT,16,1), XLBOCBBB_MASK, POWER4, { CR } },
{ "bltrl",   XLOCB(19,BOT,CBLT,16,1), XLBOCBBB_MASK, PWRCOM, { CR } },
{ "bgtlr",   XLOCB(19,BOT,CBGT,16,0), XLBOCBBB_MASK, PPCCOM, { CR } },
{ "bgtlr-",  XLOCB(19,BOT,CBGT,16,0), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bgtlr+",  XLOCB(19,BOTP,CBGT,16,0), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bgtlr-",  XLOCB(19,BOTM4,CBGT,16,0), XLBOCBBB_MASK, POWER4, { CR } },
{ "bgtlr+",  XLOCB(19,BOTP4,CBGT,16,0), XLBOCBBB_MASK, POWER4, { CR } },
{ "bgtr",    XLOCB(19,BOT,CBGT,16,0), XLBOCBBB_MASK, PWRCOM, { CR } },
{ "bgtlrl",  XLOCB(19,BOT,CBGT,16,1), XLBOCBBB_MASK, PPCCOM, { CR } },
{ "bgtlrl-", XLOCB(19,BOT,CBGT,16,1), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bgtlrl+", XLOCB(19,BOTP,CBGT,16,1), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bgtlrl-", XLOCB(19,BOTM4,CBGT,16,1), XLBOCBBB_MASK, POWER4, { CR } },
{ "bgtlrl+", XLOCB(19,BOTP4,CBGT,16,1), XLBOCBBB_MASK, POWER4, { CR } },
{ "bgtrl",   XLOCB(19,BOT,CBGT,16,1), XLBOCBBB_MASK, PWRCOM, { CR } },
{ "beqlr",   XLOCB(19,BOT,CBEQ,16,0), XLBOCBBB_MASK, PPCCOM, { CR } },
{ "beqlr-",  XLOCB(19,BOT,CBEQ,16,0), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "beqlr+",  XLOCB(19,BOTP,CBEQ,16,0), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "beqlr-",  XLOCB(19,BOTM4,CBEQ,16,0), XLBOCBBB_MASK, POWER4, { CR } },
{ "beqlr+",  XLOCB(19,BOTP4,CBEQ,16,0), XLBOCBBB_MASK, POWER4, { CR } },
{ "beqr",    XLOCB(19,BOT,CBEQ,16,0), XLBOCBBB_MASK, PWRCOM, { CR } },
{ "beqlrl",  XLOCB(19,BOT,CBEQ,16,1), XLBOCBBB_MASK, PPCCOM, { CR } },
{ "beqlrl-", XLOCB(19,BOT,CBEQ,16,1), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "beqlrl+", XLOCB(19,BOTP,CBEQ,16,1), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "beqlrl-", XLOCB(19,BOTM4,CBEQ,16,1), XLBOCBBB_MASK, POWER4, { CR } },
{ "beqlrl+", XLOCB(19,BOTP4,CBEQ,16,1), XLBOCBBB_MASK, POWER4, { CR } },
{ "beqrl",   XLOCB(19,BOT,CBEQ,16,1), XLBOCBBB_MASK, PWRCOM, { CR } },
{ "bsolr",   XLOCB(19,BOT,CBSO,16,0), XLBOCBBB_MASK, PPCCOM, { CR } },
{ "bsolr-",  XLOCB(19,BOT,CBSO,16,0), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bsolr+",  XLOCB(19,BOTP,CBSO,16,0), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bsolr-",  XLOCB(19,BOTM4,CBSO,16,0), XLBOCBBB_MASK, POWER4, { CR } },
{ "bsolr+",  XLOCB(19,BOTP4,CBSO,16,0), XLBOCBBB_MASK, POWER4, { CR } },
{ "bsor",    XLOCB(19,BOT,CBSO,16,0), XLBOCBBB_MASK, PWRCOM, { CR } },
{ "bsolrl",  XLOCB(19,BOT,CBSO,16,1), XLBOCBBB_MASK, PPCCOM, { CR } },
{ "bsolrl-", XLOCB(19,BOT,CBSO,16,1), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bsolrl+", XLOCB(19,BOTP,CBSO,16,1), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bsolrl-", XLOCB(19,BOTM4,CBSO,16,1), XLBOCBBB_MASK, POWER4, { CR } },
{ "bsolrl+", XLOCB(19,BOTP4,CBSO,16,1), XLBOCBBB_MASK, POWER4, { CR } },
{ "bsorl",   XLOCB(19,BOT,CBSO,16,1), XLBOCBBB_MASK, PWRCOM, { CR } },
{ "bunlr",   XLOCB(19,BOT,CBSO,16,0), XLBOCBBB_MASK, PPCCOM, { CR } },
{ "bunlr-",  XLOCB(19,BOT,CBSO,16,0), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bunlr+",  XLOCB(19,BOTP,CBSO,16,0), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bunlr-",  XLOCB(19,BOTM4,CBSO,16,0), XLBOCBBB_MASK, POWER4, { CR } },
{ "bunlr+",  XLOCB(19,BOTP4,CBSO,16,0), XLBOCBBB_MASK, POWER4, { CR } },
{ "bunlrl",  XLOCB(19,BOT,CBSO,16,1), XLBOCBBB_MASK, PPCCOM, { CR } },
{ "bunlrl-", XLOCB(19,BOT,CBSO,16,1), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bunlrl+", XLOCB(19,BOTP,CBSO,16,1), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bunlrl-", XLOCB(19,BOTM4,CBSO,16,1), XLBOCBBB_MASK, POWER4, { CR } },
{ "bunlrl+", XLOCB(19,BOTP4,CBSO,16,1), XLBOCBBB_MASK, POWER4, { CR } },
{ "bgelr",   XLOCB(19,BOF,CBLT,16,0), XLBOCBBB_MASK, PPCCOM, { CR } },
{ "bgelr-",  XLOCB(19,BOF,CBLT,16,0), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bgelr+",  XLOCB(19,BOFP,CBLT,16,0), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bgelr-",  XLOCB(19,BOFM4,CBLT,16,0), XLBOCBBB_MASK, POWER4, { CR } },
{ "bgelr+",  XLOCB(19,BOFP4,CBLT,16,0), XLBOCBBB_MASK, POWER4, { CR } },
{ "bger",    XLOCB(19,BOF,CBLT,16,0), XLBOCBBB_MASK, PWRCOM, { CR } },
{ "bgelrl",  XLOCB(19,BOF,CBLT,16,1), XLBOCBBB_MASK, PPCCOM, { CR } },
{ "bgelrl-", XLOCB(19,BOF,CBLT,16,1), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bgelrl+", XLOCB(19,BOFP,CBLT,16,1), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bgelrl-", XLOCB(19,BOFM4,CBLT,16,1), XLBOCBBB_MASK, POWER4, { CR } },
{ "bgelrl+", XLOCB(19,BOFP4,CBLT,16,1), XLBOCBBB_MASK, POWER4, { CR } },
{ "bgerl",   XLOCB(19,BOF,CBLT,16,1), XLBOCBBB_MASK, PWRCOM, { CR } },
{ "bnllr",   XLOCB(19,BOF,CBLT,16,0), XLBOCBBB_MASK, PPCCOM, { CR } },
{ "bnllr-",  XLOCB(19,BOF,CBLT,16,0), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bnllr+",  XLOCB(19,BOFP,CBLT,16,0), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bnllr-",  XLOCB(19,BOFM4,CBLT,16,0), XLBOCBBB_MASK, POWER4, { CR } },
{ "bnllr+",  XLOCB(19,BOFP4,CBLT,16,0), XLBOCBBB_MASK, POWER4, { CR } },
{ "bnlr",    XLOCB(19,BOF,CBLT,16,0), XLBOCBBB_MASK, PWRCOM, { CR } },
{ "bnllrl",  XLOCB(19,BOF,CBLT,16,1), XLBOCBBB_MASK, PPCCOM, { CR } },
{ "bnllrl-", XLOCB(19,BOF,CBLT,16,1), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bnllrl+", XLOCB(19,BOFP,CBLT,16,1), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bnllrl-", XLOCB(19,BOFM4,CBLT,16,1), XLBOCBBB_MASK, POWER4, { CR } },
{ "bnllrl+", XLOCB(19,BOFP4,CBLT,16,1), XLBOCBBB_MASK, POWER4, { CR } },
{ "bnlrl",   XLOCB(19,BOF,CBLT,16,1), XLBOCBBB_MASK, PWRCOM, { CR } },
{ "blelr",   XLOCB(19,BOF,CBGT,16,0), XLBOCBBB_MASK, PPCCOM, { CR } },
{ "blelr-",  XLOCB(19,BOF,CBGT,16,0), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "blelr+",  XLOCB(19,BOFP,CBGT,16,0), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "blelr-",  XLOCB(19,BOFM4,CBGT,16,0), XLBOCBBB_MASK, POWER4, { CR } },
{ "blelr+",  XLOCB(19,BOFP4,CBGT,16,0), XLBOCBBB_MASK, POWER4, { CR } },
{ "bler",    XLOCB(19,BOF,CBGT,16,0), XLBOCBBB_MASK, PWRCOM, { CR } },
{ "blelrl",  XLOCB(19,BOF,CBGT,16,1), XLBOCBBB_MASK, PPCCOM, { CR } },
{ "blelrl-", XLOCB(19,BOF,CBGT,16,1), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "blelrl+", XLOCB(19,BOFP,CBGT,16,1), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "blelrl-", XLOCB(19,BOFM4,CBGT,16,1), XLBOCBBB_MASK, POWER4, { CR } },
{ "blelrl+", XLOCB(19,BOFP4,CBGT,16,1), XLBOCBBB_MASK, POWER4, { CR } },
{ "blerl",   XLOCB(19,BOF,CBGT,16,1), XLBOCBBB_MASK, PWRCOM, { CR } },
{ "bnglr",   XLOCB(19,BOF,CBGT,16,0), XLBOCBBB_MASK, PPCCOM, { CR } },
{ "bnglr-",  XLOCB(19,BOF,CBGT,16,0), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bnglr+",  XLOCB(19,BOFP,CBGT,16,0), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bnglr-",  XLOCB(19,BOFM4,CBGT,16,0), XLBOCBBB_MASK, POWER4, { CR } },
{ "bnglr+",  XLOCB(19,BOFP4,CBGT,16,0), XLBOCBBB_MASK, POWER4, { CR } },
{ "bngr",    XLOCB(19,BOF,CBGT,16,0), XLBOCBBB_MASK, PWRCOM, { CR } },
{ "bnglrl",  XLOCB(19,BOF,CBGT,16,1), XLBOCBBB_MASK, PPCCOM, { CR } },
{ "bnglrl-", XLOCB(19,BOF,CBGT,16,1), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bnglrl+", XLOCB(19,BOFP,CBGT,16,1), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bnglrl-", XLOCB(19,BOFM4,CBGT,16,1), XLBOCBBB_MASK, POWER4, { CR } },
{ "bnglrl+", XLOCB(19,BOFP4,CBGT,16,1), XLBOCBBB_MASK, POWER4, { CR } },
{ "bngrl",   XLOCB(19,BOF,CBGT,16,1), XLBOCBBB_MASK, PWRCOM, { CR } },
{ "bnelr",   XLOCB(19,BOF,CBEQ,16,0), XLBOCBBB_MASK, PPCCOM, { CR } },
{ "bnelr-",  XLOCB(19,BOF,CBEQ,16,0), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bnelr+",  XLOCB(19,BOFP,CBEQ,16,0), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bnelr-",  XLOCB(19,BOFM4,CBEQ,16,0), XLBOCBBB_MASK, POWER4, { CR } },
{ "bnelr+",  XLOCB(19,BOFP4,CBEQ,16,0), XLBOCBBB_MASK, POWER4, { CR } },
{ "bner",    XLOCB(19,BOF,CBEQ,16,0), XLBOCBBB_MASK, PWRCOM, { CR } },
{ "bnelrl",  XLOCB(19,BOF,CBEQ,16,1), XLBOCBBB_MASK, PPCCOM, { CR } },
{ "bnelrl-", XLOCB(19,BOF,CBEQ,16,1), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bnelrl+", XLOCB(19,BOFP,CBEQ,16,1), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bnelrl-", XLOCB(19,BOFM4,CBEQ,16,1), XLBOCBBB_MASK, POWER4, { CR } },
{ "bnelrl+", XLOCB(19,BOFP4,CBEQ,16,1), XLBOCBBB_MASK, POWER4, { CR } },
{ "bnerl",   XLOCB(19,BOF,CBEQ,16,1), XLBOCBBB_MASK, PWRCOM, { CR } },
{ "bnslr",   XLOCB(19,BOF,CBSO,16,0), XLBOCBBB_MASK, PPCCOM, { CR } },
{ "bnslr-",  XLOCB(19,BOF,CBSO,16,0), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bnslr+",  XLOCB(19,BOFP,CBSO,16,0), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bnslr-",  XLOCB(19,BOFM4,CBSO,16,0), XLBOCBBB_MASK, POWER4, { CR } },
{ "bnslr+",  XLOCB(19,BOFP4,CBSO,16,0), XLBOCBBB_MASK, POWER4, { CR } },
{ "bnsr",    XLOCB(19,BOF,CBSO,16,0), XLBOCBBB_MASK, PWRCOM, { CR } },
{ "bnslrl",  XLOCB(19,BOF,CBSO,16,1), XLBOCBBB_MASK, PPCCOM, { CR } },
{ "bnslrl-", XLOCB(19,BOF,CBSO,16,1), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bnslrl+", XLOCB(19,BOFP,CBSO,16,1), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bnslrl-", XLOCB(19,BOFM4,CBSO,16,1), XLBOCBBB_MASK, POWER4, { CR } },
{ "bnslrl+", XLOCB(19,BOFP4,CBSO,16,1), XLBOCBBB_MASK, POWER4, { CR } },
{ "bnsrl",   XLOCB(19,BOF,CBSO,16,1), XLBOCBBB_MASK, PWRCOM, { CR } },
{ "bnulr",   XLOCB(19,BOF,CBSO,16,0), XLBOCBBB_MASK, PPCCOM, { CR } },
{ "bnulr-",  XLOCB(19,BOF,CBSO,16,0), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bnulr+",  XLOCB(19,BOFP,CBSO,16,0), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bnulr-",  XLOCB(19,BOFM4,CBSO,16,0), XLBOCBBB_MASK, POWER4, { CR } },
{ "bnulr+",  XLOCB(19,BOFP4,CBSO,16,0), XLBOCBBB_MASK, POWER4, { CR } },
{ "bnulrl",  XLOCB(19,BOF,CBSO,16,1), XLBOCBBB_MASK, PPCCOM, { CR } },
{ "bnulrl-", XLOCB(19,BOF,CBSO,16,1), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bnulrl+", XLOCB(19,BOFP,CBSO,16,1), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bnulrl-", XLOCB(19,BOFM4,CBSO,16,1), XLBOCBBB_MASK, POWER4, { CR } },
{ "bnulrl+", XLOCB(19,BOFP4,CBSO,16,1), XLBOCBBB_MASK, POWER4, { CR } },
{ "btlr",    XLO(19,BOT,16,0), XLBOBB_MASK, PPCCOM,	{ BI } },
{ "btlr-",   XLO(19,BOT,16,0), XLBOBB_MASK, NOPOWER4,	{ BI } },
{ "btlr+",   XLO(19,BOTP,16,0), XLBOBB_MASK, NOPOWER4,	{ BI } },
{ "btlr-",   XLO(19,BOTM4,16,0), XLBOBB_MASK, POWER4,	{ BI } },
{ "btlr+",   XLO(19,BOTP4,16,0), XLBOBB_MASK, POWER4,	{ BI } },
{ "bbtr",    XLO(19,BOT,16,0), XLBOBB_MASK, PWRCOM,	{ BI } },
{ "btlrl",   XLO(19,BOT,16,1), XLBOBB_MASK, PPCCOM,	{ BI } },
{ "btlrl-",  XLO(19,BOT,16,1), XLBOBB_MASK, NOPOWER4,	{ BI } },
{ "btlrl+",  XLO(19,BOTP,16,1), XLBOBB_MASK, NOPOWER4,	{ BI } },
{ "btlrl-",  XLO(19,BOTM4,16,1), XLBOBB_MASK, POWER4,	{ BI } },
{ "btlrl+",  XLO(19,BOTP4,16,1), XLBOBB_MASK, POWER4,	{ BI } },
{ "bbtrl",   XLO(19,BOT,16,1), XLBOBB_MASK, PWRCOM,	{ BI } },
{ "bflr",    XLO(19,BOF,16,0), XLBOBB_MASK, PPCCOM,	{ BI } },
{ "bflr-",   XLO(19,BOF,16,0), XLBOBB_MASK, NOPOWER4,	{ BI } },
{ "bflr+",   XLO(19,BOFP,16,0), XLBOBB_MASK, NOPOWER4,	{ BI } },
{ "bflr-",   XLO(19,BOFM4,16,0), XLBOBB_MASK, POWER4,	{ BI } },
{ "bflr+",   XLO(19,BOFP4,16,0), XLBOBB_MASK, POWER4,	{ BI } },
{ "bbfr",    XLO(19,BOF,16,0), XLBOBB_MASK, PWRCOM,	{ BI } },
{ "bflrl",   XLO(19,BOF,16,1), XLBOBB_MASK, PPCCOM,	{ BI } },
{ "bflrl-",  XLO(19,BOF,16,1), XLBOBB_MASK, NOPOWER4,	{ BI } },
{ "bflrl+",  XLO(19,BOFP,16,1), XLBOBB_MASK, NOPOWER4,	{ BI } },
{ "bflrl-",  XLO(19,BOFM4,16,1), XLBOBB_MASK, POWER4,	{ BI } },
{ "bflrl+",  XLO(19,BOFP4,16,1), XLBOBB_MASK, POWER4,	{ BI } },
{ "bbfrl",   XLO(19,BOF,16,1), XLBOBB_MASK, PWRCOM,	{ BI } },
{ "bdnztlr", XLO(19,BODNZT,16,0), XLBOBB_MASK, PPCCOM,	{ BI } },
{ "bdnztlr-",XLO(19,BODNZT,16,0), XLBOBB_MASK, NOPOWER4, { BI } },
{ "bdnztlr+",XLO(19,BODNZTP,16,0), XLBOBB_MASK, NOPOWER4, { BI } },
{ "bdnztlrl",XLO(19,BODNZT,16,1), XLBOBB_MASK, PPCCOM,	{ BI } },
{ "bdnztlrl-",XLO(19,BODNZT,16,1), XLBOBB_MASK, NOPOWER4, { BI } },
{ "bdnztlrl+",XLO(19,BODNZTP,16,1), XLBOBB_MASK, NOPOWER4, { BI } },
{ "bdnzflr", XLO(19,BODNZF,16,0), XLBOBB_MASK, PPCCOM,	{ BI } },
{ "bdnzflr-",XLO(19,BODNZF,16,0), XLBOBB_MASK, NOPOWER4, { BI } },
{ "bdnzflr+",XLO(19,BODNZFP,16,0), XLBOBB_MASK, NOPOWER4, { BI } },
{ "bdnzflrl",XLO(19,BODNZF,16,1), XLBOBB_MASK, PPCCOM,	{ BI } },
{ "bdnzflrl-",XLO(19,BODNZF,16,1), XLBOBB_MASK, NOPOWER4, { BI } },
{ "bdnzflrl+",XLO(19,BODNZFP,16,1), XLBOBB_MASK, NOPOWER4, { BI } },
{ "bdztlr",  XLO(19,BODZT,16,0), XLBOBB_MASK, PPCCOM,	{ BI } },
{ "bdztlr-", XLO(19,BODZT,16,0), XLBOBB_MASK, NOPOWER4,	{ BI } },
{ "bdztlr+", XLO(19,BODZTP,16,0), XLBOBB_MASK, NOPOWER4, { BI } },
{ "bdztlrl", XLO(19,BODZT,16,1), XLBOBB_MASK, PPCCOM,	{ BI } },
{ "bdztlrl-",XLO(19,BODZT,16,1), XLBOBB_MASK, NOPOWER4,	{ BI } },
{ "bdztlrl+",XLO(19,BODZTP,16,1), XLBOBB_MASK, NOPOWER4, { BI } },
{ "bdzflr",  XLO(19,BODZF,16,0), XLBOBB_MASK, PPCCOM,	{ BI } },
{ "bdzflr-", XLO(19,BODZF,16,0), XLBOBB_MASK, NOPOWER4,	{ BI } },
{ "bdzflr+", XLO(19,BODZFP,16,0), XLBOBB_MASK, NOPOWER4, { BI } },
{ "bdzflrl", XLO(19,BODZF,16,1), XLBOBB_MASK, PPCCOM,	{ BI } },
{ "bdzflrl-",XLO(19,BODZF,16,1), XLBOBB_MASK, NOPOWER4,	{ BI } },
{ "bdzflrl+",XLO(19,BODZFP,16,1), XLBOBB_MASK, NOPOWER4, { BI } },
{ "bclr",    XLLK(19,16,0), XLYBB_MASK,	PPCCOM,		{ BO, BI } },
{ "bclrl",   XLLK(19,16,1), XLYBB_MASK,	PPCCOM,		{ BO, BI } },
{ "bclr+",   XLYLK(19,16,1,0), XLYBB_MASK, PPCCOM,	{ BOE, BI } },
{ "bclrl+",  XLYLK(19,16,1,1), XLYBB_MASK, PPCCOM,	{ BOE, BI } },
{ "bclr-",   XLYLK(19,16,0,0), XLYBB_MASK, PPCCOM,	{ BOE, BI } },
{ "bclrl-",  XLYLK(19,16,0,1), XLYBB_MASK, PPCCOM,	{ BOE, BI } },
{ "bcr",     XLLK(19,16,0), XLBB_MASK,	PWRCOM,		{ BO, BI } },
{ "bcrl",    XLLK(19,16,1), XLBB_MASK,	PWRCOM,		{ BO, BI } },
{ "bclre",   XLLK(19,17,0), XLBB_MASK,	BOOKE64,	{ BO, BI } },
{ "bclrel",  XLLK(19,17,1), XLBB_MASK,	BOOKE64,	{ BO, BI } },

{ "rfid",    XL(19,18),	0xffffffff,	PPC64,		{ 0 } },

{ "crnot",   XL(19,33), XL_MASK,	PPCCOM,		{ BT, BA, BBA } },
{ "crnor",   XL(19,33),	XL_MASK,	COM,		{ BT, BA, BB } },
{ "rfmci",    X(19,38), 0xffffffff,	PPCRFMCI,	{ 0 } },

{ "rfi",     XL(19,50),	0xffffffff,	COM,		{ 0 } },
{ "rfci",    XL(19,51),	0xffffffff,	PPC403,		{ 0 } },
{ "rfci",    XL(19,51),	0xffffffff,	BOOKE,		{ 0 } },

{ "rfsvc",   XL(19,82),	0xffffffff,	POWER,		{ 0 } },

{ "crandc",  XL(19,129), XL_MASK,	COM,		{ BT, BA, BB } },

{ "isync",   XL(19,150), 0xffffffff,	PPCCOM,		{ 0 } },
{ "ics",     XL(19,150), 0xffffffff,	PWRCOM,		{ 0 } },

{ "crclr",   XL(19,193), XL_MASK,	PPCCOM,		{ BT, BAT, BBA } },
{ "crxor",   XL(19,193), XL_MASK,	COM,		{ BT, BA, BB } },

{ "crnand",  XL(19,225), XL_MASK,	COM,		{ BT, BA, BB } },

{ "crand",   XL(19,257), XL_MASK,	COM,		{ BT, BA, BB } },

{ "crset",   XL(19,289), XL_MASK,	PPCCOM,		{ BT, BAT, BBA } },
{ "creqv",   XL(19,289), XL_MASK,	COM,		{ BT, BA, BB } },

{ "crorc",   XL(19,417), XL_MASK,	COM,		{ BT, BA, BB } },

{ "crmove",  XL(19,449), XL_MASK,	PPCCOM,		{ BT, BA, BBA } },
{ "cror",    XL(19,449), XL_MASK,	COM,		{ BT, BA, BB } },

{ "bctr",    XLO(19,BOU,528,0), XLBOBIBB_MASK, COM,	{ 0 } },
{ "bctrl",   XLO(19,BOU,528,1), XLBOBIBB_MASK, COM,	{ 0 } },
{ "bltctr",  XLOCB(19,BOT,CBLT,528,0),  XLBOCBBB_MASK, PPCCOM,	{ CR } },
{ "bltctr-", XLOCB(19,BOT,CBLT,528,0),  XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bltctr+", XLOCB(19,BOTP,CBLT,528,0), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bltctr-", XLOCB(19,BOTM4,CBLT,528,0), XLBOCBBB_MASK, POWER4, { CR } },
{ "bltctr+", XLOCB(19,BOTP4,CBLT,528,0), XLBOCBBB_MASK, POWER4, { CR } },
{ "bltctrl", XLOCB(19,BOT,CBLT,528,1),  XLBOCBBB_MASK, PPCCOM,	{ CR } },
{ "bltctrl-",XLOCB(19,BOT,CBLT,528,1),  XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bltctrl+",XLOCB(19,BOTP,CBLT,528,1), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bltctrl-",XLOCB(19,BOTM4,CBLT,528,1), XLBOCBBB_MASK, POWER4, { CR } },
{ "bltctrl+",XLOCB(19,BOTP4,CBLT,528,1), XLBOCBBB_MASK, POWER4, { CR } },
{ "bgtctr",  XLOCB(19,BOT,CBGT,528,0),  XLBOCBBB_MASK, PPCCOM,	{ CR } },
{ "bgtctr-", XLOCB(19,BOT,CBGT,528,0),  XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bgtctr+", XLOCB(19,BOTP,CBGT,528,0), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bgtctr-", XLOCB(19,BOTM4,CBGT,528,0), XLBOCBBB_MASK, POWER4, { CR } },
{ "bgtctr+", XLOCB(19,BOTP4,CBGT,528,0), XLBOCBBB_MASK, POWER4, { CR } },
{ "bgtctrl", XLOCB(19,BOT,CBGT,528,1),  XLBOCBBB_MASK, PPCCOM,	{ CR } },
{ "bgtctrl-",XLOCB(19,BOT,CBGT,528,1),  XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bgtctrl+",XLOCB(19,BOTP,CBGT,528,1), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bgtctrl-",XLOCB(19,BOTM4,CBGT,528,1), XLBOCBBB_MASK, POWER4, { CR } },
{ "bgtctrl+",XLOCB(19,BOTP4,CBGT,528,1), XLBOCBBB_MASK, POWER4, { CR } },
{ "beqctr",  XLOCB(19,BOT,CBEQ,528,0),  XLBOCBBB_MASK, PPCCOM,	{ CR } },
{ "beqctr-", XLOCB(19,BOT,CBEQ,528,0),  XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "beqctr+", XLOCB(19,BOTP,CBEQ,528,0), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "beqctr-", XLOCB(19,BOTM4,CBEQ,528,0), XLBOCBBB_MASK, POWER4, { CR } },
{ "beqctr+", XLOCB(19,BOTP4,CBEQ,528,0), XLBOCBBB_MASK, POWER4, { CR } },
{ "beqctrl", XLOCB(19,BOT,CBEQ,528,1),  XLBOCBBB_MASK, PPCCOM,	{ CR } },
{ "beqctrl-",XLOCB(19,BOT,CBEQ,528,1),  XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "beqctrl+",XLOCB(19,BOTP,CBEQ,528,1), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "beqctrl-",XLOCB(19,BOTM4,CBEQ,528,1), XLBOCBBB_MASK, POWER4, { CR } },
{ "beqctrl+",XLOCB(19,BOTP4,CBEQ,528,1), XLBOCBBB_MASK, POWER4, { CR } },
{ "bsoctr",  XLOCB(19,BOT,CBSO,528,0),  XLBOCBBB_MASK, PPCCOM,	{ CR } },
{ "bsoctr-", XLOCB(19,BOT,CBSO,528,0),  XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bsoctr+", XLOCB(19,BOTP,CBSO,528,0), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bsoctr-", XLOCB(19,BOTM4,CBSO,528,0), XLBOCBBB_MASK, POWER4, { CR } },
{ "bsoctr+", XLOCB(19,BOTP4,CBSO,528,0), XLBOCBBB_MASK, POWER4, { CR } },
{ "bsoctrl", XLOCB(19,BOT,CBSO,528,1),  XLBOCBBB_MASK, PPCCOM,	{ CR } },
{ "bsoctrl-",XLOCB(19,BOT,CBSO,528,1),  XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bsoctrl+",XLOCB(19,BOTP,CBSO,528,1), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bsoctrl-",XLOCB(19,BOTM4,CBSO,528,1), XLBOCBBB_MASK, POWER4, { CR } },
{ "bsoctrl+",XLOCB(19,BOTP4,CBSO,528,1), XLBOCBBB_MASK, POWER4, { CR } },
{ "bunctr",  XLOCB(19,BOT,CBSO,528,0),  XLBOCBBB_MASK, PPCCOM,	{ CR } },
{ "bunctr-", XLOCB(19,BOT,CBSO,528,0),  XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bunctr+", XLOCB(19,BOTP,CBSO,528,0), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bunctr-", XLOCB(19,BOTM4,CBSO,528,0), XLBOCBBB_MASK, POWER4, { CR } },
{ "bunctr+", XLOCB(19,BOTP4,CBSO,528,0), XLBOCBBB_MASK, POWER4, { CR } },
{ "bunctrl", XLOCB(19,BOT,CBSO,528,1),  XLBOCBBB_MASK, PPCCOM,	{ CR } },
{ "bunctrl-",XLOCB(19,BOT,CBSO,528,1),  XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bunctrl+",XLOCB(19,BOTP,CBSO,528,1), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bunctrl-",XLOCB(19,BOTM4,CBSO,528,1), XLBOCBBB_MASK, POWER4, { CR } },
{ "bunctrl+",XLOCB(19,BOTP4,CBSO,528,1), XLBOCBBB_MASK, POWER4, { CR } },
{ "bgectr",  XLOCB(19,BOF,CBLT,528,0),  XLBOCBBB_MASK, PPCCOM,	{ CR } },
{ "bgectr-", XLOCB(19,BOF,CBLT,528,0),  XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bgectr+", XLOCB(19,BOFP,CBLT,528,0), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bgectr-", XLOCB(19,BOFM4,CBLT,528,0), XLBOCBBB_MASK, POWER4, { CR } },
{ "bgectr+", XLOCB(19,BOFP4,CBLT,528,0), XLBOCBBB_MASK, POWER4, { CR } },
{ "bgectrl", XLOCB(19,BOF,CBLT,528,1),  XLBOCBBB_MASK, PPCCOM,	{ CR } },
{ "bgectrl-",XLOCB(19,BOF,CBLT,528,1),  XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bgectrl+",XLOCB(19,BOFP,CBLT,528,1), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bgectrl-",XLOCB(19,BOFM4,CBLT,528,1), XLBOCBBB_MASK, POWER4, { CR } },
{ "bgectrl+",XLOCB(19,BOFP4,CBLT,528,1), XLBOCBBB_MASK, POWER4, { CR } },
{ "bnlctr",  XLOCB(19,BOF,CBLT,528,0),  XLBOCBBB_MASK, PPCCOM,	{ CR } },
{ "bnlctr-", XLOCB(19,BOF,CBLT,528,0),  XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bnlctr+", XLOCB(19,BOFP,CBLT,528,0), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bnlctr-", XLOCB(19,BOFM4,CBLT,528,0), XLBOCBBB_MASK, POWER4, { CR } },
{ "bnlctr+", XLOCB(19,BOFP4,CBLT,528,0), XLBOCBBB_MASK, POWER4, { CR } },
{ "bnlctrl", XLOCB(19,BOF,CBLT,528,1),  XLBOCBBB_MASK, PPCCOM,	{ CR } },
{ "bnlctrl-",XLOCB(19,BOF,CBLT,528,1),  XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bnlctrl+",XLOCB(19,BOFP,CBLT,528,1), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bnlctrl-",XLOCB(19,BOFM4,CBLT,528,1), XLBOCBBB_MASK, POWER4, { CR } },
{ "bnlctrl+",XLOCB(19,BOFP4,CBLT,528,1), XLBOCBBB_MASK, POWER4, { CR } },
{ "blectr",  XLOCB(19,BOF,CBGT,528,0),  XLBOCBBB_MASK, PPCCOM,	{ CR } },
{ "blectr-", XLOCB(19,BOF,CBGT,528,0),  XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "blectr+", XLOCB(19,BOFP,CBGT,528,0), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "blectr-", XLOCB(19,BOFM4,CBGT,528,0), XLBOCBBB_MASK, POWER4, { CR } },
{ "blectr+", XLOCB(19,BOFP4,CBGT,528,0), XLBOCBBB_MASK, POWER4, { CR } },
{ "blectrl", XLOCB(19,BOF,CBGT,528,1),  XLBOCBBB_MASK, PPCCOM,	{ CR } },
{ "blectrl-",XLOCB(19,BOF,CBGT,528,1),  XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "blectrl+",XLOCB(19,BOFP,CBGT,528,1), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "blectrl-",XLOCB(19,BOFM4,CBGT,528,1), XLBOCBBB_MASK, POWER4, { CR } },
{ "blectrl+",XLOCB(19,BOFP4,CBGT,528,1), XLBOCBBB_MASK, POWER4, { CR } },
{ "bngctr",  XLOCB(19,BOF,CBGT,528,0),  XLBOCBBB_MASK, PPCCOM,	{ CR } },
{ "bngctr-", XLOCB(19,BOF,CBGT,528,0),  XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bngctr+", XLOCB(19,BOFP,CBGT,528,0), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bngctr-", XLOCB(19,BOFM4,CBGT,528,0), XLBOCBBB_MASK, POWER4, { CR } },
{ "bngctr+", XLOCB(19,BOFP4,CBGT,528,0), XLBOCBBB_MASK, POWER4, { CR } },
{ "bngctrl", XLOCB(19,BOF,CBGT,528,1),  XLBOCBBB_MASK, PPCCOM,	{ CR } },
{ "bngctrl-",XLOCB(19,BOF,CBGT,528,1),  XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bngctrl+",XLOCB(19,BOFP,CBGT,528,1), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bngctrl-",XLOCB(19,BOFM4,CBGT,528,1), XLBOCBBB_MASK, POWER4, { CR } },
{ "bngctrl+",XLOCB(19,BOFP4,CBGT,528,1), XLBOCBBB_MASK, POWER4, { CR } },
{ "bnectr",  XLOCB(19,BOF,CBEQ,528,0),  XLBOCBBB_MASK, PPCCOM,	{ CR } },
{ "bnectr-", XLOCB(19,BOF,CBEQ,528,0),  XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bnectr+", XLOCB(19,BOFP,CBEQ,528,0), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bnectr-", XLOCB(19,BOFM4,CBEQ,528,0), XLBOCBBB_MASK, POWER4, { CR } },
{ "bnectr+", XLOCB(19,BOFP4,CBEQ,528,0), XLBOCBBB_MASK, POWER4, { CR } },
{ "bnectrl", XLOCB(19,BOF,CBEQ,528,1),  XLBOCBBB_MASK, PPCCOM,	{ CR } },
{ "bnectrl-",XLOCB(19,BOF,CBEQ,528,1),  XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bnectrl+",XLOCB(19,BOFP,CBEQ,528,1), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bnectrl-",XLOCB(19,BOFM4,CBEQ,528,1), XLBOCBBB_MASK, POWER4, { CR } },
{ "bnectrl+",XLOCB(19,BOFP4,CBEQ,528,1), XLBOCBBB_MASK, POWER4, { CR } },
{ "bnsctr",  XLOCB(19,BOF,CBSO,528,0),  XLBOCBBB_MASK, PPCCOM,	{ CR } },
{ "bnsctr-", XLOCB(19,BOF,CBSO,528,0),  XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bnsctr+", XLOCB(19,BOFP,CBSO,528,0), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bnsctr-", XLOCB(19,BOFM4,CBSO,528,0), XLBOCBBB_MASK, POWER4, { CR } },
{ "bnsctr+", XLOCB(19,BOFP4,CBSO,528,0), XLBOCBBB_MASK, POWER4, { CR } },
{ "bnsctrl", XLOCB(19,BOF,CBSO,528,1),  XLBOCBBB_MASK, PPCCOM,	{ CR } },
{ "bnsctrl-",XLOCB(19,BOF,CBSO,528,1),  XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bnsctrl+",XLOCB(19,BOFP,CBSO,528,1), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bnsctrl-",XLOCB(19,BOFM4,CBSO,528,1), XLBOCBBB_MASK, POWER4, { CR } },
{ "bnsctrl+",XLOCB(19,BOFP4,CBSO,528,1), XLBOCBBB_MASK, POWER4, { CR } },
{ "bnuctr",  XLOCB(19,BOF,CBSO,528,0),  XLBOCBBB_MASK, PPCCOM,	{ CR } },
{ "bnuctr-", XLOCB(19,BOF,CBSO,528,0),  XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bnuctr+", XLOCB(19,BOFP,CBSO,528,0), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bnuctr-", XLOCB(19,BOFM4,CBSO,528,0), XLBOCBBB_MASK, POWER4, { CR } },
{ "bnuctr+", XLOCB(19,BOFP4,CBSO,528,0), XLBOCBBB_MASK, POWER4, { CR } },
{ "bnuctrl", XLOCB(19,BOF,CBSO,528,1),  XLBOCBBB_MASK, PPCCOM,	{ CR } },
{ "bnuctrl-",XLOCB(19,BOF,CBSO,528,1),  XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bnuctrl+",XLOCB(19,BOFP,CBSO,528,1), XLBOCBBB_MASK, NOPOWER4, { CR } },
{ "bnuctrl-",XLOCB(19,BOFM4,CBSO,528,1), XLBOCBBB_MASK, POWER4, { CR } },
{ "bnuctrl+",XLOCB(19,BOFP4,CBSO,528,1), XLBOCBBB_MASK, POWER4, { CR } },
{ "btctr",   XLO(19,BOT,528,0),  XLBOBB_MASK, PPCCOM,	{ BI } },
{ "btctr-",  XLO(19,BOT,528,0),  XLBOBB_MASK, NOPOWER4,	{ BI } },
{ "btctr+",  XLO(19,BOTP,528,0), XLBOBB_MASK, NOPOWER4,	{ BI } },
{ "btctr-",  XLO(19,BOTM4,528,0), XLBOBB_MASK, POWER4, { BI } },
{ "btctr+",  XLO(19,BOTP4,528,0), XLBOBB_MASK, POWER4, { BI } },
{ "btctrl",  XLO(19,BOT,528,1),  XLBOBB_MASK, PPCCOM,	{ BI } },
{ "btctrl-", XLO(19,BOT,528,1),  XLBOBB_MASK, NOPOWER4,	{ BI } },
{ "btctrl+", XLO(19,BOTP,528,1), XLBOBB_MASK, NOPOWER4,	{ BI } },
{ "btctrl-", XLO(19,BOTM4,528,1), XLBOBB_MASK, POWER4, { BI } },
{ "btctrl+", XLO(19,BOTP4,528,1), XLBOBB_MASK, POWER4, { BI } },
{ "bfctr",   XLO(19,BOF,528,0),  XLBOBB_MASK, PPCCOM,	{ BI } },
{ "bfctr-",  XLO(19,BOF,528,0),  XLBOBB_MASK, NOPOWER4, { BI } },
{ "bfctr+",  XLO(19,BOFP,528,0), XLBOBB_MASK, NOPOWER4, { BI } },
{ "bfctr-",  XLO(19,BOFM4,528,0), XLBOBB_MASK, POWER4, { BI } },
{ "bfctr+",  XLO(19,BOFP4,528,0), XLBOBB_MASK, POWER4, { BI } },
{ "bfctrl",  XLO(19,BOF,528,1),  XLBOBB_MASK, PPCCOM,	{ BI } },
{ "bfctrl-", XLO(19,BOF,528,1),  XLBOBB_MASK, NOPOWER4, { BI } },
{ "bfctrl+", XLO(19,BOFP,528,1), XLBOBB_MASK, NOPOWER4, { BI } },
{ "bfctrl-", XLO(19,BOFM4,528,1), XLBOBB_MASK, POWER4, { BI } },
{ "bfctrl+", XLO(19,BOFP4,528,1), XLBOBB_MASK, POWER4, { BI } },
{ "bcctr",   XLLK(19,528,0),     XLYBB_MASK,  PPCCOM,	{ BO, BI } },
{ "bcctr-",  XLYLK(19,528,0,0),  XLYBB_MASK,  PPCCOM,	{ BOE, BI } },
{ "bcctr+",  XLYLK(19,528,1,0),  XLYBB_MASK,  PPCCOM,	{ BOE, BI } },
{ "bcctrl",  XLLK(19,528,1),     XLYBB_MASK,  PPCCOM,	{ BO, BI } },
{ "bcctrl-", XLYLK(19,528,0,1),  XLYBB_MASK,  PPCCOM,	{ BOE, BI } },
{ "bcctrl+", XLYLK(19,528,1,1),  XLYBB_MASK,  PPCCOM,	{ BOE, BI } },
{ "bcc",     XLLK(19,528,0),     XLBB_MASK,   PWRCOM,	{ BO, BI } },
{ "bccl",    XLLK(19,528,1),     XLBB_MASK,   PWRCOM,	{ BO, BI } },
{ "bcctre",  XLLK(19,529,0),     XLYBB_MASK,  BOOKE64,	{ BO, BI } },
{ "bcctrel", XLLK(19,529,1),     XLYBB_MASK,  BOOKE64,	{ BO, BI } },

{ "rlwimi",  M(20,0),	M_MASK,		PPCCOM,		{ RA,RS,SH,MBE,ME } },
{ "rlimi",   M(20,0),	M_MASK,		PWRCOM,		{ RA,RS,SH,MBE,ME } },

{ "rlwimi.", M(20,1),	M_MASK,		PPCCOM,		{ RA,RS,SH,MBE,ME } },
{ "rlimi.",  M(20,1),	M_MASK,		PWRCOM,		{ RA,RS,SH,MBE,ME } },

{ "rotlwi",  MME(21,31,0), MMBME_MASK,	PPCCOM,		{ RA, RS, SH } },
{ "clrlwi",  MME(21,31,0), MSHME_MASK,	PPCCOM,		{ RA, RS, MB } },
{ "rlwinm",  M(21,0),	M_MASK,		PPCCOM,		{ RA,RS,SH,MBE,ME } },
{ "rlinm",   M(21,0),	M_MASK,		PWRCOM,		{ RA,RS,SH,MBE,ME } },
{ "rotlwi.", MME(21,31,1), MMBME_MASK,	PPCCOM,		{ RA,RS,SH } },
{ "clrlwi.", MME(21,31,1), MSHME_MASK,	PPCCOM,		{ RA, RS, MB } },
{ "rlwinm.", M(21,1),	M_MASK,		PPCCOM,		{ RA,RS,SH,MBE,ME } },
{ "rlinm.",  M(21,1),	M_MASK,		PWRCOM,		{ RA,RS,SH,MBE,ME } },

{ "rlmi",    M(22,0),	M_MASK,		M601,		{ RA,RS,RB,MBE,ME } },
{ "rlmi.",   M(22,1),	M_MASK,		M601,		{ RA,RS,RB,MBE,ME } },

{ "be",	     B(22,0,0),	B_MASK,		BOOKE64,	{ LI } },
{ "bel",     B(22,0,1),	B_MASK,		BOOKE64,	{ LI } },
{ "bea",     B(22,1,0),	B_MASK,		BOOKE64,	{ LIA } },
{ "bela",    B(22,1,1),	B_MASK,		BOOKE64,	{ LIA } },

{ "rotlw",   MME(23,31,0), MMBME_MASK,	PPCCOM,		{ RA, RS, RB } },
{ "rlwnm",   M(23,0),	M_MASK,		PPCCOM,		{ RA,RS,RB,MBE,ME } },
{ "rlnm",    M(23,0),	M_MASK,		PWRCOM,		{ RA,RS,RB,MBE,ME } },
{ "rotlw.",  MME(23,31,1), MMBME_MASK,	PPCCOM,		{ RA, RS, RB } },
{ "rlwnm.",  M(23,1),	M_MASK,		PPCCOM,		{ RA,RS,RB,MBE,ME } },
{ "rlnm.",   M(23,1),	M_MASK,		PWRCOM,		{ RA,RS,RB,MBE,ME } },

{ "nop",     OP(24),	0xffffffff,	PPCCOM,		{ 0 } },
{ "ori",     OP(24),	OP_MASK,	PPCCOM,		{ RA, RS, UI } },
{ "oril",    OP(24),	OP_MASK,	PWRCOM,		{ RA, RS, UI } },

{ "oris",    OP(25),	OP_MASK,	PPCCOM,		{ RA, RS, UI } },
{ "oriu",    OP(25),	OP_MASK,	PWRCOM,		{ RA, RS, UI } },

{ "xori",    OP(26),	OP_MASK,	PPCCOM,		{ RA, RS, UI } },
{ "xoril",   OP(26),	OP_MASK,	PWRCOM,		{ RA, RS, UI } },

{ "xoris",   OP(27),	OP_MASK,	PPCCOM,		{ RA, RS, UI } },
{ "xoriu",   OP(27),	OP_MASK,	PWRCOM,		{ RA, RS, UI } },

{ "andi.",   OP(28),	OP_MASK,	PPCCOM,		{ RA, RS, UI } },
{ "andil.",  OP(28),	OP_MASK,	PWRCOM,		{ RA, RS, UI } },

{ "andis.",  OP(29),	OP_MASK,	PPCCOM,		{ RA, RS, UI } },
{ "andiu.",  OP(29),	OP_MASK,	PWRCOM,		{ RA, RS, UI } },

{ "rotldi",  MD(30,0,0), MDMB_MASK,	PPC64,		{ RA, RS, SH6 } },
{ "clrldi",  MD(30,0,0), MDSH_MASK,	PPC64,		{ RA, RS, MB6 } },
{ "rldicl",  MD(30,0,0), MD_MASK,	PPC64,		{ RA, RS, SH6, MB6 } },
{ "rotldi.", MD(30,0,1), MDMB_MASK,	PPC64,		{ RA, RS, SH6 } },
{ "clrldi.", MD(30,0,1), MDSH_MASK,	PPC64,		{ RA, RS, MB6 } },
{ "rldicl.", MD(30,0,1), MD_MASK,	PPC64,		{ RA, RS, SH6, MB6 } },

{ "rldicr",  MD(30,1,0), MD_MASK,	PPC64,		{ RA, RS, SH6, ME6 } },
{ "rldicr.", MD(30,1,1), MD_MASK,	PPC64,		{ RA, RS, SH6, ME6 } },

{ "rldic",   MD(30,2,0), MD_MASK,	PPC64,		{ RA, RS, SH6, MB6 } },
{ "rldic.",  MD(30,2,1), MD_MASK,	PPC64,		{ RA, RS, SH6, MB6 } },

{ "rldimi",  MD(30,3,0), MD_MASK,	PPC64,		{ RA, RS, SH6, MB6 } },
{ "rldimi.", MD(30,3,1), MD_MASK,	PPC64,		{ RA, RS, SH6, MB6 } },

{ "rotld",   MDS(30,8,0), MDSMB_MASK,	PPC64,		{ RA, RS, RB } },
{ "rldcl",   MDS(30,8,0), MDS_MASK,	PPC64,		{ RA, RS, RB, MB6 } },
{ "rotld.",  MDS(30,8,1), MDSMB_MASK,	PPC64,		{ RA, RS, RB } },
{ "rldcl.",  MDS(30,8,1), MDS_MASK,	PPC64,		{ RA, RS, RB, MB6 } },

{ "rldcr",   MDS(30,9,0), MDS_MASK,	PPC64,		{ RA, RS, RB, ME6 } },
{ "rldcr.",  MDS(30,9,1), MDS_MASK,	PPC64,		{ RA, RS, RB, ME6 } },

{ "cmpw",    XCMPL(31,0,0), XCMPL_MASK, PPCCOM,		{ OBF, RA, RB } },
{ "cmpd",    XCMPL(31,0,1), XCMPL_MASK, PPC64,		{ OBF, RA, RB } },
{ "cmp",     X(31,0),	XCMP_MASK,	PPCONLY,	{ BF, L, RA, RB } },
{ "cmp",     X(31,0),	XCMPL_MASK,	PWRCOM,		{ BF, RA, RB } },

{ "twlgt",   XTO(31,4,TOLGT), XTO_MASK, PPCCOM,		{ RA, RB } },
{ "tlgt",    XTO(31,4,TOLGT), XTO_MASK, PWRCOM,		{ RA, RB } },
{ "twllt",   XTO(31,4,TOLLT), XTO_MASK, PPCCOM,		{ RA, RB } },
{ "tllt",    XTO(31,4,TOLLT), XTO_MASK, PWRCOM,		{ RA, RB } },
{ "tweq",    XTO(31,4,TOEQ), XTO_MASK,	PPCCOM,		{ RA, RB } },
{ "teq",     XTO(31,4,TOEQ), XTO_MASK,	PWRCOM,		{ RA, RB } },
{ "twlge",   XTO(31,4,TOLGE), XTO_MASK, PPCCOM,		{ RA, RB } },
{ "tlge",    XTO(31,4,TOLGE), XTO_MASK, PWRCOM,		{ RA, RB } },
{ "twlnl",   XTO(31,4,TOLNL), XTO_MASK, PPCCOM,		{ RA, RB } },
{ "tlnl",    XTO(31,4,TOLNL), XTO_MASK, PWRCOM,		{ RA, RB } },
{ "twlle",   XTO(31,4,TOLLE), XTO_MASK, PPCCOM,		{ RA, RB } },
{ "tlle",    XTO(31,4,TOLLE), XTO_MASK, PWRCOM,		{ RA, RB } },
{ "twlng",   XTO(31,4,TOLNG), XTO_MASK, PPCCOM,		{ RA, RB } },
{ "tlng",    XTO(31,4,TOLNG), XTO_MASK, PWRCOM,		{ RA, RB } },
{ "twgt",    XTO(31,4,TOGT), XTO_MASK,	PPCCOM,		{ RA, RB } },
{ "tgt",     XTO(31,4,TOGT), XTO_MASK,	PWRCOM,		{ RA, RB } },
{ "twge",    XTO(31,4,TOGE), XTO_MASK,	PPCCOM,		{ RA, RB } },
{ "tge",     XTO(31,4,TOGE), XTO_MASK,	PWRCOM,		{ RA, RB } },
{ "twnl",    XTO(31,4,TONL), XTO_MASK,	PPCCOM,		{ RA, RB } },
{ "tnl",     XTO(31,4,TONL), XTO_MASK,	PWRCOM,		{ RA, RB } },
{ "twlt",    XTO(31,4,TOLT), XTO_MASK,	PPCCOM,		{ RA, RB } },
{ "tlt",     XTO(31,4,TOLT), XTO_MASK,	PWRCOM,		{ RA, RB } },
{ "twle",    XTO(31,4,TOLE), XTO_MASK,	PPCCOM,		{ RA, RB } },
{ "tle",     XTO(31,4,TOLE), XTO_MASK,	PWRCOM,		{ RA, RB } },
{ "twng",    XTO(31,4,TONG), XTO_MASK,	PPCCOM,		{ RA, RB } },
{ "tng",     XTO(31,4,TONG), XTO_MASK,	PWRCOM,		{ RA, RB } },
{ "twne",    XTO(31,4,TONE), XTO_MASK,	PPCCOM,		{ RA, RB } },
{ "tne",     XTO(31,4,TONE), XTO_MASK,	PWRCOM,		{ RA, RB } },
{ "trap",    XTO(31,4,TOU), 0xffffffff,	PPCCOM,		{ 0 } },
{ "tw",      X(31,4),	X_MASK,		PPCCOM,		{ TO, RA, RB } },
{ "t",       X(31,4),	X_MASK,		PWRCOM,		{ TO, RA, RB } },

{ "subfc",   XO(31,8,0,0), XO_MASK,	PPCCOM,		{ RT, RA, RB } },
{ "sf",      XO(31,8,0,0), XO_MASK,	PWRCOM,		{ RT, RA, RB } },
{ "subc",    XO(31,8,0,0), XO_MASK,	PPC,		{ RT, RB, RA } },
{ "subfc.",  XO(31,8,0,1), XO_MASK,	PPCCOM,		{ RT, RA, RB } },
{ "sf.",     XO(31,8,0,1), XO_MASK,	PWRCOM,		{ RT, RA, RB } },
{ "subc.",   XO(31,8,0,1), XO_MASK,	PPCCOM,		{ RT, RB, RA } },
{ "subfco",  XO(31,8,1,0), XO_MASK,	PPCCOM,		{ RT, RA, RB } },
{ "sfo",     XO(31,8,1,0), XO_MASK,	PWRCOM,		{ RT, RA, RB } },
{ "subco",   XO(31,8,1,0), XO_MASK,	PPC,		{ RT, RB, RA } },
{ "subfco.", XO(31,8,1,1), XO_MASK,	PPCCOM,		{ RT, RA, RB } },
{ "sfo.",    XO(31,8,1,1), XO_MASK,	PWRCOM,		{ RT, RA, RB } },
{ "subco.",  XO(31,8,1,1), XO_MASK,	PPC,		{ RT, RB, RA } },

{ "mulhdu",  XO(31,9,0,0), XO_MASK,	PPC64,		{ RT, RA, RB } },
{ "mulhdu.", XO(31,9,0,1), XO_MASK,	PPC64,		{ RT, RA, RB } },

{ "addc",    XO(31,10,0,0), XO_MASK,	PPCCOM,		{ RT, RA, RB } },
{ "a",       XO(31,10,0,0), XO_MASK,	PWRCOM,		{ RT, RA, RB } },
{ "addc.",   XO(31,10,0,1), XO_MASK,	PPCCOM,		{ RT, RA, RB } },
{ "a.",      XO(31,10,0,1), XO_MASK,	PWRCOM,		{ RT, RA, RB } },
{ "addco",   XO(31,10,1,0), XO_MASK,	PPCCOM,		{ RT, RA, RB } },
{ "ao",      XO(31,10,1,0), XO_MASK,	PWRCOM,		{ RT, RA, RB } },
{ "addco.",  XO(31,10,1,1), XO_MASK,	PPCCOM,		{ RT, RA, RB } },
{ "ao.",     XO(31,10,1,1), XO_MASK,	PWRCOM,		{ RT, RA, RB } },

{ "mulhwu",  XO(31,11,0,0), XO_MASK,	PPC,		{ RT, RA, RB } },
{ "mulhwu.", XO(31,11,0,1), XO_MASK,	PPC,		{ RT, RA, RB } },

{ "isellt",  X(31,15),      X_MASK,	PPCISEL,	{ RT, RA, RB } },
{ "iselgt",  X(31,47),      X_MASK,	PPCISEL,	{ RT, RA, RB } },
{ "iseleq",  X(31,79),      X_MASK,	PPCISEL,	{ RT, RA, RB } },
{ "isel",    XISEL(31,15),  XISEL_MASK,	PPCISEL,	{ RT, RA, RB, CRB } },

{ "mfcr",    X(31,19),	XRARB_MASK,	COM,		{ RT } },

{ "lwarx",   X(31,20),	X_MASK,		PPC,		{ RT, RA, RB } },

{ "ldx",     X(31,21),	X_MASK,		PPC64,		{ RT, RA, RB } },

{ "icbt",    X(31,22),	X_MASK,		BOOKE,		{ CT, RA, RB } },

{ "lwzx",    X(31,23),	X_MASK,		PPCCOM,		{ RT, RA, RB } },
{ "lx",      X(31,23),	X_MASK,		PWRCOM,		{ RT, RA, RB } },

{ "slw",     XRC(31,24,0), X_MASK,	PPCCOM,		{ RA, RS, RB } },
{ "sl",      XRC(31,24,0), X_MASK,	PWRCOM,		{ RA, RS, RB } },
{ "slw.",    XRC(31,24,1), X_MASK,	PPCCOM,		{ RA, RS, RB } },
{ "sl.",     XRC(31,24,1), X_MASK,	PWRCOM,		{ RA, RS, RB } },

{ "cntlzw",  XRC(31,26,0), XRB_MASK,	PPCCOM,		{ RA, RS } },
{ "cntlz",   XRC(31,26,0), XRB_MASK,	PWRCOM,		{ RA, RS } },
{ "cntlzw.", XRC(31,26,1), XRB_MASK,	PPCCOM,		{ RA, RS } },
{ "cntlz.",  XRC(31,26,1), XRB_MASK, 	PWRCOM,		{ RA, RS } },

{ "sld",     XRC(31,27,0), X_MASK,	PPC64,		{ RA, RS, RB } },
{ "sld.",    XRC(31,27,1), X_MASK,	PPC64,		{ RA, RS, RB } },

{ "and",     XRC(31,28,0), X_MASK,	COM,		{ RA, RS, RB } },
{ "and.",    XRC(31,28,1), X_MASK,	COM,		{ RA, RS, RB } },

{ "maskg",   XRC(31,29,0), X_MASK,	M601,		{ RA, RS, RB } },
{ "maskg.",  XRC(31,29,1), X_MASK,	M601,		{ RA, RS, RB } },

{ "icbte",   X(31,30),	X_MASK,		BOOKE64,	{ CT, RA, RB } },

{ "lwzxe",   X(31,31),	X_MASK,		BOOKE64,	{ RT, RA, RB } },

{ "cmplw",   XCMPL(31,32,0), XCMPL_MASK, PPCCOM,	{ OBF, RA, RB } },
{ "cmpld",   XCMPL(31,32,1), XCMPL_MASK, PPC64,		{ OBF, RA, RB } },
{ "cmpl",    X(31,32),	XCMP_MASK,	 PPCONLY,	{ BF, L, RA, RB } },
{ "cmpl",    X(31,32),	XCMPL_MASK,	 PWRCOM,	{ BF, RA, RB } },

{ "subf",    XO(31,40,0,0), XO_MASK,	PPC,		{ RT, RA, RB } },
{ "sub",     XO(31,40,0,0), XO_MASK,	PPC,		{ RT, RB, RA } },
{ "subf.",   XO(31,40,0,1), XO_MASK,	PPC,		{ RT, RA, RB } },
{ "sub.",    XO(31,40,0,1), XO_MASK,	PPC,		{ RT, RB, RA } },
{ "subfo",   XO(31,40,1,0), XO_MASK,	PPC,		{ RT, RA, RB } },
{ "subo",    XO(31,40,1,0), XO_MASK,	PPC,		{ RT, RB, RA } },
{ "subfo.",  XO(31,40,1,1), XO_MASK,	PPC,		{ RT, RA, RB } },
{ "subo.",   XO(31,40,1,1), XO_MASK,	PPC,		{ RT, RB, RA } },

{ "ldux",    X(31,53),	X_MASK,		PPC64,		{ RT, RAL, RB } },

{ "dcbst",   X(31,54),	XRT_MASK,	PPC,		{ RA, RB } },

{ "lwzux",   X(31,55),	X_MASK,		PPCCOM,		{ RT, RAL, RB } },
{ "lux",     X(31,55),	X_MASK,		PWRCOM,		{ RT, RA, RB } },

{ "dcbste",  X(31,62),	XRT_MASK,	BOOKE64,	{ RA, RB } },

{ "lwzuxe",  X(31,63),	X_MASK,		BOOKE64,	{ RT, RAL, RB } },

{ "cntlzd",  XRC(31,58,0), XRB_MASK,	PPC64,		{ RA, RS } },
{ "cntlzd.", XRC(31,58,1), XRB_MASK,	PPC64,		{ RA, RS } },

{ "andc",    XRC(31,60,0), X_MASK,	COM,		{ RA, RS, RB } },
{ "andc.",   XRC(31,60,1), X_MASK,	COM,		{ RA, RS, RB } },

{ "tdlgt",   XTO(31,68,TOLGT), XTO_MASK, PPC64,		{ RA, RB } },
{ "tdllt",   XTO(31,68,TOLLT), XTO_MASK, PPC64,		{ RA, RB } },
{ "tdeq",    XTO(31,68,TOEQ), XTO_MASK,  PPC64,		{ RA, RB } },
{ "tdlge",   XTO(31,68,TOLGE), XTO_MASK, PPC64,		{ RA, RB } },
{ "tdlnl",   XTO(31,68,TOLNL), XTO_MASK, PPC64,		{ RA, RB } },
{ "tdlle",   XTO(31,68,TOLLE), XTO_MASK, PPC64,		{ RA, RB } },
{ "tdlng",   XTO(31,68,TOLNG), XTO_MASK, PPC64,		{ RA, RB } },
{ "tdgt",    XTO(31,68,TOGT), XTO_MASK,  PPC64,		{ RA, RB } },
{ "tdge",    XTO(31,68,TOGE), XTO_MASK,  PPC64,		{ RA, RB } },
{ "tdnl",    XTO(31,68,TONL), XTO_MASK,  PPC64,		{ RA, RB } },
{ "tdlt",    XTO(31,68,TOLT), XTO_MASK,  PPC64,		{ RA, RB } },
{ "tdle",    XTO(31,68,TOLE), XTO_MASK,  PPC64,		{ RA, RB } },
{ "tdng",    XTO(31,68,TONG), XTO_MASK,  PPC64,		{ RA, RB } },
{ "tdne",    XTO(31,68,TONE), XTO_MASK,  PPC64,		{ RA, RB } },
{ "td",	     X(31,68),	X_MASK,		 PPC64,		{ TO, RA, RB } },

{ "mulhd",   XO(31,73,0,0), XO_MASK,	 PPC64,		{ RT, RA, RB } },
{ "mulhd.",  XO(31,73,0,1), XO_MASK,	 PPC64,		{ RT, RA, RB } },

{ "mulhw",   XO(31,75,0,0), XO_MASK,	PPC,		{ RT, RA, RB } },
{ "mulhw.",  XO(31,75,0,1), XO_MASK,	PPC,		{ RT, RA, RB } },

{ "mtsrd",   X(31,82),	XRB_MASK|(1<<20), PPC64,	{ SR, RS } },

{ "mfmsr",   X(31,83),	XRARB_MASK,	COM,		{ RT } },

{ "ldarx",   X(31,84),	X_MASK,		PPC64,		{ RT, RA, RB } },

{ "dcbf",    X(31,86),	XRT_MASK,	PPC,		{ RA, RB } },

{ "lbzx",    X(31,87),	X_MASK,		COM,		{ RT, RA, RB } },

{ "dcbfe",   X(31,94),	XRT_MASK,	BOOKE64,	{ RA, RB } },

{ "lbzxe",   X(31,95),	X_MASK,		BOOKE64,	{ RT, RA, RB } },

{ "neg",     XO(31,104,0,0), XORB_MASK,	COM,		{ RT, RA } },
{ "neg.",    XO(31,104,0,1), XORB_MASK,	COM,		{ RT, RA } },
{ "nego",    XO(31,104,1,0), XORB_MASK,	COM,		{ RT, RA } },
{ "nego.",   XO(31,104,1,1), XORB_MASK,	COM,		{ RT, RA } },

{ "mul",     XO(31,107,0,0), XO_MASK,	M601,		{ RT, RA, RB } },
{ "mul.",    XO(31,107,0,1), XO_MASK,	M601,		{ RT, RA, RB } },
{ "mulo",    XO(31,107,1,0), XO_MASK,	M601,		{ RT, RA, RB } },
{ "mulo.",   XO(31,107,1,1), XO_MASK,	M601,		{ RT, RA, RB } },

{ "mtsrdin", X(31,114),	XRA_MASK,	PPC64,		{ RS, RB } },

{ "clf",     X(31,118), XTO_MASK,	POWER,		{ RA, RB } },

{ "lbzux",   X(31,119),	X_MASK,		COM,		{ RT, RAL, RB } },

{ "not",     XRC(31,124,0), X_MASK,	COM,		{ RA, RS, RBS } },
{ "nor",     XRC(31,124,0), X_MASK,	COM,		{ RA, RS, RB } },
{ "not.",    XRC(31,124,1), X_MASK,	COM,		{ RA, RS, RBS } },
{ "nor.",    XRC(31,124,1), X_MASK,	COM,		{ RA, RS, RB } },

{ "lwarxe",  X(31,126),	X_MASK,		BOOKE64,	{ RT, RA, RB } },

{ "lbzuxe",  X(31,127),	X_MASK,		BOOKE64,	{ RT, RAL, RB } },

{ "wrtee",   X(31,131),	XRARB_MASK,	PPC403,		{ RS } },
{ "wrtee",   X(31,131),	XRARB_MASK,	BOOKE,		{ RS } },

{ "dcbtstls",X(31,134),	X_MASK,		PPCCHLK,	{ CT, RA, RB }},

{ "subfe",   XO(31,136,0,0), XO_MASK,	PPCCOM,		{ RT, RA, RB } },
{ "sfe",     XO(31,136,0,0), XO_MASK,	PWRCOM,		{ RT, RA, RB } },
{ "subfe.",  XO(31,136,0,1), XO_MASK,	PPCCOM,		{ RT, RA, RB } },
{ "sfe.",    XO(31,136,0,1), XO_MASK,	PWRCOM,		{ RT, RA, RB } },
{ "subfeo",  XO(31,136,1,0), XO_MASK,	PPCCOM,		{ RT, RA, RB } },
{ "sfeo",    XO(31,136,1,0), XO_MASK,	PWRCOM,		{ RT, RA, RB } },
{ "subfeo.", XO(31,136,1,1), XO_MASK,	PPCCOM,		{ RT, RA, RB } },
{ "sfeo.",   XO(31,136,1,1), XO_MASK,	PWRCOM,		{ RT, RA, RB } },

{ "adde",    XO(31,138,0,0), XO_MASK,	PPCCOM,		{ RT, RA, RB } },
{ "ae",      XO(31,138,0,0), XO_MASK,	PWRCOM,		{ RT, RA, RB } },
{ "adde.",   XO(31,138,0,1), XO_MASK,	PPCCOM,		{ RT, RA, RB } },
{ "ae.",     XO(31,138,0,1), XO_MASK,	PWRCOM,		{ RT, RA, RB } },
{ "addeo",   XO(31,138,1,0), XO_MASK,	PPCCOM,		{ RT, RA, RB } },
{ "aeo",     XO(31,138,1,0), XO_MASK,	PWRCOM,		{ RT, RA, RB } },
{ "addeo.",  XO(31,138,1,1), XO_MASK,	PPCCOM,		{ RT, RA, RB } },
{ "aeo.",    XO(31,138,1,1), XO_MASK,	PWRCOM,		{ RT, RA, RB } },

{ "dcbtstlse",X(31,142),X_MASK,		PPCCHLK64,	{ CT, RA, RB }},

{ "mtcr",    XFXM(31,144,0xff), XFXFXM_MASK|FXM_MASK, COM,	{ RS }},
{ "mtcrf",   X(31,144),	XFXFXM_MASK,	COM,		{ FXM, RS } },

{ "mtmsr",   X(31,146),	XRARB_MASK,	COM,		{ RS } },

{ "stdx",    X(31,149), X_MASK,		PPC64,		{ RS, RA, RB } },

{ "stwcx.",  XRC(31,150,1), X_MASK,	PPC,		{ RS, RA, RB } },

{ "stwx",    X(31,151), X_MASK,		PPCCOM,		{ RS, RA, RB } },
{ "stx",     X(31,151), X_MASK,		PWRCOM,		{ RS, RA, RB } },

{ "stwcxe.", XRC(31,158,1), X_MASK,	BOOKE64,	{ RS, RA, RB } },

{ "stwxe",   X(31,159), X_MASK,		BOOKE64,	{ RS, RA, RB } },

{ "slq",     XRC(31,152,0), X_MASK,	M601,		{ RA, RS, RB } },
{ "slq.",    XRC(31,152,1), X_MASK,	M601,		{ RA, RS, RB } },

{ "sle",     XRC(31,153,0), X_MASK,	M601,		{ RA, RS, RB } },
{ "sle.",    XRC(31,153,1), X_MASK,	M601,		{ RA, RS, RB } },

{ "wrteei",  X(31,163),	XE_MASK,	PPC403,		{ E } },
{ "wrteei",  X(31,163),	XE_MASK,	BOOKE,		{ E } },

{ "dcbtls",  X(31,166),	X_MASK,		PPCCHLK,	{ CT, RA, RB }},
{ "dcbtlse", X(31,174),	X_MASK,		PPCCHLK64,	{ CT, RA, RB }},

{ "mtmsrd",  X(31,178),	XRLARB_MASK,	PPC64,		{ RS, MTMSRD_L } },

{ "stdux",   X(31,181),	X_MASK,		PPC64,		{ RS, RAS, RB } },

{ "stwux",   X(31,183),	X_MASK,		PPCCOM,		{ RS, RAS, RB } },
{ "stux",    X(31,183),	X_MASK,		PWRCOM,		{ RS, RA, RB } },

{ "sliq",    XRC(31,184,0), X_MASK,	M601,		{ RA, RS, SH } },
{ "sliq.",   XRC(31,184,1), X_MASK,	M601,		{ RA, RS, SH } },

{ "stwuxe",  X(31,191),	X_MASK,		BOOKE64,	{ RS, RAS, RB } },

{ "subfze",  XO(31,200,0,0), XORB_MASK, PPCCOM,		{ RT, RA } },
{ "sfze",    XO(31,200,0,0), XORB_MASK, PWRCOM,		{ RT, RA } },
{ "subfze.", XO(31,200,0,1), XORB_MASK, PPCCOM,		{ RT, RA } },
{ "sfze.",   XO(31,200,0,1), XORB_MASK, PWRCOM,		{ RT, RA } },
{ "subfzeo", XO(31,200,1,0), XORB_MASK, PPCCOM,		{ RT, RA } },
{ "sfzeo",   XO(31,200,1,0), XORB_MASK, PWRCOM,		{ RT, RA } },
{ "subfzeo.",XO(31,200,1,1), XORB_MASK, PPCCOM,		{ RT, RA } },
{ "sfzeo.",  XO(31,200,1,1), XORB_MASK, PWRCOM,		{ RT, RA } },

{ "addze",   XO(31,202,0,0), XORB_MASK, PPCCOM,		{ RT, RA } },
{ "aze",     XO(31,202,0,0), XORB_MASK, PWRCOM,		{ RT, RA } },
{ "addze.",  XO(31,202,0,1), XORB_MASK, PPCCOM,		{ RT, RA } },
{ "aze.",    XO(31,202,0,1), XORB_MASK, PWRCOM,		{ RT, RA } },
{ "addzeo",  XO(31,202,1,0), XORB_MASK, PPCCOM,		{ RT, RA } },
{ "azeo",    XO(31,202,1,0), XORB_MASK, PWRCOM,		{ RT, RA } },
{ "addzeo.", XO(31,202,1,1), XORB_MASK, PPCCOM,		{ RT, RA } },
{ "azeo.",   XO(31,202,1,1), XORB_MASK, PWRCOM,		{ RT, RA } },

{ "mtsr",    X(31,210),	XRB_MASK|(1<<20), COM32,	{ SR, RS } },

{ "stdcx.",  XRC(31,214,1), X_MASK,	PPC64,		{ RS, RA, RB } },

{ "stbx",    X(31,215),	X_MASK,		COM,		{ RS, RA, RB } },

{ "sllq",    XRC(31,216,0), X_MASK,	M601,		{ RA, RS, RB } },
{ "sllq.",   XRC(31,216,1), X_MASK,	M601,		{ RA, RS, RB } },

{ "sleq",    XRC(31,217,0), X_MASK,	M601,		{ RA, RS, RB } },
{ "sleq.",   XRC(31,217,1), X_MASK,	M601,		{ RA, RS, RB } },

{ "stbxe",   X(31,223),	X_MASK,		BOOKE64,	{ RS, RA, RB } },

{ "icblc",   X(31,230),	X_MASK,		PPCCHLK,	{ CT, RA, RB }},

{ "subfme",  XO(31,232,0,0), XORB_MASK, PPCCOM,		{ RT, RA } },
{ "sfme",    XO(31,232,0,0), XORB_MASK, PWRCOM,		{ RT, RA } },
{ "subfme.", XO(31,232,0,1), XORB_MASK, PPCCOM,		{ RT, RA } },
{ "sfme.",   XO(31,232,0,1), XORB_MASK, PWRCOM,		{ RT, RA } },
{ "subfmeo", XO(31,232,1,0), XORB_MASK, PPCCOM,		{ RT, RA } },
{ "sfmeo",   XO(31,232,1,0), XORB_MASK, PWRCOM,		{ RT, RA } },
{ "subfmeo.",XO(31,232,1,1), XORB_MASK, PPCCOM,		{ RT, RA } },
{ "sfmeo.",  XO(31,232,1,1), XORB_MASK, PWRCOM,		{ RT, RA } },

{ "mulld",   XO(31,233,0,0), XO_MASK,	PPC64,		{ RT, RA, RB } },
{ "mulld.",  XO(31,233,0,1), XO_MASK,	PPC64,		{ RT, RA, RB } },
{ "mulldo",  XO(31,233,1,0), XO_MASK,	PPC64,		{ RT, RA, RB } },
{ "mulldo.", XO(31,233,1,1), XO_MASK,	PPC64,		{ RT, RA, RB } },

{ "addme",   XO(31,234,0,0), XORB_MASK, PPCCOM,		{ RT, RA } },
{ "ame",     XO(31,234,0,0), XORB_MASK, PWRCOM,		{ RT, RA } },
{ "addme.",  XO(31,234,0,1), XORB_MASK, PPCCOM,		{ RT, RA } },
{ "ame.",    XO(31,234,0,1), XORB_MASK, PWRCOM,		{ RT, RA } },
{ "addmeo",  XO(31,234,1,0), XORB_MASK, PPCCOM,		{ RT, RA } },
{ "ameo",    XO(31,234,1,0), XORB_MASK, PWRCOM,		{ RT, RA } },
{ "addmeo.", XO(31,234,1,1), XORB_MASK, PPCCOM,		{ RT, RA } },
{ "ameo.",   XO(31,234,1,1), XORB_MASK, PWRCOM,		{ RT, RA } },

{ "mullw",   XO(31,235,0,0), XO_MASK,	PPCCOM,		{ RT, RA, RB } },
{ "muls",    XO(31,235,0,0), XO_MASK,	PWRCOM,		{ RT, RA, RB } },
{ "mullw.",  XO(31,235,0,1), XO_MASK,	PPCCOM,		{ RT, RA, RB } },
{ "muls.",   XO(31,235,0,1), XO_MASK,	PWRCOM,		{ RT, RA, RB } },
{ "mullwo",  XO(31,235,1,0), XO_MASK,	PPCCOM,		{ RT, RA, RB } },
{ "mulso",   XO(31,235,1,0), XO_MASK,	PWRCOM,		{ RT, RA, RB } },
{ "mullwo.", XO(31,235,1,1), XO_MASK,	PPCCOM,		{ RT, RA, RB } },
{ "mulso.",  XO(31,235,1,1), XO_MASK,	PWRCOM,		{ RT, RA, RB } },

{ "icblce",  X(31,238),	X_MASK,		PPCCHLK64,	{ CT, RA, RB }},
{ "mtsrin",  X(31,242),	XRA_MASK,	PPC32,		{ RS, RB } },
{ "mtsri",   X(31,242),	XRA_MASK,	POWER32,	{ RS, RB } },

{ "dcbtst",  X(31,246),	XRT_MASK,	PPC,		{ CT, RA, RB } },

{ "stbux",   X(31,247),	X_MASK,		COM,		{ RS, RAS, RB } },

{ "slliq",   XRC(31,248,0), X_MASK,	M601,		{ RA, RS, SH } },
{ "slliq.",  XRC(31,248,1), X_MASK,	M601,		{ RA, RS, SH } },

{ "dcbtste", X(31,253),	X_MASK,		BOOKE64,	{ CT, RA, RB } },

{ "stbuxe",  X(31,255),	X_MASK,		BOOKE64,	{ RS, RAS, RB } },

{ "mfdcrx",  X(31,259),	X_MASK,		BOOKE,		{ RS, RA } },

{ "icbt",    X(31,262),	XRT_MASK,	PPC403,		{ RA, RB } },

{ "doz",     XO(31,264,0,0), XO_MASK,	M601,		{ RT, RA, RB } },
{ "doz.",    XO(31,264,0,1), XO_MASK,	M601,		{ RT, RA, RB } },
{ "dozo",    XO(31,264,1,0), XO_MASK,	M601,		{ RT, RA, RB } },
{ "dozo.",   XO(31,264,1,1), XO_MASK,	M601,		{ RT, RA, RB } },

{ "add",     XO(31,266,0,0), XO_MASK,	PPCCOM,		{ RT, RA, RB } },
{ "cax",     XO(31,266,0,0), XO_MASK,	PWRCOM,		{ RT, RA, RB } },
{ "add.",    XO(31,266,0,1), XO_MASK,	PPCCOM,		{ RT, RA, RB } },
{ "cax.",    XO(31,266,0,1), XO_MASK,	PWRCOM,		{ RT, RA, RB } },
{ "addo",    XO(31,266,1,0), XO_MASK,	PPCCOM,		{ RT, RA, RB } },
{ "caxo",    XO(31,266,1,0), XO_MASK,	PWRCOM,		{ RT, RA, RB } },
{ "addo.",   XO(31,266,1,1), XO_MASK,	PPCCOM,		{ RT, RA, RB } },
{ "caxo.",   XO(31,266,1,1), XO_MASK,	PWRCOM,		{ RT, RA, RB } },

{ "tlbiel",  X(31,274), XRTRA_MASK,	POWER4,		{ RB } },

{ "mfapidi", X(31,275), X_MASK,		BOOKE,		{ RT, RA } },

{ "lscbx",   XRC(31,277,0), X_MASK,	M601,		{ RT, RA, RB } },
{ "lscbx.",  XRC(31,277,1), X_MASK,	M601,		{ RT, RA, RB } },

{ "dcbt",    X(31,278),	XRT_MASK,	PPC,		{ CT, RA, RB } },

{ "lhzx",    X(31,279),	X_MASK,		COM,		{ RT, RA, RB } },

{ "eqv",     XRC(31,284,0), X_MASK,	COM,		{ RA, RS, RB } },
{ "eqv.",    XRC(31,284,1), X_MASK,	COM,		{ RA, RS, RB } },

{ "dcbte",   X(31,286),	X_MASK,		BOOKE64,	{ CT, RA, RB } },

{ "lhzxe",   X(31,287),	X_MASK,		BOOKE64,	{ RT, RA, RB } },

{ "tlbie",   X(31,306),	XRTLRA_MASK,	PPC,		{ RB, L } },
{ "tlbi",    X(31,306),	XRT_MASK,	POWER,		{ RA, RB } },

{ "eciwx",   X(31,310), X_MASK,		PPC,		{ RT, RA, RB } },

{ "lhzux",   X(31,311),	X_MASK,		COM,		{ RT, RAL, RB } },

{ "xor",     XRC(31,316,0), X_MASK,	COM,		{ RA, RS, RB } },
{ "xor.",    XRC(31,316,1), X_MASK,	COM,		{ RA, RS, RB } },

{ "lhzuxe",  X(31,319),	X_MASK,		BOOKE64,	{ RT, RAL, RB } },

{ "mfexisr",  XSPR(31,323,64),  XSPR_MASK, PPC403,	{ RT } },
{ "mfexier",  XSPR(31,323,66),  XSPR_MASK, PPC403,	{ RT } },
{ "mfbr0",    XSPR(31,323,128), XSPR_MASK, PPC403,	{ RT } },
{ "mfbr1",    XSPR(31,323,129), XSPR_MASK, PPC403,	{ RT } },
{ "mfbr2",    XSPR(31,323,130), XSPR_MASK, PPC403,	{ RT } },
{ "mfbr3",    XSPR(31,323,131), XSPR_MASK, PPC403,	{ RT } },
{ "mfbr4",    XSPR(31,323,132), XSPR_MASK, PPC403,	{ RT } },
{ "mfbr5",    XSPR(31,323,133), XSPR_MASK, PPC403,	{ RT } },
{ "mfbr6",    XSPR(31,323,134), XSPR_MASK, PPC403,	{ RT } },
{ "mfbr7",    XSPR(31,323,135), XSPR_MASK, PPC403,	{ RT } },
{ "mfbear",   XSPR(31,323,144), XSPR_MASK, PPC403,	{ RT } },
{ "mfbesr",   XSPR(31,323,145), XSPR_MASK, PPC403,	{ RT } },
{ "mfiocr",   XSPR(31,323,160), XSPR_MASK, PPC403,	{ RT } },
{ "mfdmacr0", XSPR(31,323,192), XSPR_MASK, PPC403,	{ RT } },
{ "mfdmact0", XSPR(31,323,193), XSPR_MASK, PPC403,	{ RT } },
{ "mfdmada0", XSPR(31,323,194), XSPR_MASK, PPC403,	{ RT } },
{ "mfdmasa0", XSPR(31,323,195), XSPR_MASK, PPC403,	{ RT } },
{ "mfdmacc0", XSPR(31,323,196), XSPR_MASK, PPC403,	{ RT } },
{ "mfdmacr1", XSPR(31,323,200), XSPR_MASK, PPC403,	{ RT } },
{ "mfdmact1", XSPR(31,323,201), XSPR_MASK, PPC403,	{ RT } },
{ "mfdmada1", XSPR(31,323,202), XSPR_MASK, PPC403,	{ RT } },
{ "mfdmasa1", XSPR(31,323,203), XSPR_MASK, PPC403,	{ RT } },
{ "mfdmacc1", XSPR(31,323,204), XSPR_MASK, PPC403,	{ RT } },
{ "mfdmacr2", XSPR(31,323,208), XSPR_MASK, PPC403,	{ RT } },
{ "mfdmact2", XSPR(31,323,209), XSPR_MASK, PPC403,	{ RT } },
{ "mfdmada2", XSPR(31,323,210), XSPR_MASK, PPC403,	{ RT } },
{ "mfdmasa2", XSPR(31,323,211), XSPR_MASK, PPC403,	{ RT } },
{ "mfdmacc2", XSPR(31,323,212), XSPR_MASK, PPC403,	{ RT } },
{ "mfdmacr3", XSPR(31,323,216), XSPR_MASK, PPC403,	{ RT } },
{ "mfdmact3", XSPR(31,323,217), XSPR_MASK, PPC403,	{ RT } },
{ "mfdmada3", XSPR(31,323,218), XSPR_MASK, PPC403,	{ RT } },
{ "mfdmasa3", XSPR(31,323,219), XSPR_MASK, PPC403,	{ RT } },
{ "mfdmacc3", XSPR(31,323,220), XSPR_MASK, PPC403,	{ RT } },
{ "mfdmasr",  XSPR(31,323,224), XSPR_MASK, PPC403,	{ RT } },
{ "mfdcr",    X(31,323),	X_MASK,		PPC403,		{ RT, SPR } },
{ "mfdcr",    X(31,323),	X_MASK,		BOOKE,		{ RT, SPR } },

{ "div",     XO(31,331,0,0), XO_MASK,	M601,		{ RT, RA, RB } },
{ "div.",    XO(31,331,0,1), XO_MASK,	M601,		{ RT, RA, RB } },
{ "divo",    XO(31,331,1,0), XO_MASK,	M601,		{ RT, RA, RB } },
{ "divo.",   XO(31,331,1,1), XO_MASK,	M601,		{ RT, RA, RB } },

{ "mfpmr",   X(31,334),	X_MASK,		PPCPMR,		{ RT, PMR }},

{ "mfmq",       XSPR(31,339,0),    XSPR_MASK, M601,	{ RT } },
{ "mfxer",      XSPR(31,339,1),    XSPR_MASK, COM,	{ RT } },
{ "mfrtcu",     XSPR(31,339,4),    XSPR_MASK, COM,	{ RT } },
{ "mfrtcl",     XSPR(31,339,5),    XSPR_MASK, COM,	{ RT } },
{ "mfdec",      XSPR(31,339,6),    XSPR_MASK, MFDEC1,	{ RT } },
{ "mflr",       XSPR(31,339,8),    XSPR_MASK, COM,	{ RT } },
{ "mfctr",      XSPR(31,339,9),    XSPR_MASK, COM,	{ RT } },
{ "mftid",      XSPR(31,339,17),   XSPR_MASK, POWER,	{ RT } },
{ "mfdsisr",    XSPR(31,339,18),   XSPR_MASK, COM,	{ RT } },
{ "mfdar",      XSPR(31,339,19),   XSPR_MASK, COM,	{ RT } },
{ "mfdec",      XSPR(31,339,22),   XSPR_MASK, MFDEC2,	{ RT } },
{ "mfsdr0",     XSPR(31,339,24),   XSPR_MASK, POWER,	{ RT } },
{ "mfsdr1",     XSPR(31,339,25),   XSPR_MASK, COM,	{ RT } },
{ "mfsrr0",     XSPR(31,339,26),   XSPR_MASK, COM,	{ RT } },
{ "mfsrr1",     XSPR(31,339,27),   XSPR_MASK, COM,	{ RT } },
{ "mfpid",      XSPR(31,339,48),   XSPR_MASK, BOOKE,    { RT } },
{ "mfcsrr0",    XSPR(31,339,58),   XSPR_MASK, BOOKE,    { RT } },
{ "mfcsrr1",    XSPR(31,339,59),   XSPR_MASK, BOOKE,    { RT } },
{ "mfdear",     XSPR(31,339,61),   XSPR_MASK, BOOKE,    { RT } },
{ "mfesr",      XSPR(31,339,62),   XSPR_MASK, BOOKE,    { RT } },
{ "mfivpr",     XSPR(31,339,63),   XSPR_MASK, BOOKE,    { RT } },
{ "mfcmpa",     XSPR(31,339,144),  XSPR_MASK, PPC860,	{ RT } },
{ "mfcmpb",     XSPR(31,339,145),  XSPR_MASK, PPC860,	{ RT } },
{ "mfcmpc",     XSPR(31,339,146),  XSPR_MASK, PPC860,	{ RT } },
{ "mfcmpd",     XSPR(31,339,147),  XSPR_MASK, PPC860,	{ RT } },
{ "mficr",      XSPR(31,339,148),  XSPR_MASK, PPC860,	{ RT } },
{ "mfder",      XSPR(31,339,149),  XSPR_MASK, PPC860,	{ RT } },
{ "mfcounta",   XSPR(31,339,150),  XSPR_MASK, PPC860,	{ RT } },
{ "mfcountb",   XSPR(31,339,151),  XSPR_MASK, PPC860,	{ RT } },
{ "mfcmpe",     XSPR(31,339,152),  XSPR_MASK, PPC860,	{ RT } },
{ "mfcmpf",     XSPR(31,339,153),  XSPR_MASK, PPC860,	{ RT } },
{ "mfcmpg",     XSPR(31,339,154),  XSPR_MASK, PPC860,	{ RT } },
{ "mfcmph",     XSPR(31,339,155),  XSPR_MASK, PPC860,	{ RT } },
{ "mflctrl1",   XSPR(31,339,156),  XSPR_MASK, PPC860,	{ RT } },
{ "mflctrl2",   XSPR(31,339,157),  XSPR_MASK, PPC860,	{ RT } },
{ "mfictrl",    XSPR(31,339,158),  XSPR_MASK, PPC860,	{ RT } },
{ "mfbar",      XSPR(31,339,159),  XSPR_MASK, PPC860,	{ RT } },
{ "mfvrsave",   XSPR(31,339,256),  XSPR_MASK, PPCVEC,	{ RT } },
{ "mfusprg0",   XSPR(31,339,256),  XSPR_MASK, BOOKE,    { RT } },
{ "mfsprg4",    XSPR(31,339,260),  XSPR_MASK, PPC405,	{ RT } },
{ "mfsprg5",    XSPR(31,339,261),  XSPR_MASK, PPC405,	{ RT } },
{ "mfsprg6",    XSPR(31,339,262),  XSPR_MASK, PPC405,	{ RT } },
{ "mfsprg7",    XSPR(31,339,263),  XSPR_MASK, PPC405,	{ RT } },
{ "mftb",       XSPR(31,339,268),  XSPR_MASK, BOOKE,    { RT } },
{ "mftbl",      XSPR(31,339,268),  XSPR_MASK, BOOKE,    { RT } },
{ "mftbu",      XSPR(31,339,269),  XSPR_MASK, BOOKE,    { RT } },
{ "mfsprg",     XSPR(31,339,272),  XSPRG_MASK, PPC,	{ RT, SPRG } },
{ "mfsprg0",    XSPR(31,339,272),  XSPR_MASK, PPC,	{ RT } },
{ "mfsprg1",    XSPR(31,339,273),  XSPR_MASK, PPC,	{ RT } },
{ "mfsprg2",    XSPR(31,339,274),  XSPR_MASK, PPC,	{ RT } },
{ "mfsprg3",    XSPR(31,339,275),  XSPR_MASK, PPC,	{ RT } },
{ "mfasr",      XSPR(31,339,280),  XSPR_MASK, PPC64,	{ RT } },
{ "mfear",      XSPR(31,339,282),  XSPR_MASK, PPC,	{ RT } },
{ "mfpir",      XSPR(31,339,286),  XSPR_MASK, BOOKE,    { RT } },
{ "mfpvr",      XSPR(31,339,287),  XSPR_MASK, PPC,	{ RT } },
{ "mfdbsr",     XSPR(31,339,304),  XSPR_MASK, BOOKE,    { RT } },
{ "mfdbcr0",    XSPR(31,339,308),  XSPR_MASK, BOOKE,    { RT } },
{ "mfdbcr1",    XSPR(31,339,309),  XSPR_MASK, BOOKE,    { RT } },
{ "mfdbcr2",    XSPR(31,339,310),  XSPR_MASK, BOOKE,    { RT } },
{ "mfiac1",     XSPR(31,339,312),  XSPR_MASK, BOOKE,    { RT } },
{ "mfiac2",     XSPR(31,339,313),  XSPR_MASK, BOOKE,    { RT } },
{ "mfiac3",     XSPR(31,339,314),  XSPR_MASK, BOOKE,    { RT } },
{ "mfiac4",     XSPR(31,339,315),  XSPR_MASK, BOOKE,    { RT } },
{ "mfdac1",     XSPR(31,339,316),  XSPR_MASK, BOOKE,    { RT } },
{ "mfdac2",     XSPR(31,339,317),  XSPR_MASK, BOOKE,    { RT } },
{ "mfdvc1",     XSPR(31,339,318),  XSPR_MASK, BOOKE,    { RT } },
{ "mfdvc2",     XSPR(31,339,319),  XSPR_MASK, BOOKE,    { RT } },
{ "mftsr",      XSPR(31,339,336),  XSPR_MASK, BOOKE,    { RT } },
{ "mftcr",      XSPR(31,339,340),  XSPR_MASK, BOOKE,    { RT } },
{ "mfivor0",    XSPR(31,339,400),  XSPR_MASK, BOOKE,    { RT } },
{ "mfivor1",    XSPR(31,339,401),  XSPR_MASK, BOOKE,    { RT } },
{ "mfivor2",    XSPR(31,339,402),  XSPR_MASK, BOOKE,    { RT } },
{ "mfivor3",    XSPR(31,339,403),  XSPR_MASK, BOOKE,    { RT } },
{ "mfivor4",    XSPR(31,339,404),  XSPR_MASK, BOOKE,    { RT } },
{ "mfivor5",    XSPR(31,339,405),  XSPR_MASK, BOOKE,    { RT } },
{ "mfivor6",    XSPR(31,339,406),  XSPR_MASK, BOOKE,    { RT } },
{ "mfivor7",    XSPR(31,339,407),  XSPR_MASK, BOOKE,    { RT } },
{ "mfivor8",    XSPR(31,339,408),  XSPR_MASK, BOOKE,    { RT } },
{ "mfivor9",    XSPR(31,339,409),  XSPR_MASK, BOOKE,    { RT } },
{ "mfivor10",   XSPR(31,339,410),  XSPR_MASK, BOOKE,    { RT } },
{ "mfivor11",   XSPR(31,339,411),  XSPR_MASK, BOOKE,    { RT } },
{ "mfivor12",   XSPR(31,339,412),  XSPR_MASK, BOOKE,    { RT } },
{ "mfivor13",   XSPR(31,339,413),  XSPR_MASK, BOOKE,    { RT } },
{ "mfivor14",   XSPR(31,339,414),  XSPR_MASK, BOOKE,    { RT } },
{ "mfivor15",   XSPR(31,339,415),  XSPR_MASK, BOOKE,    { RT } },
{ "mfspefscr",  XSPR(31,339,512),  XSPR_MASK, PPCSPE,	{ RT } },
{ "mfbbear",    XSPR(31,339,513),  XSPR_MASK, PPCBRLK,  { RT } },
{ "mfbbtar",    XSPR(31,339,514),  XSPR_MASK, PPCBRLK,  { RT } },
{ "mfibatu",    XSPR(31,339,528),  XSPRBAT_MASK, PPC,	{ RT, SPRBAT } },
{ "mfibatl",    XSPR(31,339,529),  XSPRBAT_MASK, PPC,	{ RT, SPRBAT } },
{ "mfdbatu",    XSPR(31,339,536),  XSPRBAT_MASK, PPC,	{ RT, SPRBAT } },
{ "mfdbatl",    XSPR(31,339,537),  XSPRBAT_MASK, PPC,	{ RT, SPRBAT } },
{ "mfic_cst",   XSPR(31,339,560),  XSPR_MASK, PPC860,	{ RT } },
{ "mfic_adr",   XSPR(31,339,561),  XSPR_MASK, PPC860,	{ RT } },
{ "mfic_dat",   XSPR(31,339,562),  XSPR_MASK, PPC860,	{ RT } },
{ "mfdc_cst",   XSPR(31,339,568),  XSPR_MASK, PPC860,	{ RT } },
{ "mfdc_adr",   XSPR(31,339,569),  XSPR_MASK, PPC860,	{ RT } },
{ "mfdc_dat",   XSPR(31,339,570),  XSPR_MASK, PPC860,	{ RT } },
{ "mfmcsrr0",   XSPR(31,339,570),  XSPR_MASK, PPCRFMCI, { RT } },
{ "mfmcsrr1",   XSPR(31,339,571),  XSPR_MASK, PPCRFMCI, { RT } },
{ "mfmcsr",     XSPR(31,339,572),  XSPR_MASK, PPCRFMCI, { RT } },
{ "mfdpdr",     XSPR(31,339,630),  XSPR_MASK, PPC860,	{ RT } },
{ "mfdpir",     XSPR(31,339,631),  XSPR_MASK, PPC860,	{ RT } },
{ "mfimmr",     XSPR(31,339,638),  XSPR_MASK, PPC860,	{ RT } },
{ "mfmi_ctr",   XSPR(31,339,784),  XSPR_MASK, PPC860,	{ RT } },
{ "mfmi_ap",    XSPR(31,339,786),  XSPR_MASK, PPC860,	{ RT } },
{ "mfmi_epn",   XSPR(31,339,787),  XSPR_MASK, PPC860,	{ RT } },
{ "mfmi_twc",   XSPR(31,339,789),  XSPR_MASK, PPC860,	{ RT } },
{ "mfmi_rpn",   XSPR(31,339,790),  XSPR_MASK, PPC860,	{ RT } },
{ "mfmd_ctr",   XSPR(31,339,792),  XSPR_MASK, PPC860,	{ RT } },
{ "mfm_casid",  XSPR(31,339,793),  XSPR_MASK, PPC860,	{ RT } },
{ "mfmd_ap",    XSPR(31,339,794),  XSPR_MASK, PPC860,	{ RT } },
{ "mfmd_epn",   XSPR(31,339,795),  XSPR_MASK, PPC860,	{ RT } },
{ "mfmd_twb",   XSPR(31,339,796),  XSPR_MASK, PPC860,	{ RT } },
{ "mfmd_twc",   XSPR(31,339,797),  XSPR_MASK, PPC860,	{ RT } },
{ "mfmd_rpn",   XSPR(31,339,798),  XSPR_MASK, PPC860,	{ RT } },
{ "mfm_tw",     XSPR(31,339,799),  XSPR_MASK, PPC860,	{ RT } },
{ "mfmi_dbcam", XSPR(31,339,816),  XSPR_MASK, PPC860,	{ RT } },
{ "mfmi_dbram0",XSPR(31,339,817),  XSPR_MASK, PPC860,	{ RT } },
{ "mfmi_dbram1",XSPR(31,339,818),  XSPR_MASK, PPC860,	{ RT } },
{ "mfmd_dbcam", XSPR(31,339,824),  XSPR_MASK, PPC860,	{ RT } },
{ "mfmd_dbram0",XSPR(31,339,825),  XSPR_MASK, PPC860,	{ RT } },
{ "mfmd_dbram1",XSPR(31,339,826),  XSPR_MASK, PPC860,	{ RT } },
{ "mfummcr0",   XSPR(31,339,936),  XSPR_MASK, PPC750,   { RT } },
{ "mfupmc1",    XSPR(31,339,937),  XSPR_MASK, PPC750,   { RT } },
{ "mfupmc2",    XSPR(31,339,938),  XSPR_MASK, PPC750,   { RT } },
{ "mfusia",     XSPR(31,339,939),  XSPR_MASK, PPC750,   { RT } },
{ "mfummcr1",   XSPR(31,339,940),  XSPR_MASK, PPC750,   { RT } },
{ "mfupmc3",    XSPR(31,339,941),  XSPR_MASK, PPC750,   { RT } },
{ "mfupmc4",    XSPR(31,339,942),  XSPR_MASK, PPC750,   { RT } },
{ "mfzpr",   	XSPR(31,339,944),  XSPR_MASK, PPC403,	{ RT } },
{ "mfpid",   	XSPR(31,339,945),  XSPR_MASK, PPC403,	{ RT } },
{ "mfccr0",  	XSPR(31,339,947),  XSPR_MASK, PPC405,	{ RT } },
{ "mfiac3",     XSPR(31,339,948),  XSPR_MASK, PPC405,	{ RT } },
{ "mfiac4",     XSPR(31,339,949),  XSPR_MASK, PPC405,	{ RT } },
{ "mfdvc1",     XSPR(31,339,950),  XSPR_MASK, PPC405,	{ RT } },
{ "mfdvc2",     XSPR(31,339,951),  XSPR_MASK, PPC405,	{ RT } },
{ "mfmmcr0",	XSPR(31,339,952),  XSPR_MASK, PPC750,	{ RT } },
{ "mfpmc1",	XSPR(31,339,953),  XSPR_MASK, PPC750,	{ RT } },
{ "mfsgr",	XSPR(31,339,953),  XSPR_MASK, PPC403,	{ RT } },
{ "mfpmc2",	XSPR(31,339,954),  XSPR_MASK, PPC750,	{ RT } },
{ "mfdcwr", 	XSPR(31,339,954),  XSPR_MASK, PPC403,	{ RT } },
{ "mfsia",	XSPR(31,339,955),  XSPR_MASK, PPC750,	{ RT } },
{ "mfsler",	XSPR(31,339,955),  XSPR_MASK, PPC405,	{ RT } },
{ "mfmmcr1",	XSPR(31,339,956),  XSPR_MASK, PPC750,	{ RT } },
{ "mfsu0r",	XSPR(31,339,956),  XSPR_MASK, PPC405,	{ RT } },
{ "mfpmc3",	XSPR(31,339,957),  XSPR_MASK, PPC750,	{ RT } },
{ "mfdbcr1", 	XSPR(31,339,957),  XSPR_MASK, PPC405,	{ RT } },
{ "mfpmc4",	XSPR(31,339,958),  XSPR_MASK, PPC750,	{ RT } },
{ "mficdbdr",   XSPR(31,339,979),  XSPR_MASK, PPC403,   { RT } },
{ "mfesr",      XSPR(31,339,980),  XSPR_MASK, PPC403,	{ RT } },
{ "mfdear",     XSPR(31,339,981),  XSPR_MASK, PPC403,	{ RT } },
{ "mfevpr",     XSPR(31,339,982),  XSPR_MASK, PPC403,	{ RT } },
{ "mfcdbcr",    XSPR(31,339,983),  XSPR_MASK, PPC403,	{ RT } },
{ "mftsr",      XSPR(31,339,984),  XSPR_MASK, PPC403,	{ RT } },
{ "mftcr",      XSPR(31,339,986),  XSPR_MASK, PPC403,	{ RT } },
{ "mfpit",      XSPR(31,339,987),  XSPR_MASK, PPC403,	{ RT } },
{ "mftbhi",     XSPR(31,339,988),  XSPR_MASK, PPC403,	{ RT } },
{ "mftblo",     XSPR(31,339,989),  XSPR_MASK, PPC403,	{ RT } },
{ "mfsrr2",     XSPR(31,339,990),  XSPR_MASK, PPC403,	{ RT } },
{ "mfsrr3",     XSPR(31,339,991),  XSPR_MASK, PPC403,	{ RT } },
{ "mfdbsr",     XSPR(31,339,1008), XSPR_MASK, PPC403,	{ RT } },
{ "mfdbcr0",    XSPR(31,339,1010), XSPR_MASK, PPC405,	{ RT } },
{ "mfiac1",     XSPR(31,339,1012), XSPR_MASK, PPC403,	{ RT } },
{ "mfiac2",     XSPR(31,339,1013), XSPR_MASK, PPC403,	{ RT } },
{ "mfdac1",     XSPR(31,339,1014), XSPR_MASK, PPC403,	{ RT } },
{ "mfdac2",     XSPR(31,339,1015), XSPR_MASK, PPC403,	{ RT } },
{ "mfl2cr",     XSPR(31,339,1017), XSPR_MASK, PPC750,   { RT } },
{ "mfdccr",     XSPR(31,339,1018), XSPR_MASK, PPC403,	{ RT } },
{ "mficcr",     XSPR(31,339,1019), XSPR_MASK, PPC403,	{ RT } },
{ "mfictc",     XSPR(31,339,1019), XSPR_MASK, PPC750,   { RT } },
{ "mfpbl1",     XSPR(31,339,1020), XSPR_MASK, PPC403,	{ RT } },
{ "mfthrm1",    XSPR(31,339,1020), XSPR_MASK, PPC750,   { RT } },
{ "mfpbu1",     XSPR(31,339,1021), XSPR_MASK, PPC403,	{ RT } },
{ "mfthrm2",    XSPR(31,339,1021), XSPR_MASK, PPC750,   { RT } },
{ "mfpbl2",     XSPR(31,339,1022), XSPR_MASK, PPC403,	{ RT } },
{ "mfthrm3",    XSPR(31,339,1022), XSPR_MASK, PPC750,   { RT } },
{ "mfpbu2",     XSPR(31,339,1023), XSPR_MASK, PPC403,	{ RT } },
{ "mfspr",      X(31,339),	   X_MASK,    COM,	{ RT, SPR } },

{ "lwax",    X(31,341),	X_MASK,		PPC64,		{ RT, RA, RB } },

{ "dst",     XDSS(31,342,0), XDSS_MASK,	PPCVEC,		{ RA, RB, STRM } },
{ "dstt",    XDSS(31,342,1), XDSS_MASK,	PPCVEC,		{ RA, RB, STRM } },

{ "lhax",    X(31,343),	X_MASK,		COM,		{ RT, RA, RB } },

{ "lhaxe",   X(31,351),	X_MASK,		BOOKE64,	{ RT, RA, RB } },

{ "dstst",   XDSS(31,374,0), XDSS_MASK,	PPCVEC,		{ RA, RB, STRM } },
{ "dststt",  XDSS(31,374,1), XDSS_MASK,	PPCVEC,		{ RA, RB, STRM } },

{ "dccci",   X(31,454),	XRT_MASK,	PPC403,		{ RA, RB } },

{ "abs",     XO(31,360,0,0), XORB_MASK, M601,		{ RT, RA } },
{ "abs.",    XO(31,360,0,1), XORB_MASK, M601,		{ RT, RA } },
{ "abso",    XO(31,360,1,0), XORB_MASK, M601,		{ RT, RA } },
{ "abso.",   XO(31,360,1,1), XORB_MASK, M601,		{ RT, RA } },

{ "divs",    XO(31,363,0,0), XO_MASK,	M601,		{ RT, RA, RB } },
{ "divs.",   XO(31,363,0,1), XO_MASK,	M601,		{ RT, RA, RB } },
{ "divso",   XO(31,363,1,0), XO_MASK,	M601,		{ RT, RA, RB } },
{ "divso.",  XO(31,363,1,1), XO_MASK,	M601,		{ RT, RA, RB } },

{ "tlbia",   X(31,370),	0xffffffff,	PPC,		{ 0 } },

{ "mftbl",   XSPR(31,371,268), XSPR_MASK, CLASSIC,	{ RT } },
{ "mftbu",   XSPR(31,371,269), XSPR_MASK, CLASSIC,	{ RT } },
{ "mftb",    X(31,371),	X_MASK,		CLASSIC,	{ RT, TBR } },

{ "lwaux",   X(31,373),	X_MASK,		PPC64,		{ RT, RAL, RB } },

{ "lhaux",   X(31,375),	X_MASK,		COM,		{ RT, RAL, RB } },

{ "lhauxe",  X(31,383),	X_MASK,		BOOKE64,	{ RT, RAL, RB } },

{ "mtdcrx",  X(31,387),	X_MASK,		BOOKE,		{ RA, RS } },

{ "dcblc",   X(31,390),	X_MASK,		PPCCHLK,	{ CT, RA, RB }},

{ "subfe64", XO(31,392,0,0), XO_MASK,	BOOKE64,	{ RT, RA, RB } },
{ "subfe64o",XO(31,392,1,0), XO_MASK,	BOOKE64,	{ RT, RA, RB } },

{ "adde64",  XO(31,394,0,0), XO_MASK,	BOOKE64,	{ RT, RA, RB } },
{ "adde64o", XO(31,394,1,0), XO_MASK,	BOOKE64,	{ RT, RA, RB } },

{ "dcblce",  X(31,398),	X_MASK,		PPCCHLK64,	{ CT, RA, RB }},

{ "slbmte",  X(31,402), XRA_MASK,	PPC64,		{ RS, RB } },

{ "sthx",    X(31,407),	X_MASK,		COM,		{ RS, RA, RB } },

{ "lfqx",    X(31,791),	X_MASK,		POWER2,		{ FRT, RA, RB } },

{ "lfqux",   X(31,823),	X_MASK,		POWER2,		{ FRT, RA, RB } },

{ "stfqx",   X(31,919),	X_MASK,		POWER2,		{ FRS, RA, RB } },

{ "stfqux",  X(31,951),	X_MASK,		POWER2,		{ FRS, RA, RB } },

{ "orc",     XRC(31,412,0), X_MASK,	COM,		{ RA, RS, RB } },
{ "orc.",    XRC(31,412,1), X_MASK,	COM,		{ RA, RS, RB } },

{ "sradi",   XS(31,413,0), XS_MASK,	PPC64,		{ RA, RS, SH6 } },
{ "sradi.",  XS(31,413,1), XS_MASK,	PPC64,		{ RA, RS, SH6 } },

{ "sthxe",   X(31,415),	X_MASK,		BOOKE64,	{ RS, RA, RB } },

{ "slbie",   X(31,434),	XRTRA_MASK,	PPC64,		{ RB } },

{ "ecowx",   X(31,438),	X_MASK,		PPC,		{ RT, RA, RB } },

{ "sthux",   X(31,439),	X_MASK,		COM,		{ RS, RAS, RB } },

{ "sthuxe",  X(31,447),	X_MASK,		BOOKE64,	{ RS, RAS, RB } },

{ "mr",	     XRC(31,444,0), X_MASK,	COM,		{ RA, RS, RBS } },
{ "or",      XRC(31,444,0), X_MASK,	COM,		{ RA, RS, RB } },
{ "mr.",     XRC(31,444,1), X_MASK,	COM,		{ RA, RS, RBS } },
{ "or.",     XRC(31,444,1), X_MASK,	COM,		{ RA, RS, RB } },

{ "mtexisr",  XSPR(31,451,64),  XSPR_MASK, PPC403,	{ RT } },
{ "mtexier",  XSPR(31,451,66),  XSPR_MASK, PPC403,	{ RT } },
{ "mtbr0",    XSPR(31,451,128), XSPR_MASK, PPC403,	{ RT } },
{ "mtbr1",    XSPR(31,451,129), XSPR_MASK, PPC403,	{ RT } },
{ "mtbr2",    XSPR(31,451,130), XSPR_MASK, PPC403,	{ RT } },
{ "mtbr3",    XSPR(31,451,131), XSPR_MASK, PPC403,	{ RT } },
{ "mtbr4",    XSPR(31,451,132), XSPR_MASK, PPC403,	{ RT } },
{ "mtbr5",    XSPR(31,451,133), XSPR_MASK, PPC403,	{ RT } },
{ "mtbr6",    XSPR(31,451,134), XSPR_MASK, PPC403,	{ RT } },
{ "mtbr7",    XSPR(31,451,135), XSPR_MASK, PPC403,	{ RT } },
{ "mtbear",   XSPR(31,451,144), XSPR_MASK, PPC403,	{ RT } },
{ "mtbesr",   XSPR(31,451,145), XSPR_MASK, PPC403,	{ RT } },
{ "mtiocr",   XSPR(31,451,160), XSPR_MASK, PPC403,	{ RT } },
{ "mtdmacr0", XSPR(31,451,192), XSPR_MASK, PPC403,	{ RT } },
{ "mtdmact0", XSPR(31,451,193), XSPR_MASK, PPC403,	{ RT } },
{ "mtdmada0", XSPR(31,451,194), XSPR_MASK, PPC403,	{ RT } },
{ "mtdmasa0", XSPR(31,451,195), XSPR_MASK, PPC403,	{ RT } },
{ "mtdmacc0", XSPR(31,451,196), XSPR_MASK, PPC403,	{ RT } },
{ "mtdmacr1", XSPR(31,451,200), XSPR_MASK, PPC403,	{ RT } },
{ "mtdmact1", XSPR(31,451,201), XSPR_MASK, PPC403,	{ RT } },
{ "mtdmada1", XSPR(31,451,202), XSPR_MASK, PPC403,	{ RT } },
{ "mtdmasa1", XSPR(31,451,203), XSPR_MASK, PPC403,	{ RT } },
{ "mtdmacc1", XSPR(31,451,204), XSPR_MASK, PPC403,	{ RT } },
{ "mtdmacr2", XSPR(31,451,208), XSPR_MASK, PPC403,	{ RT } },
{ "mtdmact2", XSPR(31,451,209), XSPR_MASK, PPC403,	{ RT } },
{ "mtdmada2", XSPR(31,451,210), XSPR_MASK, PPC403,	{ RT } },
{ "mtdmasa2", XSPR(31,451,211), XSPR_MASK, PPC403,	{ RT } },
{ "mtdmacc2", XSPR(31,451,212), XSPR_MASK, PPC403,	{ RT } },
{ "mtdmacr3", XSPR(31,451,216), XSPR_MASK, PPC403,	{ RT } },
{ "mtdmact3", XSPR(31,451,217), XSPR_MASK, PPC403,	{ RT } },
{ "mtdmada3", XSPR(31,451,218), XSPR_MASK, PPC403,	{ RT } },
{ "mtdmasa3", XSPR(31,451,219), XSPR_MASK, PPC403,	{ RT } },
{ "mtdmacc3", XSPR(31,451,220), XSPR_MASK, PPC403,	{ RT } },
{ "mtdmasr",  XSPR(31,451,224), XSPR_MASK, PPC403,	{ RT } },
{ "mtdcr",    X(31,451),	X_MASK,		PPC403,		{ SPR, RS } },
{ "mtdcr",    X(31,451),	X_MASK,		BOOKE,		{ SPR, RS } },

{ "subfze64",XO(31,456,0,0), XORB_MASK, BOOKE64,	{ RT, RA } },
{ "subfze64o",XO(31,456,1,0), XORB_MASK, BOOKE64,	{ RT, RA } },

{ "divdu",   XO(31,457,0,0), XO_MASK,	PPC64,		{ RT, RA, RB } },
{ "divdu.",  XO(31,457,0,1), XO_MASK,	PPC64,		{ RT, RA, RB } },
{ "divduo",  XO(31,457,1,0), XO_MASK,	PPC64,		{ RT, RA, RB } },
{ "divduo.", XO(31,457,1,1), XO_MASK,	PPC64,		{ RT, RA, RB } },

{ "addze64", XO(31,458,0,0), XORB_MASK, BOOKE64,	{ RT, RA } },
{ "addze64o",XO(31,458,1,0), XORB_MASK, BOOKE64,	{ RT, RA } },

{ "divwu",   XO(31,459,0,0), XO_MASK,	PPC,		{ RT, RA, RB } },
{ "divwu.",  XO(31,459,0,1), XO_MASK,	PPC,		{ RT, RA, RB } },
{ "divwuo",  XO(31,459,1,0), XO_MASK,	PPC,		{ RT, RA, RB } },
{ "divwuo.", XO(31,459,1,1), XO_MASK,	PPC,		{ RT, RA, RB } },

{ "mtmq",      XSPR(31,467,0),    XSPR_MASK, M601,	{ RS } },
{ "mtxer",     XSPR(31,467,1),    XSPR_MASK, COM,	{ RS } },
{ "mtlr",      XSPR(31,467,8),    XSPR_MASK, COM,	{ RS } },
{ "mtctr",     XSPR(31,467,9),    XSPR_MASK, COM,	{ RS } },
{ "mttid",     XSPR(31,467,17),   XSPR_MASK, POWER,	{ RS } },
{ "mtdsisr",   XSPR(31,467,18),   XSPR_MASK, COM,	{ RS } },
{ "mtdar",     XSPR(31,467,19),   XSPR_MASK, COM,	{ RS } },
{ "mtrtcu",    XSPR(31,467,20),   XSPR_MASK, COM,	{ RS } },
{ "mtrtcl",    XSPR(31,467,21),   XSPR_MASK, COM,	{ RS } },
{ "mtdec",     XSPR(31,467,22),   XSPR_MASK, COM,	{ RS } },
{ "mtsdr0",    XSPR(31,467,24),   XSPR_MASK, POWER,	{ RS } },
{ "mtsdr1",    XSPR(31,467,25),   XSPR_MASK, COM,	{ RS } },
{ "mtsrr0",    XSPR(31,467,26),   XSPR_MASK, COM,	{ RS } },
{ "mtsrr1",    XSPR(31,467,27),   XSPR_MASK, COM,	{ RS } },
{ "mtpid",     XSPR(31,467,48),   XSPR_MASK, BOOKE,     { RS } },
{ "mtdecar",   XSPR(31,467,54),   XSPR_MASK, BOOKE,     { RS } },
{ "mtcsrr0",   XSPR(31,467,58),   XSPR_MASK, BOOKE,     { RS } },
{ "mtcsrr1",   XSPR(31,467,59),   XSPR_MASK, BOOKE,     { RS } },
{ "mtdear",    XSPR(31,467,61),   XSPR_MASK, BOOKE,     { RS } },
{ "mtesr",     XSPR(31,467,62),   XSPR_MASK, BOOKE,     { RS } },
{ "mtivpr",    XSPR(31,467,63),   XSPR_MASK, BOOKE,     { RS } },
{ "mtcmpa",    XSPR(31,467,144),  XSPR_MASK, PPC860,	{ RT } },
{ "mtcmpb",    XSPR(31,467,145),  XSPR_MASK, PPC860,	{ RT } },
{ "mtcmpc",    XSPR(31,467,146),  XSPR_MASK, PPC860,	{ RT } },
{ "mtcmpd",    XSPR(31,467,147),  XSPR_MASK, PPC860,	{ RT } },
{ "mticr",     XSPR(31,467,148),  XSPR_MASK, PPC860,	{ RT } },
{ "mtder",     XSPR(31,467,149),  XSPR_MASK, PPC860,	{ RT } },
{ "mtcounta",  XSPR(31,467,150),  XSPR_MASK, PPC860,	{ RT } },
{ "mtcountb",  XSPR(31,467,151),  XSPR_MASK, PPC860,	{ RT } },
{ "mtcmpe",    XSPR(31,467,152),  XSPR_MASK, PPC860,	{ RT } },
{ "mtcmpf",    XSPR(31,467,153),  XSPR_MASK, PPC860,	{ RT } },
{ "mtcmpg",    XSPR(31,467,154),  XSPR_MASK, PPC860,	{ RT } },
{ "mtcmph",    XSPR(31,467,155),  XSPR_MASK, PPC860,	{ RT } },
{ "mtlctrl1",  XSPR(31,467,156),  XSPR_MASK, PPC860,	{ RT } },
{ "mtlctrl2",  XSPR(31,467,157),  XSPR_MASK, PPC860,	{ RT } },
{ "mtictrl",   XSPR(31,467,158),  XSPR_MASK, PPC860,	{ RT } },
{ "mtbar",     XSPR(31,467,159),  XSPR_MASK, PPC860,	{ RT } },
{ "mtvrsave",  XSPR(31,467,256),  XSPR_MASK, PPCVEC,	{ RT } },
{ "mtusprg0",  XSPR(31,467,256),  XSPR_MASK, BOOKE,     { RS } },
{ "mtsprg",    XSPR(31,467,272),  XSPRG_MASK,PPC,	{ SPRG, RS } },
{ "mtsprg0",   XSPR(31,467,272),  XSPR_MASK, PPC,	{ RT } },
{ "mtsprg1",   XSPR(31,467,273),  XSPR_MASK, PPC,	{ RT } },
{ "mtsprg2",   XSPR(31,467,274),  XSPR_MASK, PPC,	{ RT } },
{ "mtsprg3",   XSPR(31,467,275),  XSPR_MASK, PPC,	{ RT } },
{ "mtsprg4",   XSPR(31,467,276),  XSPR_MASK, PPC405,	{ RT } },
{ "mtsprg4",   XSPR(31,467,276),  XSPR_MASK, BOOKE,     { RS } },
{ "mtsprg5",   XSPR(31,467,277),  XSPR_MASK, PPC405,	{ RT } },
{ "mtsprg5",   XSPR(31,467,277),  XSPR_MASK, BOOKE,     { RS } },
{ "mtsprg6",   XSPR(31,467,278),  XSPR_MASK, PPC405,	{ RT } },
{ "mtsprg6",   XSPR(31,467,278),  XSPR_MASK, BOOKE,     { RS } },
{ "mtsprg7",   XSPR(31,467,279),  XSPR_MASK, PPC405,	{ RT } },
{ "mtsprg7",   XSPR(31,467,279),  XSPR_MASK, BOOKE,     { RS } },
{ "mtasr",     XSPR(31,467,280),  XSPR_MASK, PPC64,	{ RS } },
{ "mtear",     XSPR(31,467,282),  XSPR_MASK, PPC,	{ RS } },
{ "mttbl",     XSPR(31,467,284),  XSPR_MASK, PPC,	{ RS } },
{ "mttbu",     XSPR(31,467,285),  XSPR_MASK, PPC,	{ RS } },
{ "mtdbsr",    XSPR(31,467,304),  XSPR_MASK, BOOKE,     { RS } },
{ "mtdbcr0",   XSPR(31,467,308),  XSPR_MASK, BOOKE,     { RS } },
{ "mtdbcr1",   XSPR(31,467,309),  XSPR_MASK, BOOKE,     { RS } },
{ "mtdbcr2",   XSPR(31,467,310),  XSPR_MASK, BOOKE,     { RS } },
{ "mtiac1",    XSPR(31,467,312),  XSPR_MASK, BOOKE,     { RS } },
{ "mtiac2",    XSPR(31,467,313),  XSPR_MASK, BOOKE,     { RS } },
{ "mtiac3",    XSPR(31,467,314),  XSPR_MASK, BOOKE,     { RS } },
{ "mtiac4",    XSPR(31,467,315),  XSPR_MASK, BOOKE,     { RS } },
{ "mtdac1",    XSPR(31,467,316),  XSPR_MASK, BOOKE,     { RS } },
{ "mtdac2",    XSPR(31,467,317),  XSPR_MASK, BOOKE,     { RS } },
{ "mtdvc1",    XSPR(31,467,318),  XSPR_MASK, BOOKE,     { RS } },
{ "mtdvc2",    XSPR(31,467,319),  XSPR_MASK, BOOKE,     { RS } },
{ "mttsr",     XSPR(31,467,336),  XSPR_MASK, BOOKE,     { RS } },
{ "mttcr",     XSPR(31,467,340),  XSPR_MASK, BOOKE,     { RS } },
{ "mtivor0",   XSPR(31,467,400),  XSPR_MASK, BOOKE,     { RS } },
{ "mtivor1",   XSPR(31,467,401),  XSPR_MASK, BOOKE,     { RS } },
{ "mtivor2",   XSPR(31,467,402),  XSPR_MASK, BOOKE,     { RS } },
{ "mtivor3",   XSPR(31,467,403),  XSPR_MASK, BOOKE,     { RS } },
{ "mtivor4",   XSPR(31,467,404),  XSPR_MASK, BOOKE,     { RS } },
{ "mtivor5",   XSPR(31,467,405),  XSPR_MASK, BOOKE,     { RS } },
{ "mtivor6",   XSPR(31,467,406),  XSPR_MASK, BOOKE,     { RS } },
{ "mtivor7",   XSPR(31,467,407),  XSPR_MASK, BOOKE,     { RS } },
{ "mtivor8",   XSPR(31,467,408),  XSPR_MASK, BOOKE,     { RS } },
{ "mtivor9",   XSPR(31,467,409),  XSPR_MASK, BOOKE,     { RS } },
{ "mtivor10",  XSPR(31,467,410),  XSPR_MASK, BOOKE,     { RS } },
{ "mtivor11",  XSPR(31,467,411),  XSPR_MASK, BOOKE,     { RS } },
{ "mtivor12",  XSPR(31,467,412),  XSPR_MASK, BOOKE,     { RS } },
{ "mtivor13",  XSPR(31,467,413),  XSPR_MASK, BOOKE,     { RS } },
{ "mtivor14",  XSPR(31,467,414),  XSPR_MASK, BOOKE,     { RS } },
{ "mtivor15",  XSPR(31,467,415),  XSPR_MASK, BOOKE,     { RS } },
{ "mtspefscr",  XSPR(31,467,512),  XSPR_MASK, PPCSPE,   { RS } },
{ "mtbbear",   XSPR(31,467,513),  XSPR_MASK, PPCBRLK,   { RS } },
{ "mtbbtar",   XSPR(31,467,514),  XSPR_MASK, PPCBRLK,  { RS } },
{ "mtibatu",   XSPR(31,467,528),  XSPRBAT_MASK, PPC,	{ SPRBAT, RS } },
{ "mtibatl",   XSPR(31,467,529),  XSPRBAT_MASK, PPC,	{ SPRBAT, RS } },
{ "mtdbatu",   XSPR(31,467,536),  XSPRBAT_MASK, PPC,	{ SPRBAT, RS } },
{ "mtdbatl",   XSPR(31,467,537),  XSPRBAT_MASK, PPC,	{ SPRBAT, RS } },
{ "mtmcsrr0",  XSPR(31,467,570),  XSPR_MASK, PPCRFMCI,  { RS } },
{ "mtmcsrr1",  XSPR(31,467,571),  XSPR_MASK, PPCRFMCI,  { RS } },
{ "mtmcsr",    XSPR(31,467,572),  XSPR_MASK, PPCRFMCI,  { RS } },
{ "mtummcr0",  XSPR(31,467,936),  XSPR_MASK, PPC750,    { RT } },
{ "mtupmc1",   XSPR(31,467,937),  XSPR_MASK, PPC750,    { RT } },
{ "mtupmc2",   XSPR(31,467,938),  XSPR_MASK, PPC750,    { RT } },
{ "mtusia",    XSPR(31,467,939),  XSPR_MASK, PPC750,    { RT } },
{ "mtummcr1",  XSPR(31,467,940),  XSPR_MASK, PPC750,    { RT } },
{ "mtupmc3",   XSPR(31,467,941),  XSPR_MASK, PPC750,    { RT } },
{ "mtupmc4",   XSPR(31,467,942),  XSPR_MASK, PPC750,    { RT } },
{ "mtzpr",     XSPR(31,467,944),  XSPR_MASK, PPC403,	{ RT } },
{ "mtpid",     XSPR(31,467,945),  XSPR_MASK, PPC403,	{ RT } },
{ "mtccr0",    XSPR(31,467,947),  XSPR_MASK, PPC405,	{ RT } },
{ "mtiac3",    XSPR(31,467,948),  XSPR_MASK, PPC405,	{ RT } },
{ "mtiac4",    XSPR(31,467,949),  XSPR_MASK, PPC405,	{ RT } },
{ "mtdvc1",    XSPR(31,467,950),  XSPR_MASK, PPC405,	{ RT } },
{ "mtdvc2",    XSPR(31,467,951),  XSPR_MASK, PPC405,	{ RT } },
{ "mtmmcr0",   XSPR(31,467,952),  XSPR_MASK, PPC750,    { RT } },
{ "mtsgr",     XSPR(31,467,953),  XSPR_MASK, PPC403,	{ RT } },
{ "mtpmc1",    XSPR(31,467,953),  XSPR_MASK, PPC750,    { RT } },
{ "mtdcwr",    XSPR(31,467,954),  XSPR_MASK, PPC403,	{ RT } },
{ "mtpmc2",    XSPR(31,467,954),  XSPR_MASK, PPC750,    { RT } },
{ "mtsler",    XSPR(31,467,955),  XSPR_MASK, PPC405,	{ RT } },
{ "mtsia",     XSPR(31,467,955),  XSPR_MASK, PPC750,    { RT } },
{ "mtsu0r",    XSPR(31,467,956),  XSPR_MASK, PPC405,	{ RT } },
{ "mtmmcr1",   XSPR(31,467,956),  XSPR_MASK, PPC750,    { RT } },
{ "mtdbcr1",   XSPR(31,467,957),  XSPR_MASK, PPC405,	{ RT } },
{ "mtpmc3",    XSPR(31,467,957),  XSPR_MASK, PPC750,    { RT } },
{ "mtpmc4",    XSPR(31,467,958),  XSPR_MASK, PPC750,    { RT } },
{ "mticdbdr",  XSPR(31,467,979),  XSPR_MASK, PPC403,	{ RT } },
{ "mtesr",     XSPR(31,467,980),  XSPR_MASK, PPC403,	{ RT } },
{ "mtdear",    XSPR(31,467,981),  XSPR_MASK, PPC403,	{ RT } },
{ "mtevpr",    XSPR(31,467,982),  XSPR_MASK, PPC403,	{ RT } },
{ "mtcdbcr",   XSPR(31,467,983),  XSPR_MASK, PPC403,	{ RT } },
{ "mttsr",     XSPR(31,467,984),  XSPR_MASK, PPC403,	{ RT } },
{ "mttcr",     XSPR(31,467,986),  XSPR_MASK, PPC403,	{ RT } },
{ "mtpit",     XSPR(31,467,987),  XSPR_MASK, PPC403,	{ RT } },
{ "mttbhi",    XSPR(31,467,988),  XSPR_MASK, PPC403,	{ RT } },
{ "mttblo",    XSPR(31,467,989),  XSPR_MASK, PPC403,	{ RT } },
{ "mtsrr2",    XSPR(31,467,990),  XSPR_MASK, PPC403,	{ RT } },
{ "mtsrr3",    XSPR(31,467,991),  XSPR_MASK, PPC403,	{ RT } },
{ "mtdbsr",    XSPR(31,467,1008), XSPR_MASK, PPC403,	{ RT } },
{ "mtdbcr0",   XSPR(31,467,1010), XSPR_MASK, PPC405,	{ RT } },
{ "mtiac1",    XSPR(31,467,1012), XSPR_MASK, PPC403,	{ RT } },
{ "mtiac2",    XSPR(31,467,1013), XSPR_MASK, PPC403,	{ RT } },
{ "mtdac1",    XSPR(31,467,1014), XSPR_MASK, PPC403,	{ RT } },
{ "mtdac2",    XSPR(31,467,1015), XSPR_MASK, PPC403,	{ RT } },
{ "mtl2cr",    XSPR(31,467,1017), XSPR_MASK, PPC750,    { RT } },
{ "mtdccr",    XSPR(31,467,1018), XSPR_MASK, PPC403,	{ RT } },
{ "mticcr",    XSPR(31,467,1019), XSPR_MASK, PPC403,	{ RT } },
{ "mtictc",    XSPR(31,467,1019), XSPR_MASK, PPC750,    { RT } },
{ "mtpbl1",    XSPR(31,467,1020), XSPR_MASK, PPC403,	{ RT } },
{ "mtthrm1",   XSPR(31,467,1020), XSPR_MASK, PPC750,    { RT } },
{ "mtpbu1",    XSPR(31,467,1021), XSPR_MASK, PPC403,	{ RT } },
{ "mtthrm2",   XSPR(31,467,1021), XSPR_MASK, PPC750,    { RT } },
{ "mtpbl2",    XSPR(31,467,1022), XSPR_MASK, PPC403,	{ RT } },
{ "mtthrm3",   XSPR(31,467,1022), XSPR_MASK, PPC750,    { RT } },
{ "mtpbu2",    XSPR(31,467,1023), XSPR_MASK, PPC403,	{ RT } },
{ "mtspr",     X(31,467),	  X_MASK,    COM,	{ SPR, RS } },

{ "dcbi",    X(31,470),	XRT_MASK,	PPC,		{ RA, RB } },

{ "nand",    XRC(31,476,0), X_MASK,	COM,		{ RA, RS, RB } },
{ "nand.",   XRC(31,476,1), X_MASK,	COM,		{ RA, RS, RB } },

{ "dcbie",   X(31,478),	XRT_MASK,	BOOKE64,	{ RA, RB } },

{ "dcread",  X(31,486),	X_MASK,		PPC403,		{ RT, RA, RB }},

{ "mtpmr",   X(31,462),	X_MASK,		PPCPMR,		{ PMR, RS }},

{ "icbtls",  X(31,486),	X_MASK,		PPCCHLK,	{ CT, RA, RB }},

{ "nabs",    XO(31,488,0,0), XORB_MASK, M601,		{ RT, RA } },
{ "subfme64",XO(31,488,0,0), XORB_MASK, BOOKE64,	{ RT, RA } },
{ "nabs.",   XO(31,488,0,1), XORB_MASK, M601,		{ RT, RA } },
{ "nabso",   XO(31,488,1,0), XORB_MASK, M601,		{ RT, RA } },
{ "subfme64o",XO(31,488,1,0), XORB_MASK, BOOKE64,	{ RT, RA } },
{ "nabso.",  XO(31,488,1,1), XORB_MASK, M601,		{ RT, RA } },

{ "divd",    XO(31,489,0,0), XO_MASK,	PPC64,		{ RT, RA, RB } },
{ "divd.",   XO(31,489,0,1), XO_MASK,	PPC64,		{ RT, RA, RB } },
{ "divdo",   XO(31,489,1,0), XO_MASK,	PPC64,		{ RT, RA, RB } },
{ "divdo.",  XO(31,489,1,1), XO_MASK,	PPC64,		{ RT, RA, RB } },

{ "addme64", XO(31,490,0,0), XORB_MASK, BOOKE64,	{ RT, RA } },
{ "addme64o",XO(31,490,1,0), XORB_MASK, BOOKE64,	{ RT, RA } },

{ "divw",    XO(31,491,0,0), XO_MASK,	PPC,		{ RT, RA, RB } },
{ "divw.",   XO(31,491,0,1), XO_MASK,	PPC,		{ RT, RA, RB } },
{ "divwo",   XO(31,491,1,0), XO_MASK,	PPC,		{ RT, RA, RB } },
{ "divwo.",  XO(31,491,1,1), XO_MASK,	PPC,		{ RT, RA, RB } },

{ "icbtlse", X(31,494),	X_MASK,		PPCCHLK64,	{ CT, RA, RB }},

{ "slbia",   X(31,498),	0xffffffff,	PPC64,		{ 0 } },

{ "cli",     X(31,502), XRB_MASK,	POWER,		{ RT, RA } },

{ "stdcxe.", XRC(31,511,1), X_MASK,	BOOKE64,	{ RS, RA, RB } },

{ "mcrxr",   X(31,512),	XRARB_MASK|(3<<21), COM,	{ BF } },

{ "bblels",  X(31,518),	X_MASK,		PPCBRLK,	{ 0 }},
{ "mcrxr64", X(31,544),	XRARB_MASK|(3<<21), BOOKE64,	{ BF } },

{ "clcs",    X(31,531), XRB_MASK,	M601,		{ RT, RA } },

{ "lswx",    X(31,533),	X_MASK,		PPCCOM,		{ RT, RA, RB } },
{ "lsx",     X(31,533),	X_MASK,		PWRCOM,		{ RT, RA, RB } },

{ "lwbrx",   X(31,534),	X_MASK,		PPCCOM,		{ RT, RA, RB } },
{ "lbrx",    X(31,534),	X_MASK,		PWRCOM,		{ RT, RA, RB } },

{ "lfsx",    X(31,535),	X_MASK,		COM,		{ FRT, RA, RB } },

{ "srw",     XRC(31,536,0), X_MASK,	PPCCOM,		{ RA, RS, RB } },
{ "sr",      XRC(31,536,0), X_MASK,	PWRCOM,		{ RA, RS, RB } },
{ "srw.",    XRC(31,536,1), X_MASK,	PPCCOM,		{ RA, RS, RB } },
{ "sr.",     XRC(31,536,1), X_MASK,	PWRCOM,		{ RA, RS, RB } },

{ "rrib",    XRC(31,537,0), X_MASK,	M601,		{ RA, RS, RB } },
{ "rrib.",   XRC(31,537,1), X_MASK,	M601,		{ RA, RS, RB } },

{ "srd",     XRC(31,539,0), X_MASK,	PPC64,		{ RA, RS, RB } },
{ "srd.",    XRC(31,539,1), X_MASK,	PPC64,		{ RA, RS, RB } },

{ "maskir",  XRC(31,541,0), X_MASK,	M601,		{ RA, RS, RB } },
{ "maskir.", XRC(31,541,1), X_MASK,	M601,		{ RA, RS, RB } },

{ "lwbrxe",  X(31,542),	X_MASK,		BOOKE64,	{ RT, RA, RB } },

{ "lfsxe",   X(31,543),	X_MASK,		BOOKE64,	{ FRT, RA, RB } },

{ "bbelr",   X(31,550),	X_MASK,		PPCBRLK,	{ 0 }},
{ "tlbsync", X(31,566),	0xffffffff,	PPC,		{ 0 } },

{ "lfsux",   X(31,567),	X_MASK,		COM,		{ FRT, RAS, RB } },

{ "lfsuxe",  X(31,575),	X_MASK,		BOOKE64,	{ FRT, RAS, RB } },

{ "mfsr",    X(31,595),	XRB_MASK|(1<<20), COM32,	{ RT, SR } },

{ "lswi",    X(31,597),	X_MASK,		PPCCOM,		{ RT, RA, NB } },
{ "lsi",     X(31,597),	X_MASK,		PWRCOM,		{ RT, RA, NB } },

{ "lwsync",  XSYNC(31,598,1), 0xffffffff, PPCONLY,	{ 0 } },
{ "ptesync", XSYNC(31,598,2), 0xffffffff, PPC64,	{ 0 } },
{ "msync",   X(31,598), 0xffffffff,	BOOKE,		{ 0 } },
{ "sync",    X(31,598), XSYNC_MASK,	PPCCOM,		{ LS } },
{ "dcs",     X(31,598), 0xffffffff,	PWRCOM,		{ 0 } },

{ "lfdx",    X(31,599), X_MASK,		COM,		{ FRT, RA, RB } },

{ "lfdxe",   X(31,607), X_MASK,		BOOKE64,	{ FRT, RA, RB } },

{ "mfsri",   X(31,627), X_MASK,		PWRCOM,		{ RT, RA, RB } },

{ "dclst",   X(31,630), XRB_MASK,	PWRCOM,		{ RS, RA } },

{ "lfdux",   X(31,631), X_MASK,		COM,		{ FRT, RAS, RB } },

{ "lfduxe",  X(31,639), X_MASK,		BOOKE64,	{ FRT, RAS, RB } },

{ "mfsrin",  X(31,659), XRA_MASK,	PPC32,		{ RT, RB } },

{ "stswx",   X(31,661), X_MASK,		PPCCOM,		{ RS, RA, RB } },
{ "stsx",    X(31,661), X_MASK,		PWRCOM,		{ RS, RA, RB } },

{ "stwbrx",  X(31,662), X_MASK,		PPCCOM,		{ RS, RA, RB } },
{ "stbrx",   X(31,662), X_MASK,		PWRCOM,		{ RS, RA, RB } },

{ "stfsx",   X(31,663), X_MASK,		COM,		{ FRS, RA, RB } },

{ "srq",     XRC(31,664,0), X_MASK,	M601,		{ RA, RS, RB } },
{ "srq.",    XRC(31,664,1), X_MASK,	M601,		{ RA, RS, RB } },

{ "sre",     XRC(31,665,0), X_MASK,	M601,		{ RA, RS, RB } },
{ "sre.",    XRC(31,665,1), X_MASK,	M601,		{ RA, RS, RB } },

{ "stwbrxe", X(31,670), X_MASK,		BOOKE64,	{ RS, RA, RB } },

{ "stfsxe",  X(31,671), X_MASK,		BOOKE64,	{ FRS, RA, RB } },

{ "stfsux",  X(31,695),	X_MASK,		COM,		{ FRS, RAS, RB } },

{ "sriq",    XRC(31,696,0), X_MASK,	M601,		{ RA, RS, SH } },
{ "sriq.",   XRC(31,696,1), X_MASK,	M601,		{ RA, RS, SH } },

{ "stfsuxe", X(31,703),	X_MASK,		BOOKE64,	{ FRS, RAS, RB } },

{ "stswi",   X(31,725),	X_MASK,		PPCCOM,		{ RS, RA, NB } },
{ "stsi",    X(31,725),	X_MASK,		PWRCOM,		{ RS, RA, NB } },

{ "stfdx",   X(31,727),	X_MASK,		COM,		{ FRS, RA, RB } },

{ "srlq",    XRC(31,728,0), X_MASK,	M601,		{ RA, RS, RB } },
{ "srlq.",   XRC(31,728,1), X_MASK,	M601,		{ RA, RS, RB } },

{ "sreq",    XRC(31,729,0), X_MASK,	M601,		{ RA, RS, RB } },
{ "sreq.",   XRC(31,729,1), X_MASK,	M601,		{ RA, RS, RB } },

{ "stfdxe",  X(31,735),	X_MASK,		BOOKE64,	{ FRS, RA, RB } },

{ "dcba",    X(31,758),	XRT_MASK,	PPC405,		{ RA, RB } },
{ "dcba",    X(31,758),	XRT_MASK,	BOOKE,		{ RA, RB } },

{ "stfdux",  X(31,759),	X_MASK,		COM,		{ FRS, RAS, RB } },

{ "srliq",   XRC(31,760,0), X_MASK,	M601,		{ RA, RS, SH } },
{ "srliq.",  XRC(31,760,1), X_MASK,	M601,		{ RA, RS, SH } },

{ "dcbae",   X(31,766),	XRT_MASK,	BOOKE64,	{ RA, RB } },

{ "stfduxe", X(31,767),	X_MASK,		BOOKE64,	{ FRS, RAS, RB } },

{ "tlbivax", X(31,786),	XRT_MASK,	BOOKE,		{ RA, RB } },
{ "tlbivaxe",X(31,787),	XRT_MASK,	BOOKE64,	{ RA, RB } },

{ "lhbrx",   X(31,790),	X_MASK,		COM,		{ RT, RA, RB } },

{ "sraw",    XRC(31,792,0), X_MASK,	PPCCOM,		{ RA, RS, RB } },
{ "sra",     XRC(31,792,0), X_MASK,	PWRCOM,		{ RA, RS, RB } },
{ "sraw.",   XRC(31,792,1), X_MASK,	PPCCOM,		{ RA, RS, RB } },
{ "sra.",    XRC(31,792,1), X_MASK,	PWRCOM,		{ RA, RS, RB } },

{ "srad",    XRC(31,794,0), X_MASK,	PPC64,		{ RA, RS, RB } },
{ "srad.",   XRC(31,794,1), X_MASK,	PPC64,		{ RA, RS, RB } },

{ "lhbrxe",  X(31,798),	X_MASK,		BOOKE64,	{ RT, RA, RB } },

{ "ldxe",    X(31,799),	X_MASK,		BOOKE64,	{ RT, RA, RB } },
{ "lduxe",   X(31,831),	X_MASK,		BOOKE64,	{ RT, RA, RB } },

{ "rac",     X(31,818),	X_MASK,		PWRCOM,		{ RT, RA, RB } },

{ "dss",     XDSS(31,822,0), XDSS_MASK,	PPCVEC,		{ STRM } },
{ "dssall",  XDSS(31,822,1), XDSS_MASK,	PPCVEC,		{ 0 } },

{ "srawi",   XRC(31,824,0), X_MASK,	PPCCOM,		{ RA, RS, SH } },
{ "srai",    XRC(31,824,0), X_MASK,	PWRCOM,		{ RA, RS, SH } },
{ "srawi.",  XRC(31,824,1), X_MASK,	PPCCOM,		{ RA, RS, SH } },
{ "srai.",   XRC(31,824,1), X_MASK,	PWRCOM,		{ RA, RS, SH } },

{ "slbmfev", X(31,851), XRA_MASK,	PPC64,		{ RT, RB } },

{ "mbar",    X(31,854),	X_MASK,		BOOKE,		{ MO } },
{ "eieio",   X(31,854),	0xffffffff,	PPC,		{ 0 } },

{ "tlbsx",   XRC(31,914,0), X_MASK, 	PPC403,		{ RT, RA, RB } },
{ "tlbsx.",  XRC(31,914,1), X_MASK, 	PPC403,		{ RT, RA, RB } },

{ "tlbsx",   XRC(31,914,0), X_MASK,	BOOKE,		{ RA, RB } },
{ "tlbsx.",  XRC(31,914,1), X_MASK,	BOOKE,		{ RA, RB } },
{ "tlbsxe",  XRC(31,915,0), X_MASK,	BOOKE64,	{ RA, RB } },
{ "tlbsxe.", XRC(31,915,1), X_MASK,	BOOKE64,	{ RA, RB } },

{ "slbmfee", X(31,915), XRA_MASK,	PPC64,		{ RT, RB } },

{ "sthbrx",  X(31,918),	X_MASK,		COM,		{ RS, RA, RB } },

{ "sraq",    XRC(31,920,0), X_MASK,	M601,		{ RA, RS, RB } },
{ "sraq.",   XRC(31,920,1), X_MASK,	M601,		{ RA, RS, RB } },

{ "srea",    XRC(31,921,0), X_MASK,	M601,		{ RA, RS, RB } },
{ "srea.",   XRC(31,921,1), X_MASK,	M601,		{ RA, RS, RB } },

{ "extsh",   XRC(31,922,0), XRB_MASK,	PPCCOM,		{ RA, RS } },
{ "exts",    XRC(31,922,0), XRB_MASK,	PWRCOM,		{ RA, RS } },
{ "extsh.",  XRC(31,922,1), XRB_MASK,	PPCCOM,		{ RA, RS } },
{ "exts.",   XRC(31,922,1), XRB_MASK,	PWRCOM,		{ RA, RS } },

{ "sthbrxe", X(31,926),	X_MASK,		BOOKE64,	{ RS, RA, RB } },

{ "stdxe",   X(31,927), X_MASK,		BOOKE64,	{ RS, RA, RB } },

{ "tlbre",   X(31,946),	X_MASK,		BOOKE,		{ 0 } },

{ "tlbrehi", XTLB(31,946,0), XTLB_MASK,	PPC403,		{ RT, RA } },
{ "tlbrelo", XTLB(31,946,1), XTLB_MASK,	PPC403,		{ RT, RA } },
{ "tlbre",   X(31,946),	X_MASK,		PPC403,		{ RS, RA, SH } },

{ "sraiq",   XRC(31,952,0), X_MASK,	M601,		{ RA, RS, SH } },
{ "sraiq.",  XRC(31,952,1), X_MASK,	M601,		{ RA, RS, SH } },

{ "extsb",   XRC(31,954,0), XRB_MASK,	PPC,		{ RA, RS} },
{ "extsb.",  XRC(31,954,1), XRB_MASK,	PPC,		{ RA, RS} },

{ "stduxe",  X(31,959),	X_MASK,		BOOKE64,	{ RS, RAS, RB } },

{ "iccci",   X(31,966),	XRT_MASK,	PPC403,		{ RA, RB } },

{ "tlbwe",   X(31,978),	X_MASK,		BOOKE,		{ 0 } },

{ "tlbld",   X(31,978),	XRTRA_MASK,	PPC,		{ RB } },

{ "tlbwehi", XTLB(31,978,0), XTLB_MASK,	PPC403,		{ RT, RA } },
{ "tlbwelo", XTLB(31,978,1), XTLB_MASK,	PPC403,		{ RT, RA } },
{ "tlbwe",   X(31,978),	X_MASK,		PPC403,		{ RS, RA, SH } },

{ "icbi",    X(31,982),	XRT_MASK,	PPC,		{ RA, RB } },

{ "stfiwx",  X(31,983),	X_MASK,		PPC,		{ FRS, RA, RB } },

{ "extsw",   XRC(31,986,0), XRB_MASK,	PPC64 | BOOKE64,{ RA, RS } },
{ "extsw.",  XRC(31,986,1), XRB_MASK,	PPC64,		{ RA, RS } },

{ "icread",  X(31,998),	XRT_MASK,	PPC403,		{ RA, RB } },

{ "icbie",   X(31,990),	XRT_MASK,	BOOKE64,	{ RA, RB } },
{ "stfiwxe", X(31,991),	X_MASK,		BOOKE64,	{ FRS, RA, RB } },

{ "tlbli",   X(31,1010), XRTRA_MASK,	PPC,		{ RB } },

{ "dcbz",    X(31,1014), XRT_MASK,	PPC,		{ RA, RB } },
{ "dclz",    X(31,1014), XRT_MASK,	PPC,		{ RA, RB } },

{ "dcbze",   X(31,1022), XRT_MASK,	BOOKE64,	{ RA, RB } },

{ "lvebx",   X(31,   7), X_MASK,	PPCVEC,		{ VD, RA, RB } },
{ "lvehx",   X(31,  39), X_MASK,	PPCVEC,		{ VD, RA, RB } },
{ "lvewx",   X(31,  71), X_MASK,	PPCVEC,		{ VD, RA, RB } },
{ "lvsl",    X(31,   6), X_MASK,	PPCVEC,		{ VD, RA, RB } },
{ "lvsr",    X(31,  38), X_MASK,	PPCVEC,		{ VD, RA, RB } },
{ "lvx",     X(31, 103), X_MASK,	PPCVEC,		{ VD, RA, RB } },
{ "lvxl",    X(31, 359), X_MASK,	PPCVEC,		{ VD, RA, RB } },
{ "stvebx",  X(31, 135), X_MASK,	PPCVEC,		{ VS, RA, RB } },
{ "stvehx",  X(31, 167), X_MASK,	PPCVEC,		{ VS, RA, RB } },
{ "stvewx",  X(31, 199), X_MASK,	PPCVEC,		{ VS, RA, RB } },
{ "stvx",    X(31, 231), X_MASK,	PPCVEC,		{ VS, RA, RB } },
{ "stvxl",   X(31, 487), X_MASK,	PPCVEC,		{ VS, RA, RB } },

{ "lwz",     OP(32),	OP_MASK,	PPCCOM,		{ RT, D, RA } },
{ "l",	     OP(32),	OP_MASK,	PWRCOM,		{ RT, D, RA } },

{ "lwzu",    OP(33),	OP_MASK,	PPCCOM,		{ RT, D, RAL } },
{ "lu",      OP(33),	OP_MASK,	PWRCOM,		{ RT, D, RA } },

{ "lbz",     OP(34),	OP_MASK,	COM,		{ RT, D, RA } },

{ "lbzu",    OP(35),	OP_MASK,	COM,		{ RT, D, RAL } },

{ "stw",     OP(36),	OP_MASK,	PPCCOM,		{ RS, D, RA } },
{ "st",      OP(36),	OP_MASK,	PWRCOM,		{ RS, D, RA } },

{ "stwu",    OP(37),	OP_MASK,	PPCCOM,		{ RS, D, RAS } },
{ "stu",     OP(37),	OP_MASK,	PWRCOM,		{ RS, D, RA } },

{ "stb",     OP(38),	OP_MASK,	COM,		{ RS, D, RA } },

{ "stbu",    OP(39),	OP_MASK,	COM,		{ RS, D, RAS } },

{ "lhz",     OP(40),	OP_MASK,	COM,		{ RT, D, RA } },

{ "lhzu",    OP(41),	OP_MASK,	COM,		{ RT, D, RAL } },

{ "lha",     OP(42),	OP_MASK,	COM,		{ RT, D, RA } },

{ "lhau",    OP(43),	OP_MASK,	COM,		{ RT, D, RAL } },

{ "sth",     OP(44),	OP_MASK,	COM,		{ RS, D, RA } },

{ "sthu",    OP(45),	OP_MASK,	COM,		{ RS, D, RAS } },

{ "lmw",     OP(46),	OP_MASK,	PPCCOM,		{ RT, D, RAM } },
{ "lm",      OP(46),	OP_MASK,	PWRCOM,		{ RT, D, RA } },

{ "stmw",    OP(47),	OP_MASK,	PPCCOM,		{ RS, D, RA } },
{ "stm",     OP(47),	OP_MASK,	PWRCOM,		{ RS, D, RA } },

{ "lfs",     OP(48),	OP_MASK,	COM,		{ FRT, D, RA } },

{ "lfsu",    OP(49),	OP_MASK,	COM,		{ FRT, D, RAS } },

{ "lfd",     OP(50),	OP_MASK,	COM,		{ FRT, D, RA } },

{ "lfdu",    OP(51),	OP_MASK,	COM,		{ FRT, D, RAS } },

{ "stfs",    OP(52),	OP_MASK,	COM,		{ FRS, D, RA } },

{ "stfsu",   OP(53),	OP_MASK,	COM,		{ FRS, D, RAS } },

{ "stfd",    OP(54),	OP_MASK,	COM,		{ FRS, D, RA } },

{ "stfdu",   OP(55),	OP_MASK,	COM,		{ FRS, D, RAS } },

{ "lfq",     OP(56),	OP_MASK,	POWER2,		{ FRT, D, RA } },

{ "lfqu",    OP(57),	OP_MASK,	POWER2,		{ FRT, D, RA } },

{ "lbze",    DEO(58,0), DE_MASK,	BOOKE64,	{ RT, DE, RA } },
{ "lbzue",   DEO(58,1), DE_MASK,	BOOKE64,	{ RT, DE, RAL } },
{ "lhze",    DEO(58,2), DE_MASK,	BOOKE64,	{ RT, DE, RA } },
{ "lhzue",   DEO(58,3), DE_MASK,	BOOKE64,	{ RT, DE, RAL } },
{ "lhae",    DEO(58,4), DE_MASK,	BOOKE64,	{ RT, DE, RA } },
{ "lhaue",   DEO(58,5), DE_MASK,	BOOKE64,	{ RT, DE, RAL } },
{ "lwze",    DEO(58,6), DE_MASK,	BOOKE64,	{ RT, DE, RA } },
{ "lwzue",   DEO(58,7), DE_MASK,	BOOKE64,	{ RT, DE, RAL } },
{ "stbe",    DEO(58,8), DE_MASK,	BOOKE64,	{ RS, DE, RA } },
{ "stbue",   DEO(58,9), DE_MASK,	BOOKE64,	{ RS, DE, RAS } },
{ "sthe",    DEO(58,10), DE_MASK,	BOOKE64,	{ RS, DE, RA } },
{ "sthue",   DEO(58,11), DE_MASK,	BOOKE64,	{ RS, DE, RAS } },
{ "stwe",    DEO(58,14), DE_MASK,	BOOKE64,	{ RS, DE, RA } },
{ "stwue",   DEO(58,15), DE_MASK,	BOOKE64,	{ RS, DE, RAS } },

{ "ld",      DSO(58,0),	DS_MASK,	PPC64,		{ RT, DS, RA } },

{ "ldu",     DSO(58,1), DS_MASK,	PPC64,		{ RT, DS, RAL } },

{ "lwa",     DSO(58,2), DS_MASK,	PPC64,		{ RT, DS, RA } },

{ "fdivs",   A(59,18,0), AFRC_MASK,	PPC,		{ FRT, FRA, FRB } },
{ "fdivs.",  A(59,18,1), AFRC_MASK,	PPC,		{ FRT, FRA, FRB } },

{ "fsubs",   A(59,20,0), AFRC_MASK,	PPC,		{ FRT, FRA, FRB } },
{ "fsubs.",  A(59,20,1), AFRC_MASK,	PPC,		{ FRT, FRA, FRB } },

{ "fadds",   A(59,21,0), AFRC_MASK,	PPC,		{ FRT, FRA, FRB } },
{ "fadds.",  A(59,21,1), AFRC_MASK,	PPC,		{ FRT, FRA, FRB } },

{ "fsqrts",  A(59,22,0), AFRAFRC_MASK,	PPC,		{ FRT, FRB } },
{ "fsqrts.", A(59,22,1), AFRAFRC_MASK,	PPC,		{ FRT, FRB } },

{ "fres",    A(59,24,0), AFRAFRC_MASK,	PPC,		{ FRT, FRB } },
{ "fres.",   A(59,24,1), AFRAFRC_MASK,	PPC,		{ FRT, FRB } },

{ "fmuls",   A(59,25,0), AFRB_MASK,	PPC,		{ FRT, FRA, FRC } },
{ "fmuls.",  A(59,25,1), AFRB_MASK,	PPC,		{ FRT, FRA, FRC } },

{ "fmsubs",  A(59,28,0), A_MASK,	PPC,		{ FRT,FRA,FRC,FRB } },
{ "fmsubs.", A(59,28,1), A_MASK,	PPC,		{ FRT,FRA,FRC,FRB } },

{ "fmadds",  A(59,29,0), A_MASK,	PPC,		{ FRT,FRA,FRC,FRB } },
{ "fmadds.", A(59,29,1), A_MASK,	PPC,		{ FRT,FRA,FRC,FRB } },

{ "fnmsubs", A(59,30,0), A_MASK,	PPC,		{ FRT,FRA,FRC,FRB } },
{ "fnmsubs.",A(59,30,1), A_MASK,	PPC,		{ FRT,FRA,FRC,FRB } },

{ "fnmadds", A(59,31,0), A_MASK,	PPC,		{ FRT,FRA,FRC,FRB } },
{ "fnmadds.",A(59,31,1), A_MASK,	PPC,		{ FRT,FRA,FRC,FRB } },

{ "stfq",    OP(60),	OP_MASK,	POWER2,		{ FRS, D, RA } },

{ "stfqu",   OP(61),	OP_MASK,	POWER2,		{ FRS, D, RA } },

{ "lde",     DEO(62,0), DE_MASK,	BOOKE64,	{ RT, DES, RA } },
{ "ldue",    DEO(62,1), DE_MASK,	BOOKE64,	{ RT, DES, RA } },
{ "lfse",    DEO(62,4), DE_MASK,	BOOKE64,	{ FRT, DES, RA } },
{ "lfsue",   DEO(62,5), DE_MASK,	BOOKE64,	{ FRT, DES, RAS } },
{ "lfde",    DEO(62,6), DE_MASK,	BOOKE64,	{ FRT, DES, RA } },
{ "lfdue",   DEO(62,7), DE_MASK,	BOOKE64,	{ FRT, DES, RAS } },
{ "stde",    DEO(62,8), DE_MASK,	BOOKE64,	{ RS, DES, RA } },
{ "stdue",   DEO(62,9), DE_MASK,	BOOKE64,	{ RS, DES, RAS } },
{ "stfse",   DEO(62,12), DE_MASK,	BOOKE64,	{ FRS, DES, RA } },
{ "stfsue",  DEO(62,13), DE_MASK,	BOOKE64,	{ FRS, DES, RAS } },
{ "stfde",   DEO(62,14), DE_MASK,	BOOKE64,	{ FRS, DES, RA } },
{ "stfdue",  DEO(62,15), DE_MASK,	BOOKE64,	{ FRS, DES, RAS } },

{ "std",     DSO(62,0),	DS_MASK,	PPC64,		{ RS, DS, RA } },

{ "stdu",    DSO(62,1),	DS_MASK,	PPC64,		{ RS, DS, RAS } },

{ "fcmpu",   X(63,0),	X_MASK|(3<<21),	COM,		{ BF, FRA, FRB } },

{ "frsp",    XRC(63,12,0), XRA_MASK,	COM,		{ FRT, FRB } },
{ "frsp.",   XRC(63,12,1), XRA_MASK,	COM,		{ FRT, FRB } },

{ "fctiw",   XRC(63,14,0), XRA_MASK,	PPCCOM,		{ FRT, FRB } },
{ "fcir",    XRC(63,14,0), XRA_MASK,	POWER2,		{ FRT, FRB } },
{ "fctiw.",  XRC(63,14,1), XRA_MASK,	PPCCOM,		{ FRT, FRB } },
{ "fcir.",   XRC(63,14,1), XRA_MASK,	POWER2,		{ FRT, FRB } },

{ "fctiwz",  XRC(63,15,0), XRA_MASK,	PPCCOM,		{ FRT, FRB } },
{ "fcirz",   XRC(63,15,0), XRA_MASK,	POWER2,		{ FRT, FRB } },
{ "fctiwz.", XRC(63,15,1), XRA_MASK,	PPCCOM,		{ FRT, FRB } },
{ "fcirz.",  XRC(63,15,1), XRA_MASK,	POWER2,		{ FRT, FRB } },

{ "fdiv",    A(63,18,0), AFRC_MASK,	PPCCOM,		{ FRT, FRA, FRB } },
{ "fd",      A(63,18,0), AFRC_MASK,	PWRCOM,		{ FRT, FRA, FRB } },
{ "fdiv.",   A(63,18,1), AFRC_MASK,	PPCCOM,		{ FRT, FRA, FRB } },
{ "fd.",     A(63,18,1), AFRC_MASK,	PWRCOM,		{ FRT, FRA, FRB } },

{ "fsub",    A(63,20,0), AFRC_MASK,	PPCCOM,		{ FRT, FRA, FRB } },
{ "fs",      A(63,20,0), AFRC_MASK,	PWRCOM,		{ FRT, FRA, FRB } },
{ "fsub.",   A(63,20,1), AFRC_MASK,	PPCCOM,		{ FRT, FRA, FRB } },
{ "fs.",     A(63,20,1), AFRC_MASK,	PWRCOM,		{ FRT, FRA, FRB } },

{ "fadd",    A(63,21,0), AFRC_MASK,	PPCCOM,		{ FRT, FRA, FRB } },
{ "fa",      A(63,21,0), AFRC_MASK,	PWRCOM,		{ FRT, FRA, FRB } },
{ "fadd.",   A(63,21,1), AFRC_MASK,	PPCCOM,		{ FRT, FRA, FRB } },
{ "fa.",     A(63,21,1), AFRC_MASK,	PWRCOM,		{ FRT, FRA, FRB } },

{ "fsqrt",   A(63,22,0), AFRAFRC_MASK,	PPCPWR2,	{ FRT, FRB } },
{ "fsqrt.",  A(63,22,1), AFRAFRC_MASK,	PPCPWR2,	{ FRT, FRB } },

{ "fsel",    A(63,23,0), A_MASK,	PPC,		{ FRT,FRA,FRC,FRB } },
{ "fsel.",   A(63,23,1), A_MASK,	PPC,		{ FRT,FRA,FRC,FRB } },

{ "fmul",    A(63,25,0), AFRB_MASK,	PPCCOM,		{ FRT, FRA, FRC } },
{ "fm",      A(63,25,0), AFRB_MASK,	PWRCOM,		{ FRT, FRA, FRC } },
{ "fmul.",   A(63,25,1), AFRB_MASK,	PPCCOM,		{ FRT, FRA, FRC } },
{ "fm.",     A(63,25,1), AFRB_MASK,	PWRCOM,		{ FRT, FRA, FRC } },

{ "frsqrte", A(63,26,0), AFRAFRC_MASK,	PPC,		{ FRT, FRB } },
{ "frsqrte.",A(63,26,1), AFRAFRC_MASK,	PPC,		{ FRT, FRB } },

{ "fmsub",   A(63,28,0), A_MASK,	PPCCOM,		{ FRT,FRA,FRC,FRB } },
{ "fms",     A(63,28,0), A_MASK,	PWRCOM,		{ FRT,FRA,FRC,FRB } },
{ "fmsub.",  A(63,28,1), A_MASK,	PPCCOM,		{ FRT,FRA,FRC,FRB } },
{ "fms.",    A(63,28,1), A_MASK,	PWRCOM,		{ FRT,FRA,FRC,FRB } },

{ "fmadd",   A(63,29,0), A_MASK,	PPCCOM,		{ FRT,FRA,FRC,FRB } },
{ "fma",     A(63,29,0), A_MASK,	PWRCOM,		{ FRT,FRA,FRC,FRB } },
{ "fmadd.",  A(63,29,1), A_MASK,	PPCCOM,		{ FRT,FRA,FRC,FRB } },
{ "fma.",    A(63,29,1), A_MASK,	PWRCOM,		{ FRT,FRA,FRC,FRB } },

{ "fnmsub",  A(63,30,0), A_MASK,	PPCCOM,		{ FRT,FRA,FRC,FRB } },
{ "fnms",    A(63,30,0), A_MASK,	PWRCOM,		{ FRT,FRA,FRC,FRB } },
{ "fnmsub.", A(63,30,1), A_MASK,	PPCCOM,		{ FRT,FRA,FRC,FRB } },
{ "fnms.",   A(63,30,1), A_MASK,	PWRCOM,		{ FRT,FRA,FRC,FRB } },

{ "fnmadd",  A(63,31,0), A_MASK,	PPCCOM,		{ FRT,FRA,FRC,FRB } },
{ "fnma",    A(63,31,0), A_MASK,	PWRCOM,		{ FRT,FRA,FRC,FRB } },
{ "fnmadd.", A(63,31,1), A_MASK,	PPCCOM,		{ FRT,FRA,FRC,FRB } },
{ "fnma.",   A(63,31,1), A_MASK,	PWRCOM,		{ FRT,FRA,FRC,FRB } },

{ "fcmpo",   X(63,32),	X_MASK|(3<<21),	COM,		{ BF, FRA, FRB } },

{ "mtfsb1",  XRC(63,38,0), XRARB_MASK,	COM,		{ BT } },
{ "mtfsb1.", XRC(63,38,1), XRARB_MASK,	COM,		{ BT } },

{ "fneg",    XRC(63,40,0), XRA_MASK,	COM,		{ FRT, FRB } },
{ "fneg.",   XRC(63,40,1), XRA_MASK,	COM,		{ FRT, FRB } },

{ "mcrfs",   X(63,64),	XRB_MASK|(3<<21)|(3<<16), COM,	{ BF, BFA } },

{ "mtfsb0",  XRC(63,70,0), XRARB_MASK,	COM,		{ BT } },
{ "mtfsb0.", XRC(63,70,1), XRARB_MASK,	COM,		{ BT } },

{ "fmr",     XRC(63,72,0), XRA_MASK,	COM,		{ FRT, FRB } },
{ "fmr.",    XRC(63,72,1), XRA_MASK,	COM,		{ FRT, FRB } },

{ "mtfsfi",  XRC(63,134,0), XRA_MASK|(3<<21)|(1<<11), COM, { BF, U } },
{ "mtfsfi.", XRC(63,134,1), XRA_MASK|(3<<21)|(1<<11), COM, { BF, U } },

{ "fnabs",   XRC(63,136,0), XRA_MASK,	COM,		{ FRT, FRB } },
{ "fnabs.",  XRC(63,136,1), XRA_MASK,	COM,		{ FRT, FRB } },

{ "fabs",    XRC(63,264,0), XRA_MASK,	COM,		{ FRT, FRB } },
{ "fabs.",   XRC(63,264,1), XRA_MASK,	COM,		{ FRT, FRB } },

{ "mffs",    XRC(63,583,0), XRARB_MASK,	COM,		{ FRT } },
{ "mffs.",   XRC(63,583,1), XRARB_MASK,	COM,		{ FRT } },

{ "mtfsf",   XFL(63,711,0), XFL_MASK,	COM,		{ FLM, FRB } },
{ "mtfsf.",  XFL(63,711,1), XFL_MASK,	COM,		{ FLM, FRB } },

{ "fctid",   XRC(63,814,0), XRA_MASK,	PPC64,		{ FRT, FRB } },
{ "fctid.",  XRC(63,814,1), XRA_MASK,	PPC64,		{ FRT, FRB } },

{ "fctidz",  XRC(63,815,0), XRA_MASK,	PPC64,		{ FRT, FRB } },
{ "fctidz.", XRC(63,815,1), XRA_MASK,	PPC64,		{ FRT, FRB } },

{ "fcfid",   XRC(63,846,0), XRA_MASK,	PPC64,		{ FRT, FRB } },
{ "fcfid.",  XRC(63,846,1), XRA_MASK,	PPC64,		{ FRT, FRB } },

};

const int powerpc_num_opcodes =
  sizeof (powerpc_opcodes) / sizeof (powerpc_opcodes[0]);

/* The macro table.  This is only used by the assembler.  */

/* The expressions of the form (-x ! 31) & (x | 31) have the value 0
   when x=0; 32-x when x is between 1 and 31; are negative if x is
   negative; and are 32 or more otherwise.  This is what you want
   when, for instance, you are emulating a right shift by a
   rotate-left-and-mask, because the underlying instructions support
   shifts of size 0 but not shifts of size 32.  By comparison, when
   extracting x bits from some word you want to use just 32-x, because
   the underlying instructions don't support extracting 0 bits but do
   support extracting the whole word (32 bits in this case).  */

const struct powerpc_macro powerpc_macros[] = {
{ "extldi",  4,   PPC64,	"rldicr %0,%1,%3,(%2)-1" },
{ "extldi.", 4,   PPC64,	"rldicr. %0,%1,%3,(%2)-1" },
{ "extrdi",  4,   PPC64,	"rldicl %0,%1,(%2)+(%3),64-(%2)" },
{ "extrdi.", 4,   PPC64,	"rldicl. %0,%1,(%2)+(%3),64-(%2)" },
{ "insrdi",  4,   PPC64,	"rldimi %0,%1,64-((%2)+(%3)),%3" },
{ "insrdi.", 4,   PPC64,	"rldimi. %0,%1,64-((%2)+(%3)),%3" },
{ "rotrdi",  3,   PPC64,	"rldicl %0,%1,(-(%2)!63)&((%2)|63),0" },
{ "rotrdi.", 3,   PPC64,	"rldicl. %0,%1,(-(%2)!63)&((%2)|63),0" },
{ "sldi",    3,   PPC64,	"rldicr %0,%1,%2,63-(%2)" },
{ "sldi.",   3,   PPC64,	"rldicr. %0,%1,%2,63-(%2)" },
{ "srdi",    3,   PPC64,	"rldicl %0,%1,(-(%2)!63)&((%2)|63),%2" },
{ "srdi.",   3,   PPC64,	"rldicl. %0,%1,(-(%2)!63)&((%2)|63),%2" },
{ "clrrdi",  3,   PPC64,	"rldicr %0,%1,0,63-(%2)" },
{ "clrrdi.", 3,   PPC64,	"rldicr. %0,%1,0,63-(%2)" },
{ "clrlsldi",4,   PPC64,	"rldic %0,%1,%3,(%2)-(%3)" },
{ "clrlsldi.",4,  PPC64,	"rldic. %0,%1,%3,(%2)-(%3)" },

{ "extlwi",  4,   PPCCOM,	"rlwinm %0,%1,%3,0,(%2)-1" },
{ "extlwi.", 4,   PPCCOM,	"rlwinm. %0,%1,%3,0,(%2)-1" },
{ "extrwi",  4,   PPCCOM,	"rlwinm %0,%1,((%2)+(%3))&((%2)+(%3)<>32),32-(%2),31" },
{ "extrwi.", 4,   PPCCOM,	"rlwinm. %0,%1,((%2)+(%3))&((%2)+(%3)<>32),32-(%2),31" },
{ "inslwi",  4,   PPCCOM,	"rlwimi %0,%1,(-(%3)!31)&((%3)|31),%3,(%2)+(%3)-1" },
{ "inslwi.", 4,   PPCCOM,	"rlwimi. %0,%1,(-(%3)!31)&((%3)|31),%3,(%2)+(%3)-1"},
{ "insrwi",  4,   PPCCOM,	"rlwimi %0,%1,32-((%2)+(%3)),%3,(%2)+(%3)-1" },
{ "insrwi.", 4,   PPCCOM,	"rlwimi. %0,%1,32-((%2)+(%3)),%3,(%2)+(%3)-1"},
{ "rotrwi",  3,   PPCCOM,	"rlwinm %0,%1,(-(%2)!31)&((%2)|31),0,31" },
{ "rotrwi.", 3,   PPCCOM,	"rlwinm. %0,%1,(-(%2)!31)&((%2)|31),0,31" },
{ "slwi",    3,   PPCCOM,	"rlwinm %0,%1,%2,0,31-(%2)" },
{ "sli",     3,   PWRCOM,	"rlinm %0,%1,%2,0,31-(%2)" },
{ "slwi.",   3,   PPCCOM,	"rlwinm. %0,%1,%2,0,31-(%2)" },
{ "sli.",    3,   PWRCOM,	"rlinm. %0,%1,%2,0,31-(%2)" },
{ "srwi",    3,   PPCCOM,	"rlwinm %0,%1,(-(%2)!31)&((%2)|31),%2,31" },
{ "sri",     3,   PWRCOM,	"rlinm %0,%1,(-(%2)!31)&((%2)|31),%2,31" },
{ "srwi.",   3,   PPCCOM,	"rlwinm. %0,%1,(-(%2)!31)&((%2)|31),%2,31" },
{ "sri.",    3,   PWRCOM,	"rlinm. %0,%1,(-(%2)!31)&((%2)|31),%2,31" },
{ "clrrwi",  3,   PPCCOM,	"rlwinm %0,%1,0,0,31-(%2)" },
{ "clrrwi.", 3,   PPCCOM,	"rlwinm. %0,%1,0,0,31-(%2)" },
{ "clrlslwi",4,   PPCCOM,	"rlwinm %0,%1,%3,(%2)-(%3),31-(%3)" },
{ "clrlslwi.",4,  PPCCOM,	"rlwinm. %0,%1,%3,(%2)-(%3),31-(%3)" },
};

const int powerpc_num_macros =
  sizeof (powerpc_macros) / sizeof (powerpc_macros[0]);

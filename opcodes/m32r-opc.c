/* Generic opcode table support for targets using CGEN. -*- C -*-
   CGEN: Cpu tools GENerator

This file is used to generate m32r-opc.c.

Copyright (C) 1998 Free Software Foundation, Inc.

This file is part of the GNU Binutils and GDB, the GNU debugger.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#include "sysdep.h"
#include <stdio.h>
#include "ansidecl.h"
#include "libiberty.h"
#include "bfd.h"
#include "symcat.h"
#include "m32r-opc.h"

/* Look up instruction INSN_VALUE and extract its fields.
   If non-null INSN is the insn table entry.
   Otherwise INSN_VALUE is examined to compute it.
   LENGTH is the bit length of INSN_VALUE if known, otherwise 0.
   ALIAS_P is non-zero if alias insns are to be included in the search.
   The result a pointer to the insn table entry, or NULL if the instruction
   wasn't recognized.  */

const CGEN_INSN *
m32r_cgen_lookup_insn (insn, insn_value, length, fields, alias_p)
     const CGEN_INSN *insn;
     cgen_insn_t insn_value;
     int length;
     CGEN_FIELDS *fields;
{
  char buf[16];

  if (!insn)
    {
      const CGEN_INSN_LIST *insn_list;

#ifdef CGEN_INT_INSN
      switch (length)
	{
	case 8:
	  buf[0] = insn_value;
	  break;
	case 16:
	  if (cgen_current_endian == CGEN_ENDIAN_BIG)
	    bfd_putb16 (insn_value, buf);
	  else
	    bfd_putl16 (insn_value, buf);
	  break;
	case 32:
	  if (cgen_current_endian == CGEN_ENDIAN_BIG)
	    bfd_putb32 (insn_value, buf);
	  else
	    bfd_putl32 (insn_value, buf);
	  break;
	default:
	  abort ();
	}
#else
      abort (); /* FIXME: unfinished */
#endif

      /* The instructions are stored in hash lists.
	 Pick the first one and keep trying until we find the right one.  */

      insn_list = CGEN_DIS_LOOKUP_INSN (buf, insn_value);
      while (insn_list != NULL)
	{
	  insn = insn_list->insn;

	  if (alias_p
	      || ! CGEN_INSN_ATTR (insn, CGEN_INSN_ALIAS))
	    {
	      /* Basic bit mask must be correct.  */
	      /* ??? May wish to allow target to defer this check until the
		 extract handler.  */
	      if ((insn_value & CGEN_INSN_MASK (insn)) == CGEN_INSN_VALUE (insn))
		{
		  length = (*CGEN_EXTRACT_FN (insn)) (insn, NULL, insn_value, fields);
		  if (length > 0)
		    return insn;
		}
	    }

	  insn_list = CGEN_DIS_NEXT_INSN (insn_list);
	}
    }
  else
    {
      /* Sanity check: can't pass an alias insn if ! alias_p.  */
      if (! alias_p
	  && CGEN_INSN_ATTR (insn, CGEN_INSN_ALIAS))
	abort ();

      length = (*CGEN_EXTRACT_FN (insn)) (insn, NULL, insn_value, fields);
      if (length > 0)
	return insn;
    }

  return NULL;
}

/* Fill in the operand instances used by insn INSN_VALUE.
   If non-null INS is the insn table entry.
   Otherwise INSN_VALUE is examined to compute it.
   LENGTH is the number of bits in INSN_VALUE if known, otherwise 0.
   INDICES is a pointer to a buffer of MAX_OPERANDS ints to be filled in.
   The result a pointer to the insn table entry, or NULL if the instruction
   wasn't recognized.  */

const CGEN_INSN *
m32r_cgen_get_insn_operands (insn, insn_value, length, indices)
     const CGEN_INSN *insn;
     cgen_insn_t insn_value;
     int length;
     int *indices;
{
  CGEN_FIELDS fields;
  const CGEN_OPERAND_INSTANCE *opinst;
  int i;

  /* FIXME: ALIAS insns are in transition from being record in the insn table
     to being recorded separately as macros.  They don't have semantic code
     so they can't be used here.  Thus we currently always ignore the INSN
     argument.  */
  insn = m32r_cgen_lookup_insn (NULL, insn_value, length, &fields, 0);
  if (! insn)
    return NULL;

  for (i = 0, opinst = CGEN_INSN_OPERANDS (insn);
       opinst != NULL
	 && CGEN_OPERAND_INSTANCE_TYPE (opinst) != CGEN_OPERAND_INSTANCE_END;
       ++i, ++opinst)
    {
      const CGEN_OPERAND *op = CGEN_OPERAND_INSTANCE_OPERAND (opinst);
      if (op == NULL)
	indices[i] = CGEN_OPERAND_INSTANCE_INDEX (opinst);
      else
	indices[i] = m32r_cgen_get_operand (CGEN_OPERAND_INDEX (op), &fields);
    }

  return insn;
}
/* Attributes.  */

static const CGEN_ATTR_ENTRY MACH_attr[] =
{
  { "m32r", MACH_M32R },
/* start-sanitize-m32rx */
  { "m32rx", MACH_M32RX },
/* end-sanitize-m32rx */
  { "max", MACH_MAX },
  { 0, 0 }
};

/* start-sanitize-m32rx */
static const CGEN_ATTR_ENTRY PIPE_attr[] =
{
  { "NONE", PIPE_NONE },
  { "O", PIPE_O },
  { "S", PIPE_S },
  { "OS", PIPE_OS },
  { 0, 0 }
};

/* end-sanitize-m32rx */
const CGEN_ATTR_TABLE m32r_cgen_operand_attr_table[] =
{
  { "ABS-ADDR", NULL },
  { "FAKE", NULL },
  { "NEGATIVE", NULL },
  { "PC", NULL },
  { "PCREL-ADDR", NULL },
  { "RELAX", NULL },
  { "RELOC", NULL },
  { "SIGN-OPT", NULL },
  { "UNSIGNED", NULL },
  { 0, 0 }
};

const CGEN_ATTR_TABLE m32r_cgen_insn_attr_table[] =
{
  { "MACH", & MACH_attr[0] },
/* start-sanitize-m32rx */
  { "PIPE", & PIPE_attr[0] },
/* end-sanitize-m32rx */
  { "ALIAS", NULL },
  { "COND-CTI", NULL },
  { "FILL-SLOT", NULL },
  { "PARALLEL", NULL },
  { "RELAX", NULL },
  { "RELAXABLE", NULL },
  { "UNCOND-CTI", NULL },
  { 0, 0 }
};

CGEN_KEYWORD_ENTRY m32r_cgen_opval_h_gr_entries[] = 
{
  { "fp", 13 },
  { "lr", 14 },
  { "sp", 15 },
  { "r0", 0 },
  { "r1", 1 },
  { "r2", 2 },
  { "r3", 3 },
  { "r4", 4 },
  { "r5", 5 },
  { "r6", 6 },
  { "r7", 7 },
  { "r8", 8 },
  { "r9", 9 },
  { "r10", 10 },
  { "r11", 11 },
  { "r12", 12 },
  { "r13", 13 },
  { "r14", 14 },
  { "r15", 15 }
};

CGEN_KEYWORD m32r_cgen_opval_h_gr = 
{
  & m32r_cgen_opval_h_gr_entries[0],
  19
};

CGEN_KEYWORD_ENTRY m32r_cgen_opval_h_cr_entries[] = 
{
  { "psw", 0 },
  { "cbr", 1 },
  { "spi", 2 },
  { "spu", 3 },
  { "bpc", 6 },
  { "cr0", 0 },
  { "cr1", 1 },
  { "cr2", 2 },
  { "cr3", 3 },
  { "cr4", 4 },
  { "cr5", 5 },
  { "cr6", 6 }
};

CGEN_KEYWORD m32r_cgen_opval_h_cr = 
{
  & m32r_cgen_opval_h_cr_entries[0],
  12
};

/* start-sanitize-m32rx */
CGEN_KEYWORD_ENTRY m32r_cgen_opval_h_accums_entries[] = 
{
  { "a0", 0 },
  { "a1", 1 }
};

CGEN_KEYWORD m32r_cgen_opval_h_accums = 
{
  & m32r_cgen_opval_h_accums_entries[0],
  2
};

/* end-sanitize-m32rx */

/* The hardware table.  */

#define HW_ENT(n) m32r_cgen_hw_entries[n]
static const CGEN_HW_ENTRY m32r_cgen_hw_entries[] =
{
  { HW_H_PC, & HW_ENT (HW_H_PC + 1), "h-pc", CGEN_ASM_KEYWORD, (PTR) 0 },
  { HW_H_MEMORY, & HW_ENT (HW_H_MEMORY + 1), "h-memory", CGEN_ASM_KEYWORD, (PTR) 0 },
  { HW_H_SINT, & HW_ENT (HW_H_SINT + 1), "h-sint", CGEN_ASM_KEYWORD, (PTR) 0 },
  { HW_H_UINT, & HW_ENT (HW_H_UINT + 1), "h-uint", CGEN_ASM_KEYWORD, (PTR) 0 },
  { HW_H_ADDR, & HW_ENT (HW_H_ADDR + 1), "h-addr", CGEN_ASM_KEYWORD, (PTR) 0 },
  { HW_H_IADDR, & HW_ENT (HW_H_IADDR + 1), "h-iaddr", CGEN_ASM_KEYWORD, (PTR) 0 },
  { HW_H_HI16, & HW_ENT (HW_H_HI16 + 1), "h-hi16", CGEN_ASM_KEYWORD, (PTR) 0 },
  { HW_H_SLO16, & HW_ENT (HW_H_SLO16 + 1), "h-slo16", CGEN_ASM_KEYWORD, (PTR) 0 },
  { HW_H_ULO16, & HW_ENT (HW_H_ULO16 + 1), "h-ulo16", CGEN_ASM_KEYWORD, (PTR) 0 },
  { HW_H_GR, & HW_ENT (HW_H_GR + 1), "h-gr", CGEN_ASM_KEYWORD, (PTR) & m32r_cgen_opval_h_gr },
  { HW_H_CR, & HW_ENT (HW_H_CR + 1), "h-cr", CGEN_ASM_KEYWORD, (PTR) & m32r_cgen_opval_h_cr },
  { HW_H_ACCUM, & HW_ENT (HW_H_ACCUM + 1), "h-accum", CGEN_ASM_KEYWORD, (PTR) 0 },
/* start-sanitize-m32rx */
  { HW_H_ACCUMS, & HW_ENT (HW_H_ACCUMS + 1), "h-accums", CGEN_ASM_KEYWORD, (PTR) & m32r_cgen_opval_h_accums },
/* end-sanitize-m32rx */
/* start-sanitize-m32rx */
  { HW_H_ABORT, & HW_ENT (HW_H_ABORT + 1), "h-abort", CGEN_ASM_KEYWORD, (PTR) 0 },
/* end-sanitize-m32rx */
  { HW_H_COND, & HW_ENT (HW_H_COND + 1), "h-cond", CGEN_ASM_KEYWORD, (PTR) 0 },
  { HW_H_SM, & HW_ENT (HW_H_SM + 1), "h-sm", CGEN_ASM_KEYWORD, (PTR) 0 },
  { HW_H_BSM, & HW_ENT (HW_H_BSM + 1), "h-bsm", CGEN_ASM_KEYWORD, (PTR) 0 },
  { HW_H_IE, & HW_ENT (HW_H_IE + 1), "h-ie", CGEN_ASM_KEYWORD, (PTR) 0 },
  { HW_H_BIE, & HW_ENT (HW_H_BIE + 1), "h-bie", CGEN_ASM_KEYWORD, (PTR) 0 },
  { HW_H_BCOND, & HW_ENT (HW_H_BCOND + 1), "h-bcond", CGEN_ASM_KEYWORD, (PTR) 0 },
  { HW_H_BPC, & HW_ENT (HW_H_BPC + 1), "h-bpc", CGEN_ASM_KEYWORD, (PTR) 0 },
  { 0 }
};

/* The operand table.  */

#define OP_ENT(op) m32r_cgen_operand_table[CONCAT2 (M32R_OPERAND_,op)]
const CGEN_OPERAND m32r_cgen_operand_table[MAX_OPERANDS] =
{
/* pc: program counter */
  { "pc", & HW_ENT (HW_H_PC), 0, 0,
    { 0, 0|(1<<CGEN_OPERAND_FAKE)|(1<<CGEN_OPERAND_PC), { 0 } }  },
/* sr: source register */
  { "sr", & HW_ENT (HW_H_GR), 12, 4,
    { 0, 0|(1<<CGEN_OPERAND_UNSIGNED), { 0 } }  },
/* dr: destination register */
  { "dr", & HW_ENT (HW_H_GR), 4, 4,
    { 0, 0|(1<<CGEN_OPERAND_UNSIGNED), { 0 } }  },
/* src1: source register 1 */
  { "src1", & HW_ENT (HW_H_GR), 4, 4,
    { 0, 0|(1<<CGEN_OPERAND_UNSIGNED), { 0 } }  },
/* src2: source register 2 */
  { "src2", & HW_ENT (HW_H_GR), 12, 4,
    { 0, 0|(1<<CGEN_OPERAND_UNSIGNED), { 0 } }  },
/* scr: source control register */
  { "scr", & HW_ENT (HW_H_CR), 12, 4,
    { 0, 0|(1<<CGEN_OPERAND_UNSIGNED), { 0 } }  },
/* dcr: destination control register */
  { "dcr", & HW_ENT (HW_H_CR), 4, 4,
    { 0, 0|(1<<CGEN_OPERAND_UNSIGNED), { 0 } }  },
/* simm8: 8 bit signed immediate */
  { "simm8", & HW_ENT (HW_H_SINT), 8, 8,
    { 0, 0, { 0 } }  },
/* simm16: 16 bit signed immediate */
  { "simm16", & HW_ENT (HW_H_SINT), 16, 16,
    { 0, 0, { 0 } }  },
/* uimm4: 4 bit trap number */
  { "uimm4", & HW_ENT (HW_H_UINT), 12, 4,
    { 0, 0|(1<<CGEN_OPERAND_UNSIGNED), { 0 } }  },
/* uimm5: 5 bit shift count */
  { "uimm5", & HW_ENT (HW_H_UINT), 11, 5,
    { 0, 0|(1<<CGEN_OPERAND_UNSIGNED), { 0 } }  },
/* uimm16: 16 bit unsigned immediate */
  { "uimm16", & HW_ENT (HW_H_UINT), 16, 16,
    { 0, 0|(1<<CGEN_OPERAND_UNSIGNED), { 0 } }  },
/* start-sanitize-m32rx */
/* imm1: 1 bit immediate */
  { "imm1", & HW_ENT (HW_H_UINT), 15, 1,
    { 0, 0|(1<<CGEN_OPERAND_UNSIGNED), { 0 } }  },
/* end-sanitize-m32rx */
/* start-sanitize-m32rx */
/* accd: accumulator destination register */
  { "accd", & HW_ENT (HW_H_ACCUMS), 4, 2,
    { 0, 0|(1<<CGEN_OPERAND_UNSIGNED), { 0 } }  },
/* end-sanitize-m32rx */
/* start-sanitize-m32rx */
/* accs: accumulator source register */
  { "accs", & HW_ENT (HW_H_ACCUMS), 12, 2,
    { 0, 0|(1<<CGEN_OPERAND_UNSIGNED), { 0 } }  },
/* end-sanitize-m32rx */
/* start-sanitize-m32rx */
/* acc: accumulator reg (d) */
  { "acc", & HW_ENT (HW_H_ACCUMS), 8, 1,
    { 0, 0|(1<<CGEN_OPERAND_UNSIGNED), { 0 } }  },
/* end-sanitize-m32rx */
/* hi16: high 16 bit immediate, sign optional */
  { "hi16", & HW_ENT (HW_H_HI16), 16, 16,
    { 0, 0|(1<<CGEN_OPERAND_SIGN_OPT)|(1<<CGEN_OPERAND_UNSIGNED), { 0 } }  },
/* slo16: 16 bit signed immediate, for low() */
  { "slo16", & HW_ENT (HW_H_SLO16), 16, 16,
    { 0, 0, { 0 } }  },
/* ulo16: 16 bit unsigned immediate, for low() */
  { "ulo16", & HW_ENT (HW_H_ULO16), 16, 16,
    { 0, 0|(1<<CGEN_OPERAND_UNSIGNED), { 0 } }  },
/* uimm24: 24 bit address */
  { "uimm24", & HW_ENT (HW_H_ADDR), 8, 24,
    { 0, 0|(1<<CGEN_OPERAND_RELOC)|(1<<CGEN_OPERAND_ABS_ADDR)|(1<<CGEN_OPERAND_UNSIGNED), { 0 } }  },
/* disp8: 8 bit displacement */
  { "disp8", & HW_ENT (HW_H_IADDR), 8, 8,
    { 0, 0|(1<<CGEN_OPERAND_RELAX)|(1<<CGEN_OPERAND_RELOC)|(1<<CGEN_OPERAND_PCREL_ADDR), { 0 } }  },
/* disp16: 16 bit displacement */
  { "disp16", & HW_ENT (HW_H_IADDR), 16, 16,
    { 0, 0|(1<<CGEN_OPERAND_RELOC)|(1<<CGEN_OPERAND_PCREL_ADDR), { 0 } }  },
/* disp24: 24 bit displacement */
  { "disp24", & HW_ENT (HW_H_IADDR), 8, 24,
    { 0, 0|(1<<CGEN_OPERAND_RELAX)|(1<<CGEN_OPERAND_RELOC)|(1<<CGEN_OPERAND_PCREL_ADDR), { 0 } }  },
/* condbit: condition bit */
  { "condbit", & HW_ENT (HW_H_COND), 0, 0,
    { 0, 0|(1<<CGEN_OPERAND_FAKE), { 0 } }  },
/* accum: accumulator */
  { "accum", & HW_ENT (HW_H_ACCUM), 0, 0,
    { 0, 0|(1<<CGEN_OPERAND_FAKE), { 0 } }  },
/* start-sanitize-m32rx */
/* abort-parallel-execution: abort parallel execution */
  { "abort-parallel-execution", & HW_ENT (HW_H_ABORT), 0, 0,
    { 0, 0|(1<<CGEN_OPERAND_FAKE), { 0 } }  },
/* end-sanitize-m32rx */
};

/* Operand references.  */

#define INPUT CGEN_OPERAND_INSTANCE_INPUT
#define OUTPUT CGEN_OPERAND_INSTANCE_OUTPUT

static const CGEN_OPERAND_INSTANCE fmt_0_add_ops[] = {
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (DR), 0 },
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SR), 0 },
  { OUTPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (DR), 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_1_add3_ops[] = {
  { INPUT, & HW_ENT (HW_H_SLO16), CGEN_MODE_HI, & OP_ENT (SLO16), 0 },
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SR), 0 },
  { OUTPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (DR), 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_2_and3_ops[] = {
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SR), 0 },
  { INPUT, & HW_ENT (HW_H_UINT), CGEN_MODE_USI, & OP_ENT (UIMM16), 0 },
  { OUTPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (DR), 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_3_or3_ops[] = {
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SR), 0 },
  { INPUT, & HW_ENT (HW_H_ULO16), CGEN_MODE_UHI, & OP_ENT (ULO16), 0 },
  { OUTPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (DR), 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_4_addi_ops[] = {
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (DR), 0 },
  { INPUT, & HW_ENT (HW_H_SINT), CGEN_MODE_SI, & OP_ENT (SIMM8), 0 },
  { OUTPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (DR), 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_5_addv_ops[] = {
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (DR), 0 },
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SR), 0 },
  { OUTPUT, & HW_ENT (HW_H_COND), CGEN_MODE_UBI, 0, 0 },
  { OUTPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (DR), 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_6_addv3_ops[] = {
  { INPUT, & HW_ENT (HW_H_SINT), CGEN_MODE_SI, & OP_ENT (SIMM16), 0 },
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SR), 0 },
  { OUTPUT, & HW_ENT (HW_H_COND), CGEN_MODE_UBI, 0, 0 },
  { OUTPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (DR), 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_7_addx_ops[] = {
  { INPUT, & HW_ENT (HW_H_COND), CGEN_MODE_UBI, 0, 0 },
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (DR), 0 },
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SR), 0 },
  { OUTPUT, & HW_ENT (HW_H_COND), CGEN_MODE_UBI, 0, 0 },
  { OUTPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (DR), 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_8_bc8_ops[] = {
  { INPUT, & HW_ENT (HW_H_COND), CGEN_MODE_UBI, 0, 0 },
  { INPUT, & HW_ENT (HW_H_IADDR), CGEN_MODE_VM, & OP_ENT (DISP8), 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_10_bc24_ops[] = {
  { INPUT, & HW_ENT (HW_H_COND), CGEN_MODE_UBI, 0, 0 },
  { INPUT, & HW_ENT (HW_H_IADDR), CGEN_MODE_VM, & OP_ENT (DISP24), 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_12_beq_ops[] = {
  { INPUT, & HW_ENT (HW_H_IADDR), CGEN_MODE_VM, & OP_ENT (DISP16), 0 },
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SRC1), 0 },
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SRC2), 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_13_beqz_ops[] = {
  { INPUT, & HW_ENT (HW_H_IADDR), CGEN_MODE_VM, & OP_ENT (DISP16), 0 },
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SRC2), 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_14_bl8_ops[] = {
  { INPUT, & HW_ENT (HW_H_IADDR), CGEN_MODE_VM, & OP_ENT (DISP8), 0 },
  { INPUT, & HW_ENT (HW_H_PC), CGEN_MODE_USI, 0, 0 },
  { OUTPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, 0, 14 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_15_bl24_ops[] = {
  { INPUT, & HW_ENT (HW_H_IADDR), CGEN_MODE_VM, & OP_ENT (DISP24), 0 },
  { INPUT, & HW_ENT (HW_H_PC), CGEN_MODE_USI, 0, 0 },
  { OUTPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, 0, 14 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_16_bcl8_ops[] = {
  { INPUT, & HW_ENT (HW_H_COND), CGEN_MODE_UBI, 0, 0 },
  { INPUT, & HW_ENT (HW_H_IADDR), CGEN_MODE_VM, & OP_ENT (DISP8), 0 },
  { INPUT, & HW_ENT (HW_H_PC), CGEN_MODE_USI, 0, 0 },
  { OUTPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, 0, 14 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_17_bcl24_ops[] = {
  { INPUT, & HW_ENT (HW_H_COND), CGEN_MODE_UBI, 0, 0 },
  { INPUT, & HW_ENT (HW_H_IADDR), CGEN_MODE_VM, & OP_ENT (DISP24), 0 },
  { INPUT, & HW_ENT (HW_H_PC), CGEN_MODE_USI, 0, 0 },
  { OUTPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, 0, 14 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_18_bra8_ops[] = {
  { INPUT, & HW_ENT (HW_H_IADDR), CGEN_MODE_VM, & OP_ENT (DISP8), 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_19_bra24_ops[] = {
  { INPUT, & HW_ENT (HW_H_IADDR), CGEN_MODE_VM, & OP_ENT (DISP24), 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_20_cmp_ops[] = {
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SRC1), 0 },
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SRC2), 0 },
  { OUTPUT, & HW_ENT (HW_H_COND), CGEN_MODE_UBI, 0, 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_21_cmpi_ops[] = {
  { INPUT, & HW_ENT (HW_H_SINT), CGEN_MODE_SI, & OP_ENT (SIMM16), 0 },
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SRC2), 0 },
  { OUTPUT, & HW_ENT (HW_H_COND), CGEN_MODE_UBI, 0, 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_22_cmpui_ops[] = {
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SRC2), 0 },
  { INPUT, & HW_ENT (HW_H_UINT), CGEN_MODE_USI, & OP_ENT (UIMM16), 0 },
  { OUTPUT, & HW_ENT (HW_H_COND), CGEN_MODE_UBI, 0, 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_23_cmpz_ops[] = {
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SRC2), 0 },
  { OUTPUT, & HW_ENT (HW_H_COND), CGEN_MODE_UBI, 0, 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_24_div_ops[] = {
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (DR), 0 },
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SR), 0 },
  { OUTPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (DR), 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_25_jc_ops[] = {
  { INPUT, & HW_ENT (HW_H_COND), CGEN_MODE_UBI, 0, 0 },
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SR), 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_26_jl_ops[] = {
  { INPUT, & HW_ENT (HW_H_PC), CGEN_MODE_USI, 0, 0 },
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SR), 0 },
  { OUTPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, 0, 14 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_27_jmp_ops[] = {
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SR), 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_28_ld_ops[] = {
  { INPUT, & HW_ENT (HW_H_MEMORY), CGEN_MODE_SI, 0, 0 },
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SR), 0 },
  { OUTPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (DR), 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_30_ld_d_ops[] = {
  { INPUT, & HW_ENT (HW_H_MEMORY), CGEN_MODE_SI, 0, 0 },
  { INPUT, & HW_ENT (HW_H_SLO16), CGEN_MODE_HI, & OP_ENT (SLO16), 0 },
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SR), 0 },
  { OUTPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (DR), 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_32_ldb_ops[] = {
  { INPUT, & HW_ENT (HW_H_MEMORY), CGEN_MODE_QI, 0, 0 },
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SR), 0 },
  { OUTPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (DR), 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_33_ldb_d_ops[] = {
  { INPUT, & HW_ENT (HW_H_MEMORY), CGEN_MODE_QI, 0, 0 },
  { INPUT, & HW_ENT (HW_H_SLO16), CGEN_MODE_HI, & OP_ENT (SLO16), 0 },
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SR), 0 },
  { OUTPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (DR), 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_34_ldh_ops[] = {
  { INPUT, & HW_ENT (HW_H_MEMORY), CGEN_MODE_HI, 0, 0 },
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SR), 0 },
  { OUTPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (DR), 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_35_ldh_d_ops[] = {
  { INPUT, & HW_ENT (HW_H_MEMORY), CGEN_MODE_HI, 0, 0 },
  { INPUT, & HW_ENT (HW_H_SLO16), CGEN_MODE_HI, & OP_ENT (SLO16), 0 },
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SR), 0 },
  { OUTPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (DR), 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_36_ld_plus_ops[] = {
  { INPUT, & HW_ENT (HW_H_MEMORY), CGEN_MODE_SI, 0, 0 },
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SR), 0 },
  { OUTPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (DR), 0 },
  { OUTPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SR), 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_37_ld24_ops[] = {
  { INPUT, & HW_ENT (HW_H_ADDR), CGEN_MODE_VM, & OP_ENT (UIMM24), 0 },
  { OUTPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (DR), 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_38_ldi8_ops[] = {
  { INPUT, & HW_ENT (HW_H_SINT), CGEN_MODE_SI, & OP_ENT (SIMM8), 0 },
  { OUTPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (DR), 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_39_ldi16_ops[] = {
  { INPUT, & HW_ENT (HW_H_SLO16), CGEN_MODE_HI, & OP_ENT (SLO16), 0 },
  { OUTPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (DR), 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_40_lock_ops[] = {
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (DR), 0 },
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SR), 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_41_machi_ops[] = {
  { INPUT, & HW_ENT (HW_H_ACCUM), CGEN_MODE_DI, 0, 0 },
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SRC1), 0 },
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SRC2), 0 },
  { OUTPUT, & HW_ENT (HW_H_ACCUM), CGEN_MODE_DI, 0, 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_42_machi_a_ops[] = {
  { INPUT, & HW_ENT (HW_H_ACCUMS), CGEN_MODE_DI, & OP_ENT (ACC), 0 },
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SRC1), 0 },
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SRC2), 0 },
  { OUTPUT, & HW_ENT (HW_H_ACCUMS), CGEN_MODE_DI, & OP_ENT (ACC), 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_43_mulhi_ops[] = {
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SRC1), 0 },
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SRC2), 0 },
  { OUTPUT, & HW_ENT (HW_H_ACCUM), CGEN_MODE_DI, 0, 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_44_mulhi_a_ops[] = {
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SRC1), 0 },
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SRC2), 0 },
  { OUTPUT, & HW_ENT (HW_H_ACCUMS), CGEN_MODE_DI, & OP_ENT (ACC), 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_45_mv_ops[] = {
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SR), 0 },
  { OUTPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (DR), 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_46_mvfachi_ops[] = {
  { INPUT, & HW_ENT (HW_H_ACCUM), CGEN_MODE_DI, 0, 0 },
  { OUTPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (DR), 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_47_mvfachi_a_ops[] = {
  { INPUT, & HW_ENT (HW_H_ACCUMS), CGEN_MODE_DI, & OP_ENT (ACCS), 0 },
  { OUTPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (DR), 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_48_mvfc_ops[] = {
  { INPUT, & HW_ENT (HW_H_CR), CGEN_MODE_SI, & OP_ENT (SCR), 0 },
  { OUTPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (DR), 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_49_mvtachi_ops[] = {
  { INPUT, & HW_ENT (HW_H_ACCUM), CGEN_MODE_DI, 0, 0 },
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SRC1), 0 },
  { OUTPUT, & HW_ENT (HW_H_ACCUM), CGEN_MODE_DI, 0, 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_50_mvtachi_a_ops[] = {
  { INPUT, & HW_ENT (HW_H_ACCUMS), CGEN_MODE_DI, & OP_ENT (ACCS), 0 },
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SRC1), 0 },
  { OUTPUT, & HW_ENT (HW_H_ACCUMS), CGEN_MODE_DI, & OP_ENT (ACCS), 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_51_mvtc_ops[] = {
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SR), 0 },
  { OUTPUT, & HW_ENT (HW_H_CR), CGEN_MODE_SI, & OP_ENT (DCR), 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_53_rac_ops[] = {
  { INPUT, & HW_ENT (HW_H_ACCUM), CGEN_MODE_DI, 0, 0 },
  { OUTPUT, & HW_ENT (HW_H_ACCUM), CGEN_MODE_DI, 0, 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_54_rac_d_ops[] = {
  { INPUT, & HW_ENT (HW_H_ACCUM), CGEN_MODE_DI, 0, 0 },
  { OUTPUT, & HW_ENT (HW_H_ACCUMS), CGEN_MODE_DI, & OP_ENT (ACCD), 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_55_rac_ds_ops[] = {
  { INPUT, & HW_ENT (HW_H_ACCUMS), CGEN_MODE_DI, & OP_ENT (ACCS), 0 },
  { OUTPUT, & HW_ENT (HW_H_ACCUMS), CGEN_MODE_DI, & OP_ENT (ACCD), 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_56_rac_dsi_ops[] = {
  { INPUT, & HW_ENT (HW_H_ACCUMS), CGEN_MODE_DI, & OP_ENT (ACCS), 0 },
  { INPUT, & HW_ENT (HW_H_UINT), CGEN_MODE_USI, & OP_ENT (IMM1), 0 },
  { OUTPUT, & HW_ENT (HW_H_ACCUMS), CGEN_MODE_DI, & OP_ENT (ACCD), 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_57_rte_ops[] = {
  { INPUT, & HW_ENT (HW_H_BCOND), CGEN_MODE_VM, 0, 0 },
  { INPUT, & HW_ENT (HW_H_BIE), CGEN_MODE_VM, 0, 0 },
  { INPUT, & HW_ENT (HW_H_BPC), CGEN_MODE_VM, 0, 0 },
  { INPUT, & HW_ENT (HW_H_BSM), CGEN_MODE_VM, 0, 0 },
  { OUTPUT, & HW_ENT (HW_H_COND), CGEN_MODE_UBI, 0, 0 },
  { OUTPUT, & HW_ENT (HW_H_PC), CGEN_MODE_USI, 0, 0 },
  { OUTPUT, & HW_ENT (HW_H_IE), CGEN_MODE_VM, 0, 0 },
  { OUTPUT, & HW_ENT (HW_H_SM), CGEN_MODE_VM, 0, 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_58_seth_ops[] = {
  { INPUT, & HW_ENT (HW_H_HI16), CGEN_MODE_UHI, & OP_ENT (HI16), 0 },
  { OUTPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (DR), 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_59_sll3_ops[] = {
  { INPUT, & HW_ENT (HW_H_SINT), CGEN_MODE_SI, & OP_ENT (SIMM16), 0 },
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SR), 0 },
  { OUTPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (DR), 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_60_slli_ops[] = {
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (DR), 0 },
  { INPUT, & HW_ENT (HW_H_UINT), CGEN_MODE_USI, & OP_ENT (UIMM5), 0 },
  { OUTPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (DR), 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_61_st_ops[] = {
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SRC1), 0 },
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SRC2), 0 },
  { OUTPUT, & HW_ENT (HW_H_MEMORY), CGEN_MODE_SI, 0, 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_63_st_d_ops[] = {
  { INPUT, & HW_ENT (HW_H_SLO16), CGEN_MODE_HI, & OP_ENT (SLO16), 0 },
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SRC1), 0 },
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SRC2), 0 },
  { OUTPUT, & HW_ENT (HW_H_MEMORY), CGEN_MODE_SI, 0, 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_65_stb_ops[] = {
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SRC1), 0 },
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SRC2), 0 },
  { OUTPUT, & HW_ENT (HW_H_MEMORY), CGEN_MODE_QI, 0, 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_66_stb_d_ops[] = {
  { INPUT, & HW_ENT (HW_H_SLO16), CGEN_MODE_HI, & OP_ENT (SLO16), 0 },
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SRC1), 0 },
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SRC2), 0 },
  { OUTPUT, & HW_ENT (HW_H_MEMORY), CGEN_MODE_QI, 0, 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_67_sth_ops[] = {
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SRC1), 0 },
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SRC2), 0 },
  { OUTPUT, & HW_ENT (HW_H_MEMORY), CGEN_MODE_HI, 0, 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_68_sth_d_ops[] = {
  { INPUT, & HW_ENT (HW_H_SLO16), CGEN_MODE_HI, & OP_ENT (SLO16), 0 },
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SRC1), 0 },
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SRC2), 0 },
  { OUTPUT, & HW_ENT (HW_H_MEMORY), CGEN_MODE_HI, 0, 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_69_st_plus_ops[] = {
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SRC1), 0 },
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SRC2), 0 },
  { OUTPUT, & HW_ENT (HW_H_MEMORY), CGEN_MODE_SI, 0, 0 },
  { OUTPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SRC2), 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_70_trap_ops[] = {
  { INPUT, & HW_ENT (HW_H_UINT), CGEN_MODE_USI, & OP_ENT (UIMM4), 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_71_unlock_ops[] = {
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SRC1), 0 },
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SRC2), 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_74_satb_ops[] = {
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SRC2), 0 },
  { OUTPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (DR), 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_75_sat_ops[] = {
  { INPUT, & HW_ENT (HW_H_COND), CGEN_MODE_UBI, 0, 0 },
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SRC2), 0 },
  { OUTPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (DR), 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_76_sadd_ops[] = {
  { INPUT, & HW_ENT (HW_H_ACCUMS), CGEN_MODE_DI, 0, 0 },
  { INPUT, & HW_ENT (HW_H_ACCUMS), CGEN_MODE_DI, 0, 1 },
  { OUTPUT, & HW_ENT (HW_H_ACCUMS), CGEN_MODE_DI, 0, 0 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_77_macwu1_ops[] = {
  { INPUT, & HW_ENT (HW_H_ACCUMS), CGEN_MODE_DI, 0, 1 },
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SRC1), 0 },
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SRC2), 0 },
  { OUTPUT, & HW_ENT (HW_H_ACCUMS), CGEN_MODE_DI, 0, 1 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_78_mulwu1_ops[] = {
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SRC1), 0 },
  { INPUT, & HW_ENT (HW_H_GR), CGEN_MODE_SI, & OP_ENT (SRC2), 0 },
  { OUTPUT, & HW_ENT (HW_H_ACCUMS), CGEN_MODE_DI, 0, 1 },
  { 0 }
};

static const CGEN_OPERAND_INSTANCE fmt_79_sc_ops[] = {
  { INPUT, & HW_ENT (HW_H_COND), CGEN_MODE_UBI, 0, 0 },
  { OUTPUT, & HW_ENT (HW_H_ABORT), CGEN_MODE_UBI, 0, 0 },
  { 0 }
};

#undef INPUT
#undef OUTPUT

#define OP 1 /* syntax value for mnemonic */

static const CGEN_SYNTAX syntax_table[] =
{
/* <op> $dr,$sr */
/*   0 */  { OP, ' ', 130, ',', 129, 0 },
/* <op> $dr,$sr,#$slo16 */
/*   1 */  { OP, ' ', 130, ',', 129, ',', '#', 145, 0 },
/* <op> $dr,$sr,$slo16 */
/*   2 */  { OP, ' ', 130, ',', 129, ',', 145, 0 },
/* <op> $dr,$sr,#$uimm16 */
/*   3 */  { OP, ' ', 130, ',', 129, ',', '#', 139, 0 },
/* <op> $dr,$sr,$uimm16 */
/*   4 */  { OP, ' ', 130, ',', 129, ',', 139, 0 },
/* <op> $dr,$sr,#$ulo16 */
/*   5 */  { OP, ' ', 130, ',', 129, ',', '#', 146, 0 },
/* <op> $dr,$sr,$ulo16 */
/*   6 */  { OP, ' ', 130, ',', 129, ',', 146, 0 },
/* <op> $dr,#$simm8 */
/*   7 */  { OP, ' ', 130, ',', '#', 135, 0 },
/* <op> $dr,$simm8 */
/*   8 */  { OP, ' ', 130, ',', 135, 0 },
/* <op> $dr,$sr,#$simm16 */
/*   9 */  { OP, ' ', 130, ',', 129, ',', '#', 136, 0 },
/* <op> $dr,$sr,$simm16 */
/*  10 */  { OP, ' ', 130, ',', 129, ',', 136, 0 },
/* <op> $disp8 */
/*  11 */  { OP, ' ', 148, 0 },
/* <op> $disp24 */
/*  12 */  { OP, ' ', 150, 0 },
/* <op> $src1,$src2,$disp16 */
/*  13 */  { OP, ' ', 131, ',', 132, ',', 149, 0 },
/* <op> $src2,$disp16 */
/*  14 */  { OP, ' ', 132, ',', 149, 0 },
/* <op> $src1,$src2 */
/*  15 */  { OP, ' ', 131, ',', 132, 0 },
/* <op> $src2,#$simm16 */
/*  16 */  { OP, ' ', 132, ',', '#', 136, 0 },
/* <op> $src2,$simm16 */
/*  17 */  { OP, ' ', 132, ',', 136, 0 },
/* <op> $src2,#$uimm16 */
/*  18 */  { OP, ' ', 132, ',', '#', 139, 0 },
/* <op> $src2,$uimm16 */
/*  19 */  { OP, ' ', 132, ',', 139, 0 },
/* <op> $src2 */
/*  20 */  { OP, ' ', 132, 0 },
/* <op> $sr */
/*  21 */  { OP, ' ', 129, 0 },
/* <op> $dr,@$sr */
/*  22 */  { OP, ' ', 130, ',', '@', 129, 0 },
/* <op> $dr,@($sr) */
/*  23 */  { OP, ' ', 130, ',', '@', '(', 129, ')', 0 },
/* <op> $dr,@($slo16,$sr) */
/*  24 */  { OP, ' ', 130, ',', '@', '(', 145, ',', 129, ')', 0 },
/* <op> $dr,@($sr,$slo16) */
/*  25 */  { OP, ' ', 130, ',', '@', '(', 129, ',', 145, ')', 0 },
/* <op> $dr,@$sr+ */
/*  26 */  { OP, ' ', 130, ',', '@', 129, '+', 0 },
/* <op> $dr,#$uimm24 */
/*  27 */  { OP, ' ', 130, ',', '#', 147, 0 },
/* <op> $dr,$uimm24 */
/*  28 */  { OP, ' ', 130, ',', 147, 0 },
/* <op> $dr,$slo16 */
/*  29 */  { OP, ' ', 130, ',', 145, 0 },
/* <op> $src1,$src2,$acc */
/*  30 */  { OP, ' ', 131, ',', 132, ',', 143, 0 },
/* <op> $dr */
/*  31 */  { OP, ' ', 130, 0 },
/* <op> $dr,$accs */
/*  32 */  { OP, ' ', 130, ',', 142, 0 },
/* <op> $dr,$scr */
/*  33 */  { OP, ' ', 130, ',', 133, 0 },
/* <op> $src1 */
/*  34 */  { OP, ' ', 131, 0 },
/* <op> $src1,$accs */
/*  35 */  { OP, ' ', 131, ',', 142, 0 },
/* <op> $sr,$dcr */
/*  36 */  { OP, ' ', 129, ',', 134, 0 },
/* <op> */
/*  37 */  { OP, 0 },
/* <op> $accd */
/*  38 */  { OP, ' ', 141, 0 },
/* <op> $accd,$accs */
/*  39 */  { OP, ' ', 141, ',', 142, 0 },
/* <op> $accd,$accs,#$imm1 */
/*  40 */  { OP, ' ', 141, ',', 142, ',', '#', 140, 0 },
/* <op> $dr,#$hi16 */
/*  41 */  { OP, ' ', 130, ',', '#', 144, 0 },
/* <op> $dr,$hi16 */
/*  42 */  { OP, ' ', 130, ',', 144, 0 },
/* <op> $dr,#$uimm5 */
/*  43 */  { OP, ' ', 130, ',', '#', 138, 0 },
/* <op> $dr,$uimm5 */
/*  44 */  { OP, ' ', 130, ',', 138, 0 },
/* <op> $src1,@$src2 */
/*  45 */  { OP, ' ', 131, ',', '@', 132, 0 },
/* <op> $src1,@($src2) */
/*  46 */  { OP, ' ', 131, ',', '@', '(', 132, ')', 0 },
/* <op> $src1,@($slo16,$src2) */
/*  47 */  { OP, ' ', 131, ',', '@', '(', 145, ',', 132, ')', 0 },
/* <op> $src1,@($src2,$slo16) */
/*  48 */  { OP, ' ', 131, ',', '@', '(', 132, ',', 145, ')', 0 },
/* <op> $src1,@+$src2 */
/*  49 */  { OP, ' ', 131, ',', '@', '+', 132, 0 },
/* <op> $src1,@-$src2 */
/*  50 */  { OP, ' ', 131, ',', '@', '-', 132, 0 },
/* <op> #$uimm4 */
/*  51 */  { OP, ' ', '#', 137, 0 },
/* <op> $uimm4 */
/*  52 */  { OP, ' ', 137, 0 },
/* <op> $dr,$src2 */
/*  53 */  { OP, ' ', 130, ',', 132, 0 },
};

#undef OP

static const CGEN_FORMAT format_table[] = 
{
/* (f-op1 #)(f-r1 dr)(f-op2 #)(f-r2 sr)(dr SI)(sr SI)(dr SI) */
/*   0 */  { 16, 16, 0xf0f0 },
/* (f-op1 #)(f-r1 dr)(f-op2 #)(f-r2 sr)(f-simm16 slo16)(slo16 HI)(sr SI)(dr SI) */
/*   1 */  { 32, 32, 0xf0f00000 },
/* (f-op1 #)(f-r1 dr)(f-op2 #)(f-r2 sr)(f-uimm16 uimm16)(sr SI)(uimm16 USI)(dr SI) */
/*   2 */  { 32, 32, 0xf0f00000 },
/* (f-op1 #)(f-r1 dr)(f-op2 #)(f-r2 sr)(f-uimm16 ulo16)(sr SI)(ulo16 UHI)(dr SI) */
/*   3 */  { 32, 32, 0xf0f00000 },
/* (f-op1 #)(f-r1 dr)(f-simm8 simm8)(dr SI)(simm8 SI)(dr SI) */
/*   4 */  { 16, 16, 0xf000 },
/* (f-op1 #)(f-r1 dr)(f-op2 #)(f-r2 sr)(dr SI)(sr SI)(condbit UBI)(dr SI) */
/*   5 */  { 16, 16, 0xf0f0 },
/* (f-op1 #)(f-r1 dr)(f-op2 #)(f-r2 sr)(f-simm16 simm16)(simm16 SI)(sr SI)(condbit UBI)(dr SI) */
/*   6 */  { 32, 32, 0xf0f00000 },
/* (f-op1 #)(f-r1 dr)(f-op2 #)(f-r2 sr)(condbit UBI)(dr SI)(sr SI)(condbit UBI)(dr SI) */
/*   7 */  { 16, 16, 0xf0f0 },
/* (f-op1 #)(f-r1 #)(f-disp8 disp8)(condbit UBI)(disp8 VM) */
/*   8 */  { 16, 16, 0xff00 },
/* (f-op1 #)(f-r1 #)(f-disp8 disp8) */
/*   9 */  { 16, 16, 0xff00 },
/* (f-op1 #)(f-r1 #)(f-disp24 disp24)(condbit UBI)(disp24 VM) */
/*  10 */  { 32, 32, 0xff000000 },
/* (f-op1 #)(f-r1 #)(f-disp24 disp24) */
/*  11 */  { 32, 32, 0xff000000 },
/* (f-op1 #)(f-r1 src1)(f-op2 #)(f-r2 src2)(f-disp16 disp16)(disp16 VM)(src1 SI)(src2 SI) */
/*  12 */  { 32, 32, 0xf0f00000 },
/* (f-op1 #)(f-r1 #)(f-op2 #)(f-r2 src2)(f-disp16 disp16)(disp16 VM)(src2 SI) */
/*  13 */  { 32, 32, 0xfff00000 },
/* (f-op1 #)(f-r1 #)(f-disp8 disp8)(disp8 VM)(pc USI)(h-gr-14 SI) */
/*  14 */  { 16, 16, 0xff00 },
/* (f-op1 #)(f-r1 #)(f-disp24 disp24)(disp24 VM)(pc USI)(h-gr-14 SI) */
/*  15 */  { 32, 32, 0xff000000 },
/* (f-op1 #)(f-r1 #)(f-disp8 disp8)(condbit UBI)(disp8 VM)(pc USI)(h-gr-14 SI) */
/*  16 */  { 16, 16, 0xff00 },
/* (f-op1 #)(f-r1 #)(f-disp24 disp24)(condbit UBI)(disp24 VM)(pc USI)(h-gr-14 SI) */
/*  17 */  { 32, 32, 0xff000000 },
/* (f-op1 #)(f-r1 #)(f-disp8 disp8)(disp8 VM) */
/*  18 */  { 16, 16, 0xff00 },
/* (f-op1 #)(f-r1 #)(f-disp24 disp24)(disp24 VM) */
/*  19 */  { 32, 32, 0xff000000 },
/* (f-op1 #)(f-r1 src1)(f-op2 #)(f-r2 src2)(src1 SI)(src2 SI)(condbit UBI) */
/*  20 */  { 16, 16, 0xf0f0 },
/* (f-op1 #)(f-r1 #)(f-op2 #)(f-r2 src2)(f-simm16 simm16)(simm16 SI)(src2 SI)(condbit UBI) */
/*  21 */  { 32, 32, 0xfff00000 },
/* (f-op1 #)(f-r1 #)(f-op2 #)(f-r2 src2)(f-uimm16 uimm16)(src2 SI)(uimm16 USI)(condbit UBI) */
/*  22 */  { 32, 32, 0xfff00000 },
/* (f-op1 #)(f-r1 #)(f-op2 #)(f-r2 src2)(src2 SI)(condbit UBI) */
/*  23 */  { 16, 16, 0xfff0 },
/* (f-op1 #)(f-r1 dr)(f-op2 #)(f-r2 sr)(f-simm16 #)(dr SI)(sr SI)(dr SI) */
/*  24 */  { 32, 32, 0xf0f0ffff },
/* (f-op1 #)(f-r1 #)(f-op2 #)(f-r2 sr)(condbit UBI)(sr SI) */
/*  25 */  { 16, 16, 0xfff0 },
/* (f-op1 #)(f-r1 #)(f-op2 #)(f-r2 sr)(pc USI)(sr SI)(h-gr-14 SI) */
/*  26 */  { 16, 16, 0xfff0 },
/* (f-op1 #)(f-r1 #)(f-op2 #)(f-r2 sr)(sr SI) */
/*  27 */  { 16, 16, 0xfff0 },
/* (f-op1 #)(f-r1 dr)(f-op2 #)(f-r2 sr)(h-memory-sr SI)(sr SI)(dr SI) */
/*  28 */  { 16, 16, 0xf0f0 },
/* (f-op1 #)(f-r1 dr)(f-op2 #)(f-r2 sr) */
/*  29 */  { 16, 16, 0xf0f0 },
/* (f-op1 #)(f-r1 dr)(f-op2 #)(f-r2 sr)(f-simm16 slo16)(h-memory-add-WI-sr-slo16 SI)(slo16 HI)(sr SI)(dr SI) */
/*  30 */  { 32, 32, 0xf0f00000 },
/* (f-op1 #)(f-r1 dr)(f-op2 #)(f-r2 sr)(f-simm16 slo16) */
/*  31 */  { 32, 32, 0xf0f00000 },
/* (f-op1 #)(f-r1 dr)(f-op2 #)(f-r2 sr)(h-memory-sr QI)(sr SI)(dr SI) */
/*  32 */  { 16, 16, 0xf0f0 },
/* (f-op1 #)(f-r1 dr)(f-op2 #)(f-r2 sr)(f-simm16 slo16)(h-memory-add-WI-sr-slo16 QI)(slo16 HI)(sr SI)(dr SI) */
/*  33 */  { 32, 32, 0xf0f00000 },
/* (f-op1 #)(f-r1 dr)(f-op2 #)(f-r2 sr)(h-memory-sr HI)(sr SI)(dr SI) */
/*  34 */  { 16, 16, 0xf0f0 },
/* (f-op1 #)(f-r1 dr)(f-op2 #)(f-r2 sr)(f-simm16 slo16)(h-memory-add-WI-sr-slo16 HI)(slo16 HI)(sr SI)(dr SI) */
/*  35 */  { 32, 32, 0xf0f00000 },
/* (f-op1 #)(f-r1 dr)(f-op2 #)(f-r2 sr)(h-memory-sr SI)(sr SI)(dr SI)(sr SI) */
/*  36 */  { 16, 16, 0xf0f0 },
/* (f-op1 #)(f-r1 dr)(f-uimm24 uimm24)(uimm24 VM)(dr SI) */
/*  37 */  { 32, 32, 0xf0000000 },
/* (f-op1 #)(f-r1 dr)(f-simm8 simm8)(simm8 SI)(dr SI) */
/*  38 */  { 16, 16, 0xf000 },
/* (f-op1 #)(f-r1 dr)(f-op2 #)(f-r2 #)(f-simm16 slo16)(slo16 HI)(dr SI) */
/*  39 */  { 32, 32, 0xf0ff0000 },
/* (f-op1 #)(f-r1 dr)(f-op2 #)(f-r2 sr)(dr SI)(sr SI) */
/*  40 */  { 16, 16, 0xf0f0 },
/* (f-op1 #)(f-r1 src1)(f-op2 #)(f-r2 src2)(accum DI)(src1 SI)(src2 SI)(accum DI) */
/*  41 */  { 16, 16, 0xf0f0 },
/* (f-op1 #)(f-r1 src1)(f-acc acc)(f-op23 #)(f-r2 src2)(acc DI)(src1 SI)(src2 SI)(acc DI) */
/*  42 */  { 16, 16, 0xf070 },
/* (f-op1 #)(f-r1 src1)(f-op2 #)(f-r2 src2)(src1 SI)(src2 SI)(accum DI) */
/*  43 */  { 16, 16, 0xf0f0 },
/* (f-op1 #)(f-r1 src1)(f-acc acc)(f-op23 #)(f-r2 src2)(src1 SI)(src2 SI)(acc DI) */
/*  44 */  { 16, 16, 0xf070 },
/* (f-op1 #)(f-r1 dr)(f-op2 #)(f-r2 sr)(sr SI)(dr SI) */
/*  45 */  { 16, 16, 0xf0f0 },
/* (f-op1 #)(f-r1 dr)(f-op2 #)(f-r2 #)(accum DI)(dr SI) */
/*  46 */  { 16, 16, 0xf0ff },
/* (f-op1 #)(f-r1 dr)(f-op2 #)(f-accs accs)(f-op3 #)(accs DI)(dr SI) */
/*  47 */  { 16, 16, 0xf0f3 },
/* (f-op1 #)(f-r1 dr)(f-op2 #)(f-r2 scr)(scr SI)(dr SI) */
/*  48 */  { 16, 16, 0xf0f0 },
/* (f-op1 #)(f-r1 src1)(f-op2 #)(f-r2 #)(accum DI)(src1 SI)(accum DI) */
/*  49 */  { 16, 16, 0xf0ff },
/* (f-op1 #)(f-r1 src1)(f-op2 #)(f-accs accs)(f-op3 #)(accs DI)(src1 SI)(accs DI) */
/*  50 */  { 16, 16, 0xf0f3 },
/* (f-op1 #)(f-r1 dcr)(f-op2 #)(f-r2 sr)(sr SI)(dcr SI) */
/*  51 */  { 16, 16, 0xf0f0 },
/* (f-op1 #)(f-r1 #)(f-op2 #)(f-r2 #) */
/*  52 */  { 16, 16, 0xffff },
/* (f-op1 #)(f-r1 #)(f-op2 #)(f-r2 #)(accum DI)(accum DI) */
/*  53 */  { 16, 16, 0xffff },
/* (f-op1 #)(f-accd accd)(f-bits67 #)(f-op2 #)(f-accs #)(f-bit14 #)(f-imm1 #)(accum DI)(accd DI) */
/*  54 */  { 16, 16, 0xf3ff },
/* (f-op1 #)(f-accd accd)(f-bits67 #)(f-op2 #)(f-accs accs)(f-bit14 #)(f-imm1 #)(accs DI)(accd DI) */
/*  55 */  { 16, 16, 0xf3f3 },
/* (f-op1 #)(f-accd accd)(f-bits67 #)(f-op2 #)(f-accs accs)(f-bit14 #)(f-imm1 imm1)(accs DI)(imm1 USI)(accd DI) */
/*  56 */  { 16, 16, 0xf3f2 },
/* (f-op1 #)(f-r1 #)(f-op2 #)(f-r2 #)(h-bcond-0 VM)(h-bie-0 VM)(h-bpc-0 VM)(h-bsm-0 VM)(condbit UBI)(pc USI)(h-ie-0 VM)(h-sm-0 VM) */
/*  57 */  { 16, 16, 0xffff },
/* (f-op1 #)(f-r1 dr)(f-op2 #)(f-r2 #)(f-hi16 hi16)(hi16 UHI)(dr SI) */
/*  58 */  { 32, 32, 0xf0ff0000 },
/* (f-op1 #)(f-r1 dr)(f-op2 #)(f-r2 sr)(f-simm16 simm16)(simm16 SI)(sr SI)(dr SI) */
/*  59 */  { 32, 32, 0xf0f00000 },
/* (f-op1 #)(f-r1 dr)(f-shift-op2 #)(f-uimm5 uimm5)(dr SI)(uimm5 USI)(dr SI) */
/*  60 */  { 16, 16, 0xf0e0 },
/* (f-op1 #)(f-r1 src1)(f-op2 #)(f-r2 src2)(src1 SI)(src2 SI)(h-memory-src2 SI) */
/*  61 */  { 16, 16, 0xf0f0 },
/* (f-op1 #)(f-r1 src1)(f-op2 #)(f-r2 src2) */
/*  62 */  { 16, 16, 0xf0f0 },
/* (f-op1 #)(f-r1 src1)(f-op2 #)(f-r2 src2)(f-simm16 slo16)(slo16 HI)(src1 SI)(src2 SI)(h-memory-add-WI-src2-slo16 SI) */
/*  63 */  { 32, 32, 0xf0f00000 },
/* (f-op1 #)(f-r1 src1)(f-op2 #)(f-r2 src2)(f-simm16 slo16) */
/*  64 */  { 32, 32, 0xf0f00000 },
/* (f-op1 #)(f-r1 src1)(f-op2 #)(f-r2 src2)(src1 SI)(src2 SI)(h-memory-src2 QI) */
/*  65 */  { 16, 16, 0xf0f0 },
/* (f-op1 #)(f-r1 src1)(f-op2 #)(f-r2 src2)(f-simm16 slo16)(slo16 HI)(src1 SI)(src2 SI)(h-memory-add-WI-src2-slo16 QI) */
/*  66 */  { 32, 32, 0xf0f00000 },
/* (f-op1 #)(f-r1 src1)(f-op2 #)(f-r2 src2)(src1 SI)(src2 SI)(h-memory-src2 HI) */
/*  67 */  { 16, 16, 0xf0f0 },
/* (f-op1 #)(f-r1 src1)(f-op2 #)(f-r2 src2)(f-simm16 slo16)(slo16 HI)(src1 SI)(src2 SI)(h-memory-add-WI-src2-slo16 HI) */
/*  68 */  { 32, 32, 0xf0f00000 },
/* (f-op1 #)(f-r1 src1)(f-op2 #)(f-r2 src2)(src1 SI)(src2 SI)(h-memory-src2 SI)(src2 SI) */
/*  69 */  { 16, 16, 0xf0f0 },
/* (f-op1 #)(f-r1 #)(f-op2 #)(f-uimm4 uimm4)(uimm4 USI) */
/*  70 */  { 16, 16, 0xfff0 },
/* (f-op1 #)(f-r1 src1)(f-op2 #)(f-r2 src2)(src1 SI)(src2 SI) */
/*  71 */  { 16, 16, 0xf0f0 },
/* (f-op1 #)(f-r1 src1)(f-op2 #)(f-r2 #) */
/*  72 */  { 16, 16, 0xf0ff },
/* (f-op1 #)(f-r1 dr)(f-op2 #)(f-r2 #) */
/*  73 */  { 16, 16, 0xf0ff },
/* (f-op1 #)(f-r1 dr)(f-op2 #)(f-r2 src2)(f-uimm16 #)(src2 SI)(dr SI) */
/*  74 */  { 32, 32, 0xf0f0ffff },
/* (f-op1 #)(f-r1 dr)(f-op2 #)(f-r2 src2)(f-uimm16 #)(condbit UBI)(src2 SI)(dr SI) */
/*  75 */  { 32, 32, 0xf0f0ffff },
/* (f-op1 #)(f-r1 #)(f-op2 #)(f-r2 #)(h-accums-0 DI)(h-accums-1 DI)(h-accums-0 DI) */
/*  76 */  { 16, 16, 0xffff },
/* (f-op1 #)(f-r1 src1)(f-op2 #)(f-r2 src2)(h-accums-1 DI)(src1 SI)(src2 SI)(h-accums-1 DI) */
/*  77 */  { 16, 16, 0xf0f0 },
/* (f-op1 #)(f-r1 src1)(f-op2 #)(f-r2 src2)(src1 SI)(src2 SI)(h-accums-1 DI) */
/*  78 */  { 16, 16, 0xf0f0 },
/* (f-op1 #)(f-r1 #)(f-op2 #)(f-r2 #)(condbit UBI)(abort-parallel-execution UBI) */
/*  79 */  { 16, 16, 0xffff },
};

#define A(a) (1 << CGEN_CAT3 (CGEN_INSN,_,a))
#define SYN(n) (& syntax_table[n])
#define FMT(n) (& format_table[n])

/* The instruction table.  */

const CGEN_INSN m32r_cgen_insn_table_entries[MAX_INSNS] =
{
  /* null first entry, end of all hash chains */
  { { 0 }, 0 },
/* add $dr,$sr */
  {
    { 1, 1, 1, 1 },
    "add", "add", SYN (0), FMT (0), 0xa0,
    & fmt_0_add_ops[0],
    { 2, 0|A(PARALLEL), { (1<<MACH_M32R), PIPE_OS } }
  },
/* add3 $dr,$sr,#$slo16 */
  {
    { 1, 1, 1, 1 },
    "add3", "add3", SYN (1), FMT (1), 0x80a00000,
    & fmt_1_add3_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_NONE } }
  },
/* add3 $dr,$sr,$slo16 */
  {
    { 1, 1, 1, 1 },
    "add3.a", "add3", SYN (2), FMT (1), 0x80a00000,
    & fmt_1_add3_ops[0],
    { 2, 0|A(ALIAS), { (1<<MACH_M32R), PIPE_NONE } }
  },
/* and $dr,$sr */
  {
    { 1, 1, 1, 1 },
    "and", "and", SYN (0), FMT (0), 0xc0,
    & fmt_0_add_ops[0],
    { 2, 0|A(PARALLEL), { (1<<MACH_M32R), PIPE_OS } }
  },
/* and3 $dr,$sr,#$uimm16 */
  {
    { 1, 1, 1, 1 },
    "and3", "and3", SYN (3), FMT (2), 0x80c00000,
    & fmt_2_and3_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_NONE } }
  },
/* and3 $dr,$sr,$uimm16 */
  {
    { 1, 1, 1, 1 },
    "and3.a", "and3", SYN (4), FMT (2), 0x80c00000,
    & fmt_2_and3_ops[0],
    { 2, 0|A(ALIAS), { (1<<MACH_M32R), PIPE_NONE } }
  },
/* or $dr,$sr */
  {
    { 1, 1, 1, 1 },
    "or", "or", SYN (0), FMT (0), 0xe0,
    & fmt_0_add_ops[0],
    { 2, 0|A(PARALLEL), { (1<<MACH_M32R), PIPE_OS } }
  },
/* or3 $dr,$sr,#$ulo16 */
  {
    { 1, 1, 1, 1 },
    "or3", "or3", SYN (5), FMT (3), 0x80e00000,
    & fmt_3_or3_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_NONE } }
  },
/* or3 $dr,$sr,$ulo16 */
  {
    { 1, 1, 1, 1 },
    "or3.a", "or3", SYN (6), FMT (3), 0x80e00000,
    & fmt_3_or3_ops[0],
    { 2, 0|A(ALIAS), { (1<<MACH_M32R), PIPE_NONE } }
  },
/* xor $dr,$sr */
  {
    { 1, 1, 1, 1 },
    "xor", "xor", SYN (0), FMT (0), 0xd0,
    & fmt_0_add_ops[0],
    { 2, 0|A(PARALLEL), { (1<<MACH_M32R), PIPE_OS } }
  },
/* xor3 $dr,$sr,#$uimm16 */
  {
    { 1, 1, 1, 1 },
    "xor3", "xor3", SYN (3), FMT (2), 0x80d00000,
    & fmt_2_and3_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_NONE } }
  },
/* xor3 $dr,$sr,$uimm16 */
  {
    { 1, 1, 1, 1 },
    "xor3.a", "xor3", SYN (4), FMT (2), 0x80d00000,
    & fmt_2_and3_ops[0],
    { 2, 0|A(ALIAS), { (1<<MACH_M32R), PIPE_NONE } }
  },
/* addi $dr,#$simm8 */
  {
    { 1, 1, 1, 1 },
    "addi", "addi", SYN (7), FMT (4), 0x4000,
    & fmt_4_addi_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_OS } }
  },
/* addi $dr,$simm8 */
  {
    { 1, 1, 1, 1 },
    "addi.a", "addi", SYN (8), FMT (4), 0x4000,
    & fmt_4_addi_ops[0],
    { 2, 0|A(ALIAS), { (1<<MACH_M32R), PIPE_OS } }
  },
/* addv $dr,$sr */
  {
    { 1, 1, 1, 1 },
    "addv", "addv", SYN (0), FMT (5), 0x80,
    & fmt_5_addv_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_OS } }
  },
/* addv3 $dr,$sr,#$simm16 */
  {
    { 1, 1, 1, 1 },
    "addv3", "addv3", SYN (9), FMT (6), 0x80800000,
    & fmt_6_addv3_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_NONE } }
  },
/* addv3 $dr,$sr,$simm16 */
  {
    { 1, 1, 1, 1 },
    "addv3.a", "addv3", SYN (10), FMT (6), 0x80800000,
    & fmt_6_addv3_ops[0],
    { 2, 0|A(ALIAS), { (1<<MACH_M32R), PIPE_NONE } }
  },
/* addx $dr,$sr */
  {
    { 1, 1, 1, 1 },
    "addx", "addx", SYN (0), FMT (7), 0x90,
    & fmt_7_addx_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_OS } }
  },
/* bc $disp8 */
  {
    { 1, 1, 1, 1 },
    "bc8", "bc", SYN (11), FMT (8), 0x7c00,
    & fmt_8_bc8_ops[0],
    { 2, 0|A(RELAXABLE)|A(COND_CTI), { (1<<MACH_M32R), PIPE_O } }
  },
/* bc.s $disp8 */
  {
    { 1, 1, 1, 1 },
    "bc8.s", "bc.s", SYN (11), FMT (9), 0x7c00,
    0,
    { 2, 0|A(ALIAS)|A(COND_CTI), { (1<<MACH_M32R), PIPE_O } }
  },
/* bc $disp24 */
  {
    { 1, 1, 1, 1 },
    "bc24", "bc", SYN (12), FMT (10), 0xfc000000,
    & fmt_10_bc24_ops[0],
    { 2, 0|A(RELAX)|A(COND_CTI), { (1<<MACH_M32R), PIPE_NONE } }
  },
/* bc.l $disp24 */
  {
    { 1, 1, 1, 1 },
    "bc24.l", "bc.l", SYN (12), FMT (11), 0xfc000000,
    0,
    { 2, 0|A(ALIAS)|A(COND_CTI), { (1<<MACH_M32R), PIPE_NONE } }
  },
/* beq $src1,$src2,$disp16 */
  {
    { 1, 1, 1, 1 },
    "beq", "beq", SYN (13), FMT (12), 0xb0000000,
    & fmt_12_beq_ops[0],
    { 2, 0|A(COND_CTI), { (1<<MACH_M32R), PIPE_NONE } }
  },
/* beqz $src2,$disp16 */
  {
    { 1, 1, 1, 1 },
    "beqz", "beqz", SYN (14), FMT (13), 0xb0800000,
    & fmt_13_beqz_ops[0],
    { 2, 0|A(COND_CTI), { (1<<MACH_M32R), PIPE_NONE } }
  },
/* bgez $src2,$disp16 */
  {
    { 1, 1, 1, 1 },
    "bgez", "bgez", SYN (14), FMT (13), 0xb0b00000,
    & fmt_13_beqz_ops[0],
    { 2, 0|A(COND_CTI), { (1<<MACH_M32R), PIPE_NONE } }
  },
/* bgtz $src2,$disp16 */
  {
    { 1, 1, 1, 1 },
    "bgtz", "bgtz", SYN (14), FMT (13), 0xb0d00000,
    & fmt_13_beqz_ops[0],
    { 2, 0|A(COND_CTI), { (1<<MACH_M32R), PIPE_NONE } }
  },
/* blez $src2,$disp16 */
  {
    { 1, 1, 1, 1 },
    "blez", "blez", SYN (14), FMT (13), 0xb0c00000,
    & fmt_13_beqz_ops[0],
    { 2, 0|A(COND_CTI), { (1<<MACH_M32R), PIPE_NONE } }
  },
/* bltz $src2,$disp16 */
  {
    { 1, 1, 1, 1 },
    "bltz", "bltz", SYN (14), FMT (13), 0xb0a00000,
    & fmt_13_beqz_ops[0],
    { 2, 0|A(COND_CTI), { (1<<MACH_M32R), PIPE_NONE } }
  },
/* bnez $src2,$disp16 */
  {
    { 1, 1, 1, 1 },
    "bnez", "bnez", SYN (14), FMT (13), 0xb0900000,
    & fmt_13_beqz_ops[0],
    { 2, 0|A(COND_CTI), { (1<<MACH_M32R), PIPE_NONE } }
  },
/* bl $disp8 */
  {
    { 1, 1, 1, 1 },
    "bl8", "bl", SYN (11), FMT (14), 0x7e00,
    & fmt_14_bl8_ops[0],
    { 2, 0|A(FILL_SLOT)|A(RELAXABLE)|A(UNCOND_CTI), { (1<<MACH_M32R), PIPE_O } }
  },
/* bl.s $disp8 */
  {
    { 1, 1, 1, 1 },
    "bl8.s", "bl.s", SYN (11), FMT (9), 0x7e00,
    0,
    { 2, 0|A(FILL_SLOT)|A(ALIAS)|A(UNCOND_CTI), { (1<<MACH_M32R), PIPE_O } }
  },
/* bl $disp24 */
  {
    { 1, 1, 1, 1 },
    "bl24", "bl", SYN (12), FMT (15), 0xfe000000,
    & fmt_15_bl24_ops[0],
    { 2, 0|A(RELAX)|A(UNCOND_CTI), { (1<<MACH_M32R), PIPE_NONE } }
  },
/* bl.l $disp24 */
  {
    { 1, 1, 1, 1 },
    "bl24.l", "bl.l", SYN (12), FMT (11), 0xfe000000,
    0,
    { 2, 0|A(ALIAS)|A(UNCOND_CTI), { (1<<MACH_M32R), PIPE_NONE } }
  },
/* start-sanitize-m32rx */
/* bcl $disp8 */
  {
    { 1, 1, 1, 1 },
    "bcl8", "bcl", SYN (11), FMT (16), 0x7800,
    & fmt_16_bcl8_ops[0],
    { 2, 0|A(RELAXABLE)|A(COND_CTI), { (1<<MACH_M32RX), PIPE_O } }
  },
/* end-sanitize-m32rx */
/* start-sanitize-m32rx */
/* bcl.s $disp8 */
  {
    { 1, 1, 1, 1 },
    "bcl8.s", "bcl.s", SYN (11), FMT (9), 0x7800,
    0,
    { 2, 0|A(ALIAS)|A(COND_CTI), { (1<<MACH_M32RX), PIPE_O } }
  },
/* end-sanitize-m32rx */
/* start-sanitize-m32rx */
/* bcl $disp24 */
  {
    { 1, 1, 1, 1 },
    "bcl24", "bcl", SYN (12), FMT (17), 0xf8000000,
    & fmt_17_bcl24_ops[0],
    { 2, 0|A(RELAX)|A(COND_CTI), { (1<<MACH_M32RX), PIPE_NONE } }
  },
/* end-sanitize-m32rx */
/* start-sanitize-m32rx */
/* bcl.l $disp24 */
  {
    { 1, 1, 1, 1 },
    "bcl24.l", "bcl.l", SYN (12), FMT (11), 0xf8000000,
    0,
    { 2, 0|A(ALIAS)|A(COND_CTI), { (1<<MACH_M32RX), PIPE_NONE } }
  },
/* end-sanitize-m32rx */
/* bnc $disp8 */
  {
    { 1, 1, 1, 1 },
    "bnc8", "bnc", SYN (11), FMT (8), 0x7d00,
    & fmt_8_bc8_ops[0],
    { 2, 0|A(RELAXABLE)|A(COND_CTI), { (1<<MACH_M32R), PIPE_O } }
  },
/* bnc.s $disp8 */
  {
    { 1, 1, 1, 1 },
    "bnc8.s", "bnc.s", SYN (11), FMT (9), 0x7d00,
    0,
    { 2, 0|A(ALIAS)|A(COND_CTI), { (1<<MACH_M32R), PIPE_O } }
  },
/* bnc $disp24 */
  {
    { 1, 1, 1, 1 },
    "bnc24", "bnc", SYN (12), FMT (10), 0xfd000000,
    & fmt_10_bc24_ops[0],
    { 2, 0|A(RELAX)|A(COND_CTI), { (1<<MACH_M32R), PIPE_NONE } }
  },
/* bnc.l $disp24 */
  {
    { 1, 1, 1, 1 },
    "bnc24.l", "bnc.l", SYN (12), FMT (11), 0xfd000000,
    0,
    { 2, 0|A(ALIAS)|A(COND_CTI), { (1<<MACH_M32R), PIPE_NONE } }
  },
/* bne $src1,$src2,$disp16 */
  {
    { 1, 1, 1, 1 },
    "bne", "bne", SYN (13), FMT (12), 0xb0100000,
    & fmt_12_beq_ops[0],
    { 2, 0|A(COND_CTI), { (1<<MACH_M32R), PIPE_NONE } }
  },
/* bra $disp8 */
  {
    { 1, 1, 1, 1 },
    "bra8", "bra", SYN (11), FMT (18), 0x7f00,
    & fmt_18_bra8_ops[0],
    { 2, 0|A(FILL_SLOT)|A(RELAXABLE)|A(UNCOND_CTI), { (1<<MACH_M32R), PIPE_O } }
  },
/* bra.s $disp8 */
  {
    { 1, 1, 1, 1 },
    "bra8.s", "bra.s", SYN (11), FMT (9), 0x7f00,
    0,
    { 2, 0|A(ALIAS)|A(UNCOND_CTI), { (1<<MACH_M32R), PIPE_O } }
  },
/* bra $disp24 */
  {
    { 1, 1, 1, 1 },
    "bra24", "bra", SYN (12), FMT (19), 0xff000000,
    & fmt_19_bra24_ops[0],
    { 2, 0|A(RELAX)|A(UNCOND_CTI), { (1<<MACH_M32R), PIPE_NONE } }
  },
/* bra.l $disp24 */
  {
    { 1, 1, 1, 1 },
    "bra24.l", "bra.l", SYN (12), FMT (11), 0xff000000,
    0,
    { 2, 0|A(ALIAS)|A(UNCOND_CTI), { (1<<MACH_M32R), PIPE_NONE } }
  },
/* start-sanitize-m32rx */
/* bncl $disp8 */
  {
    { 1, 1, 1, 1 },
    "bncl8", "bncl", SYN (11), FMT (16), 0x7900,
    & fmt_16_bcl8_ops[0],
    { 2, 0|A(RELAXABLE)|A(COND_CTI), { (1<<MACH_M32RX), PIPE_O } }
  },
/* end-sanitize-m32rx */
/* start-sanitize-m32rx */
/* bncl.s $disp8 */
  {
    { 1, 1, 1, 1 },
    "bncl8.s", "bncl.s", SYN (11), FMT (9), 0x7900,
    0,
    { 2, 0|A(ALIAS)|A(COND_CTI), { (1<<MACH_M32RX), PIPE_O } }
  },
/* end-sanitize-m32rx */
/* start-sanitize-m32rx */
/* bncl $disp24 */
  {
    { 1, 1, 1, 1 },
    "bncl24", "bncl", SYN (12), FMT (17), 0xf9000000,
    & fmt_17_bcl24_ops[0],
    { 2, 0|A(RELAX)|A(COND_CTI), { (1<<MACH_M32RX), PIPE_NONE } }
  },
/* end-sanitize-m32rx */
/* start-sanitize-m32rx */
/* bncl.l $disp24 */
  {
    { 1, 1, 1, 1 },
    "bncl24.l", "bncl.l", SYN (12), FMT (11), 0xf9000000,
    0,
    { 2, 0|A(ALIAS)|A(COND_CTI), { (1<<MACH_M32RX), PIPE_NONE } }
  },
/* end-sanitize-m32rx */
/* cmp $src1,$src2 */
  {
    { 1, 1, 1, 1 },
    "cmp", "cmp", SYN (15), FMT (20), 0x40,
    & fmt_20_cmp_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_OS } }
  },
/* cmpi $src2,#$simm16 */
  {
    { 1, 1, 1, 1 },
    "cmpi", "cmpi", SYN (16), FMT (21), 0x80400000,
    & fmt_21_cmpi_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_NONE } }
  },
/* cmpi $src2,$simm16 */
  {
    { 1, 1, 1, 1 },
    "cmpi.a", "cmpi", SYN (17), FMT (21), 0x80400000,
    & fmt_21_cmpi_ops[0],
    { 2, 0|A(ALIAS), { (1<<MACH_M32R), PIPE_NONE } }
  },
/* cmpu $src1,$src2 */
  {
    { 1, 1, 1, 1 },
    "cmpu", "cmpu", SYN (15), FMT (20), 0x50,
    & fmt_20_cmp_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_OS } }
  },
/* cmpui $src2,#$uimm16 */
  {
    { 1, 1, 1, 1 },
    "cmpui", "cmpui", SYN (18), FMT (22), 0x80500000,
    & fmt_22_cmpui_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_NONE } }
  },
/* cmpui $src2,$uimm16 */
  {
    { 1, 1, 1, 1 },
    "cmpui.a", "cmpui", SYN (19), FMT (22), 0x80500000,
    & fmt_22_cmpui_ops[0],
    { 2, 0|A(ALIAS), { (1<<MACH_M32R), PIPE_NONE } }
  },
/* start-sanitize-m32rx */
/* cmpeq $src1,$src2 */
  {
    { 1, 1, 1, 1 },
    "cmpeq", "cmpeq", SYN (15), FMT (20), 0x60,
    & fmt_20_cmp_ops[0],
    { 2, 0, { (1<<MACH_M32RX), PIPE_OS } }
  },
/* end-sanitize-m32rx */
/* start-sanitize-m32rx */
/* cmpz $src2 */
  {
    { 1, 1, 1, 1 },
    "cmpz", "cmpz", SYN (20), FMT (23), 0x70,
    & fmt_23_cmpz_ops[0],
    { 2, 0, { (1<<MACH_M32RX), PIPE_OS } }
  },
/* end-sanitize-m32rx */
/* div $dr,$sr */
  {
    { 1, 1, 1, 1 },
    "div", "div", SYN (0), FMT (24), 0x90000000,
    & fmt_24_div_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_NONE } }
  },
/* divu $dr,$sr */
  {
    { 1, 1, 1, 1 },
    "divu", "divu", SYN (0), FMT (24), 0x90100000,
    & fmt_24_div_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_NONE } }
  },
/* rem $dr,$sr */
  {
    { 1, 1, 1, 1 },
    "rem", "rem", SYN (0), FMT (24), 0x90200000,
    & fmt_24_div_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_NONE } }
  },
/* remu $dr,$sr */
  {
    { 1, 1, 1, 1 },
    "remu", "remu", SYN (0), FMT (24), 0x90300000,
    & fmt_24_div_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_NONE } }
  },
/* start-sanitize-m32rx */
/* divh $dr,$sr */
  {
    { 1, 1, 1, 1 },
    "divh", "divh", SYN (0), FMT (24), 0x90000010,
    & fmt_24_div_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_NONE } }
  },
/* end-sanitize-m32rx */
/* start-sanitize-m32rx */
/* jc $sr */
  {
    { 1, 1, 1, 1 },
    "jc", "jc", SYN (21), FMT (25), 0x1cc0,
    & fmt_25_jc_ops[0],
    { 2, 0|A(COND_CTI), { (1<<MACH_M32RX), PIPE_O } }
  },
/* end-sanitize-m32rx */
/* start-sanitize-m32rx */
/* jnc $sr */
  {
    { 1, 1, 1, 1 },
    "jnc", "jnc", SYN (21), FMT (25), 0x1dc0,
    & fmt_25_jc_ops[0],
    { 2, 0|A(COND_CTI), { (1<<MACH_M32RX), PIPE_O } }
  },
/* end-sanitize-m32rx */
/* jl $sr */
  {
    { 1, 1, 1, 1 },
    "jl", "jl", SYN (21), FMT (26), 0x1ec0,
    & fmt_26_jl_ops[0],
    { 2, 0|A(FILL_SLOT)|A(UNCOND_CTI), { (1<<MACH_M32R), PIPE_O } }
  },
/* jmp $sr */
  {
    { 1, 1, 1, 1 },
    "jmp", "jmp", SYN (21), FMT (27), 0x1fc0,
    & fmt_27_jmp_ops[0],
    { 2, 0|A(UNCOND_CTI), { (1<<MACH_M32R), PIPE_O } }
  },
/* ld $dr,@$sr */
  {
    { 1, 1, 1, 1 },
    "ld", "ld", SYN (22), FMT (28), 0x20c0,
    & fmt_28_ld_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_O } }
  },
/* ld $dr,@($sr) */
  {
    { 1, 1, 1, 1 },
    "ld-2", "ld", SYN (23), FMT (29), 0x20c0,
    0,
    { 2, 0|A(ALIAS), { (1<<MACH_M32R), PIPE_O } }
  },
/* ld $dr,@($slo16,$sr) */
  {
    { 1, 1, 1, 1 },
    "ld-d", "ld", SYN (24), FMT (30), 0xa0c00000,
    & fmt_30_ld_d_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_NONE } }
  },
/* ld $dr,@($sr,$slo16) */
  {
    { 1, 1, 1, 1 },
    "ld-d2", "ld", SYN (25), FMT (31), 0xa0c00000,
    0,
    { 2, 0|A(ALIAS), { (1<<MACH_M32R), PIPE_NONE } }
  },
/* ldb $dr,@$sr */
  {
    { 1, 1, 1, 1 },
    "ldb", "ldb", SYN (22), FMT (32), 0x2080,
    & fmt_32_ldb_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_O } }
  },
/* ldb $dr,@($sr) */
  {
    { 1, 1, 1, 1 },
    "ldb-2", "ldb", SYN (23), FMT (29), 0x2080,
    0,
    { 2, 0|A(ALIAS), { (1<<MACH_M32R), PIPE_O } }
  },
/* ldb $dr,@($slo16,$sr) */
  {
    { 1, 1, 1, 1 },
    "ldb-d", "ldb", SYN (24), FMT (33), 0xa0800000,
    & fmt_33_ldb_d_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_NONE } }
  },
/* ldb $dr,@($sr,$slo16) */
  {
    { 1, 1, 1, 1 },
    "ldb-d2", "ldb", SYN (25), FMT (31), 0xa0800000,
    0,
    { 2, 0|A(ALIAS), { (1<<MACH_M32R), PIPE_NONE } }
  },
/* ldh $dr,@$sr */
  {
    { 1, 1, 1, 1 },
    "ldh", "ldh", SYN (22), FMT (34), 0x20a0,
    & fmt_34_ldh_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_O } }
  },
/* ldh $dr,@($sr) */
  {
    { 1, 1, 1, 1 },
    "ldh-2", "ldh", SYN (23), FMT (29), 0x20a0,
    0,
    { 2, 0|A(ALIAS), { (1<<MACH_M32R), PIPE_O } }
  },
/* ldh $dr,@($slo16,$sr) */
  {
    { 1, 1, 1, 1 },
    "ldh-d", "ldh", SYN (24), FMT (35), 0xa0a00000,
    & fmt_35_ldh_d_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_NONE } }
  },
/* ldh $dr,@($sr,$slo16) */
  {
    { 1, 1, 1, 1 },
    "ldh-d2", "ldh", SYN (25), FMT (31), 0xa0a00000,
    0,
    { 2, 0|A(ALIAS), { (1<<MACH_M32R), PIPE_NONE } }
  },
/* ldub $dr,@$sr */
  {
    { 1, 1, 1, 1 },
    "ldub", "ldub", SYN (22), FMT (32), 0x2090,
    & fmt_32_ldb_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_O } }
  },
/* ldub $dr,@($sr) */
  {
    { 1, 1, 1, 1 },
    "ldub-2", "ldub", SYN (23), FMT (29), 0x2090,
    0,
    { 2, 0|A(ALIAS), { (1<<MACH_M32R), PIPE_O } }
  },
/* ldub $dr,@($slo16,$sr) */
  {
    { 1, 1, 1, 1 },
    "ldub-d", "ldub", SYN (24), FMT (33), 0xa0900000,
    & fmt_33_ldb_d_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_NONE } }
  },
/* ldub $dr,@($sr,$slo16) */
  {
    { 1, 1, 1, 1 },
    "ldub-d2", "ldub", SYN (25), FMT (31), 0xa0900000,
    0,
    { 2, 0|A(ALIAS), { (1<<MACH_M32R), PIPE_NONE } }
  },
/* lduh $dr,@$sr */
  {
    { 1, 1, 1, 1 },
    "lduh", "lduh", SYN (22), FMT (34), 0x20b0,
    & fmt_34_ldh_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_O } }
  },
/* lduh $dr,@($sr) */
  {
    { 1, 1, 1, 1 },
    "lduh-2", "lduh", SYN (23), FMT (29), 0x20b0,
    0,
    { 2, 0|A(ALIAS), { (1<<MACH_M32R), PIPE_O } }
  },
/* lduh $dr,@($slo16,$sr) */
  {
    { 1, 1, 1, 1 },
    "lduh-d", "lduh", SYN (24), FMT (35), 0xa0b00000,
    & fmt_35_ldh_d_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_NONE } }
  },
/* lduh $dr,@($sr,$slo16) */
  {
    { 1, 1, 1, 1 },
    "lduh-d2", "lduh", SYN (25), FMT (31), 0xa0b00000,
    0,
    { 2, 0|A(ALIAS), { (1<<MACH_M32R), PIPE_NONE } }
  },
/* ld $dr,@$sr+ */
  {
    { 1, 1, 1, 1 },
    "ld-plus", "ld", SYN (26), FMT (36), 0x20e0,
    & fmt_36_ld_plus_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_O } }
  },
/* ld24 $dr,#$uimm24 */
  {
    { 1, 1, 1, 1 },
    "ld24", "ld24", SYN (27), FMT (37), 0xe0000000,
    & fmt_37_ld24_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_NONE } }
  },
/* ld24 $dr,$uimm24 */
  {
    { 1, 1, 1, 1 },
    "ld24.a", "ld24", SYN (28), FMT (37), 0xe0000000,
    & fmt_37_ld24_ops[0],
    { 2, 0|A(ALIAS), { (1<<MACH_M32R), PIPE_NONE } }
  },
/* ldi $dr,#$simm8 */
  {
    { 1, 1, 1, 1 },
    "ldi8", "ldi", SYN (7), FMT (38), 0x6000,
    & fmt_38_ldi8_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_OS } }
  },
/* ldi $dr,$simm8 */
  {
    { 1, 1, 1, 1 },
    "ldi8.a", "ldi", SYN (8), FMT (38), 0x6000,
    & fmt_38_ldi8_ops[0],
    { 2, 0|A(ALIAS), { (1<<MACH_M32R), PIPE_OS } }
  },
/* ldi8 $dr,#$simm8 */
  {
    { 1, 1, 1, 1 },
    "ldi8a", "ldi8", SYN (7), FMT (38), 0x6000,
    & fmt_38_ldi8_ops[0],
    { 2, 0|A(ALIAS), { (1<<MACH_M32R), PIPE_OS } }
  },
/* ldi8 $dr,$simm8 */
  {
    { 1, 1, 1, 1 },
    "ldi8a.a", "ldi8", SYN (8), FMT (38), 0x6000,
    & fmt_38_ldi8_ops[0],
    { 2, 0|A(ALIAS), { (1<<MACH_M32R), PIPE_OS } }
  },
/* ldi $dr,$slo16 */
  {
    { 1, 1, 1, 1 },
    "ldi16", "ldi", SYN (29), FMT (39), 0x90f00000,
    & fmt_39_ldi16_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_NONE } }
  },
/* ldi16 $dr,$slo16 */
  {
    { 1, 1, 1, 1 },
    "ldi16a", "ldi16", SYN (29), FMT (39), 0x90f00000,
    & fmt_39_ldi16_ops[0],
    { 2, 0|A(ALIAS), { (1<<MACH_M32R), PIPE_NONE } }
  },
/* lock $dr,@$sr */
  {
    { 1, 1, 1, 1 },
    "lock", "lock", SYN (22), FMT (40), 0x20d0,
    & fmt_40_lock_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_O } }
  },
/* machi $src1,$src2 */
  {
    { 1, 1, 1, 1 },
    "machi", "machi", SYN (15), FMT (41), 0x3040,
    & fmt_41_machi_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_S } }
  },
/* start-sanitize-m32rx */
/* machi $src1,$src2,$acc */
  {
    { 1, 1, 1, 1 },
    "machi-a", "machi", SYN (30), FMT (42), 0x3040,
    & fmt_42_machi_a_ops[0],
    { 2, 0, { (1<<MACH_M32RX), PIPE_S } }
  },
/* end-sanitize-m32rx */
/* maclo $src1,$src2 */
  {
    { 1, 1, 1, 1 },
    "maclo", "maclo", SYN (15), FMT (41), 0x3050,
    & fmt_41_machi_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_S } }
  },
/* start-sanitize-m32rx */
/* maclo $src1,$src2,$acc */
  {
    { 1, 1, 1, 1 },
    "maclo-a", "maclo", SYN (30), FMT (42), 0x3050,
    & fmt_42_machi_a_ops[0],
    { 2, 0, { (1<<MACH_M32RX), PIPE_S } }
  },
/* end-sanitize-m32rx */
/* macwhi $src1,$src2 */
  {
    { 1, 1, 1, 1 },
    "macwhi", "macwhi", SYN (15), FMT (41), 0x3060,
    & fmt_41_machi_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_S } }
  },
/* macwlo $src1,$src2 */
  {
    { 1, 1, 1, 1 },
    "macwlo", "macwlo", SYN (15), FMT (41), 0x3070,
    & fmt_41_machi_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_S } }
  },
/* mul $dr,$sr */
  {
    { 1, 1, 1, 1 },
    "mul", "mul", SYN (0), FMT (0), 0x1060,
    & fmt_0_add_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_S } }
  },
/* mulhi $src1,$src2 */
  {
    { 1, 1, 1, 1 },
    "mulhi", "mulhi", SYN (15), FMT (43), 0x3000,
    & fmt_43_mulhi_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_S } }
  },
/* start-sanitize-m32rx */
/* mulhi $src1,$src2,$acc */
  {
    { 1, 1, 1, 1 },
    "mulhi-a", "mulhi", SYN (30), FMT (44), 0x3000,
    & fmt_44_mulhi_a_ops[0],
    { 2, 0, { (1<<MACH_M32RX), PIPE_S } }
  },
/* end-sanitize-m32rx */
/* mullo $src1,$src2 */
  {
    { 1, 1, 1, 1 },
    "mullo", "mullo", SYN (15), FMT (43), 0x3010,
    & fmt_43_mulhi_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_S } }
  },
/* start-sanitize-m32rx */
/* mullo $src1,$src2,$acc */
  {
    { 1, 1, 1, 1 },
    "mullo-a", "mullo", SYN (30), FMT (44), 0x3010,
    & fmt_44_mulhi_a_ops[0],
    { 2, 0, { (1<<MACH_M32RX), PIPE_S } }
  },
/* end-sanitize-m32rx */
/* mulwhi $src1,$src2 */
  {
    { 1, 1, 1, 1 },
    "mulwhi", "mulwhi", SYN (15), FMT (43), 0x3020,
    & fmt_43_mulhi_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_S } }
  },
/* mulwlo $src1,$src2 */
  {
    { 1, 1, 1, 1 },
    "mulwlo", "mulwlo", SYN (15), FMT (43), 0x3030,
    & fmt_43_mulhi_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_S } }
  },
/* mv $dr,$sr */
  {
    { 1, 1, 1, 1 },
    "mv", "mv", SYN (0), FMT (45), 0x1080,
    & fmt_45_mv_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_OS } }
  },
/* mvfachi $dr */
  {
    { 1, 1, 1, 1 },
    "mvfachi", "mvfachi", SYN (31), FMT (46), 0x50f0,
    & fmt_46_mvfachi_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_S } }
  },
/* start-sanitize-m32rx */
/* mvfachi $dr,$accs */
  {
    { 1, 1, 1, 1 },
    "mvfachi-a", "mvfachi", SYN (32), FMT (47), 0x50f0,
    & fmt_47_mvfachi_a_ops[0],
    { 2, 0, { (1<<MACH_M32RX), PIPE_S } }
  },
/* end-sanitize-m32rx */
/* mvfaclo $dr */
  {
    { 1, 1, 1, 1 },
    "mvfaclo", "mvfaclo", SYN (31), FMT (46), 0x50f1,
    & fmt_46_mvfachi_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_S } }
  },
/* start-sanitize-m32rx */
/* mvfaclo $dr,$accs */
  {
    { 1, 1, 1, 1 },
    "mvfaclo-a", "mvfaclo", SYN (32), FMT (47), 0x50f1,
    & fmt_47_mvfachi_a_ops[0],
    { 2, 0, { (1<<MACH_M32RX), PIPE_S } }
  },
/* end-sanitize-m32rx */
/* mvfacmi $dr */
  {
    { 1, 1, 1, 1 },
    "mvfacmi", "mvfacmi", SYN (31), FMT (46), 0x50f2,
    & fmt_46_mvfachi_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_S } }
  },
/* start-sanitize-m32rx */
/* mvfacmi $dr,$accs */
  {
    { 1, 1, 1, 1 },
    "mvfacmi-a", "mvfacmi", SYN (32), FMT (47), 0x50f2,
    & fmt_47_mvfachi_a_ops[0],
    { 2, 0, { (1<<MACH_M32RX), PIPE_S } }
  },
/* end-sanitize-m32rx */
/* mvfc $dr,$scr */
  {
    { 1, 1, 1, 1 },
    "mvfc", "mvfc", SYN (33), FMT (48), 0x1090,
    & fmt_48_mvfc_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_O } }
  },
/* mvtachi $src1 */
  {
    { 1, 1, 1, 1 },
    "mvtachi", "mvtachi", SYN (34), FMT (49), 0x5070,
    & fmt_49_mvtachi_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_S } }
  },
/* start-sanitize-m32rx */
/* mvtachi $src1,$accs */
  {
    { 1, 1, 1, 1 },
    "mvtachi-a", "mvtachi", SYN (35), FMT (50), 0x5070,
    & fmt_50_mvtachi_a_ops[0],
    { 2, 0, { (1<<MACH_M32RX), PIPE_S } }
  },
/* end-sanitize-m32rx */
/* mvtaclo $src1 */
  {
    { 1, 1, 1, 1 },
    "mvtaclo", "mvtaclo", SYN (34), FMT (49), 0x5071,
    & fmt_49_mvtachi_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_S } }
  },
/* start-sanitize-m32rx */
/* mvtaclo $src1,$accs */
  {
    { 1, 1, 1, 1 },
    "mvtaclo-a", "mvtaclo", SYN (35), FMT (50), 0x5071,
    & fmt_50_mvtachi_a_ops[0],
    { 2, 0, { (1<<MACH_M32RX), PIPE_S } }
  },
/* end-sanitize-m32rx */
/* mvtc $sr,$dcr */
  {
    { 1, 1, 1, 1 },
    "mvtc", "mvtc", SYN (36), FMT (51), 0x10a0,
    & fmt_51_mvtc_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_O } }
  },
/* neg $dr,$sr */
  {
    { 1, 1, 1, 1 },
    "neg", "neg", SYN (0), FMT (45), 0x30,
    & fmt_45_mv_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_OS } }
  },
/* nop */
  {
    { 1, 1, 1, 1 },
    "nop", "nop", SYN (37), FMT (52), 0x7000,
    0,
    { 2, 0, { (1<<MACH_M32R), PIPE_OS } }
  },
/* not $dr,$sr */
  {
    { 1, 1, 1, 1 },
    "not", "not", SYN (0), FMT (45), 0xb0,
    & fmt_45_mv_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_OS } }
  },
/* rac */
  {
    { 1, 1, 1, 1 },
    "rac", "rac", SYN (37), FMT (53), 0x5090,
    & fmt_53_rac_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_S } }
  },
/* start-sanitize-m32rx */
/* rac $accd */
  {
    { 1, 1, 1, 1 },
    "rac-d", "rac", SYN (38), FMT (54), 0x5090,
    & fmt_54_rac_d_ops[0],
    { 2, 0, { (1<<MACH_M32RX), PIPE_S } }
  },
/* end-sanitize-m32rx */
/* start-sanitize-m32rx */
/* rac $accd,$accs */
  {
    { 1, 1, 1, 1 },
    "rac-ds", "rac", SYN (39), FMT (55), 0x5090,
    & fmt_55_rac_ds_ops[0],
    { 2, 0, { (1<<MACH_M32RX), PIPE_S } }
  },
/* end-sanitize-m32rx */
/* start-sanitize-m32rx */
/* rac $accd,$accs,#$imm1 */
  {
    { 1, 1, 1, 1 },
    "rac-dsi", "rac", SYN (40), FMT (56), 0x5090,
    & fmt_56_rac_dsi_ops[0],
    { 2, 0, { (1<<MACH_M32RX), PIPE_S } }
  },
/* end-sanitize-m32rx */
/* rach */
  {
    { 1, 1, 1, 1 },
    "rach", "rach", SYN (37), FMT (53), 0x5080,
    & fmt_53_rac_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_S } }
  },
/* start-sanitize-m32rx */
/* rach $accd */
  {
    { 1, 1, 1, 1 },
    "rach-d", "rach", SYN (38), FMT (54), 0x5080,
    & fmt_54_rac_d_ops[0],
    { 2, 0, { (1<<MACH_M32RX), PIPE_S } }
  },
/* end-sanitize-m32rx */
/* start-sanitize-m32rx */
/* rach $accd,$accs */
  {
    { 1, 1, 1, 1 },
    "rach-ds", "rach", SYN (39), FMT (55), 0x5080,
    & fmt_55_rac_ds_ops[0],
    { 2, 0, { (1<<MACH_M32RX), PIPE_S } }
  },
/* end-sanitize-m32rx */
/* start-sanitize-m32rx */
/* rach $accd,$accs,#$imm1 */
  {
    { 1, 1, 1, 1 },
    "rach-dsi", "rach", SYN (40), FMT (56), 0x5080,
    & fmt_56_rac_dsi_ops[0],
    { 2, 0, { (1<<MACH_M32RX), PIPE_S } }
  },
/* end-sanitize-m32rx */
/* rte */
  {
    { 1, 1, 1, 1 },
    "rte", "rte", SYN (37), FMT (57), 0x10d6,
    & fmt_57_rte_ops[0],
    { 2, 0|A(UNCOND_CTI), { (1<<MACH_M32R), PIPE_O } }
  },
/* seth $dr,#$hi16 */
  {
    { 1, 1, 1, 1 },
    "seth", "seth", SYN (41), FMT (58), 0xd0c00000,
    & fmt_58_seth_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_NONE } }
  },
/* seth $dr,$hi16 */
  {
    { 1, 1, 1, 1 },
    "seth.a", "seth", SYN (42), FMT (58), 0xd0c00000,
    & fmt_58_seth_ops[0],
    { 2, 0|A(ALIAS), { (1<<MACH_M32R), PIPE_NONE } }
  },
/* sll $dr,$sr */
  {
    { 1, 1, 1, 1 },
    "sll", "sll", SYN (0), FMT (0), 0x1040,
    & fmt_0_add_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_O } }
  },
/* sll3 $dr,$sr,#$simm16 */
  {
    { 1, 1, 1, 1 },
    "sll3", "sll3", SYN (9), FMT (59), 0x90c00000,
    & fmt_59_sll3_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_NONE } }
  },
/* sll3 $dr,$sr,$simm16 */
  {
    { 1, 1, 1, 1 },
    "sll3.a", "sll3", SYN (10), FMT (59), 0x90c00000,
    & fmt_59_sll3_ops[0],
    { 2, 0|A(ALIAS), { (1<<MACH_M32R), PIPE_NONE } }
  },
/* slli $dr,#$uimm5 */
  {
    { 1, 1, 1, 1 },
    "slli", "slli", SYN (43), FMT (60), 0x5040,
    & fmt_60_slli_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_O } }
  },
/* slli $dr,$uimm5 */
  {
    { 1, 1, 1, 1 },
    "slli.a", "slli", SYN (44), FMT (60), 0x5040,
    & fmt_60_slli_ops[0],
    { 2, 0|A(ALIAS), { (1<<MACH_M32R), PIPE_O } }
  },
/* sra $dr,$sr */
  {
    { 1, 1, 1, 1 },
    "sra", "sra", SYN (0), FMT (0), 0x1020,
    & fmt_0_add_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_O } }
  },
/* sra3 $dr,$sr,#$simm16 */
  {
    { 1, 1, 1, 1 },
    "sra3", "sra3", SYN (9), FMT (59), 0x90a00000,
    & fmt_59_sll3_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_NONE } }
  },
/* sra3 $dr,$sr,$simm16 */
  {
    { 1, 1, 1, 1 },
    "sra3.a", "sra3", SYN (10), FMT (59), 0x90a00000,
    & fmt_59_sll3_ops[0],
    { 2, 0|A(ALIAS), { (1<<MACH_M32R), PIPE_NONE } }
  },
/* srai $dr,#$uimm5 */
  {
    { 1, 1, 1, 1 },
    "srai", "srai", SYN (43), FMT (60), 0x5020,
    & fmt_60_slli_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_O } }
  },
/* srai $dr,$uimm5 */
  {
    { 1, 1, 1, 1 },
    "srai.a", "srai", SYN (44), FMT (60), 0x5020,
    & fmt_60_slli_ops[0],
    { 2, 0|A(ALIAS), { (1<<MACH_M32R), PIPE_O } }
  },
/* srl $dr,$sr */
  {
    { 1, 1, 1, 1 },
    "srl", "srl", SYN (0), FMT (0), 0x1000,
    & fmt_0_add_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_O } }
  },
/* srl3 $dr,$sr,#$simm16 */
  {
    { 1, 1, 1, 1 },
    "srl3", "srl3", SYN (9), FMT (59), 0x90800000,
    & fmt_59_sll3_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_NONE } }
  },
/* srl3 $dr,$sr,$simm16 */
  {
    { 1, 1, 1, 1 },
    "srl3.a", "srl3", SYN (10), FMT (59), 0x90800000,
    & fmt_59_sll3_ops[0],
    { 2, 0|A(ALIAS), { (1<<MACH_M32R), PIPE_NONE } }
  },
/* srli $dr,#$uimm5 */
  {
    { 1, 1, 1, 1 },
    "srli", "srli", SYN (43), FMT (60), 0x5000,
    & fmt_60_slli_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_O } }
  },
/* srli $dr,$uimm5 */
  {
    { 1, 1, 1, 1 },
    "srli.a", "srli", SYN (44), FMT (60), 0x5000,
    & fmt_60_slli_ops[0],
    { 2, 0|A(ALIAS), { (1<<MACH_M32R), PIPE_O } }
  },
/* st $src1,@$src2 */
  {
    { 1, 1, 1, 1 },
    "st", "st", SYN (45), FMT (61), 0x2040,
    & fmt_61_st_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_O } }
  },
/* st $src1,@($src2) */
  {
    { 1, 1, 1, 1 },
    "st-2", "st", SYN (46), FMT (62), 0x2040,
    0,
    { 2, 0|A(ALIAS), { (1<<MACH_M32R), PIPE_O } }
  },
/* st $src1,@($slo16,$src2) */
  {
    { 1, 1, 1, 1 },
    "st-d", "st", SYN (47), FMT (63), 0xa0400000,
    & fmt_63_st_d_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_NONE } }
  },
/* st $src1,@($src2,$slo16) */
  {
    { 1, 1, 1, 1 },
    "st-d2", "st", SYN (48), FMT (64), 0xa0400000,
    0,
    { 2, 0|A(ALIAS), { (1<<MACH_M32R), PIPE_NONE } }
  },
/* stb $src1,@$src2 */
  {
    { 1, 1, 1, 1 },
    "stb", "stb", SYN (45), FMT (65), 0x2000,
    & fmt_65_stb_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_O } }
  },
/* stb $src1,@($src2) */
  {
    { 1, 1, 1, 1 },
    "stb-2", "stb", SYN (46), FMT (62), 0x2000,
    0,
    { 2, 0|A(ALIAS), { (1<<MACH_M32R), PIPE_O } }
  },
/* stb $src1,@($slo16,$src2) */
  {
    { 1, 1, 1, 1 },
    "stb-d", "stb", SYN (47), FMT (66), 0xa0000000,
    & fmt_66_stb_d_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_NONE } }
  },
/* stb $src1,@($src2,$slo16) */
  {
    { 1, 1, 1, 1 },
    "stb-d2", "stb", SYN (48), FMT (64), 0xa0000000,
    0,
    { 2, 0|A(ALIAS), { (1<<MACH_M32R), PIPE_NONE } }
  },
/* sth $src1,@$src2 */
  {
    { 1, 1, 1, 1 },
    "sth", "sth", SYN (45), FMT (67), 0x2020,
    & fmt_67_sth_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_O } }
  },
/* sth $src1,@($src2) */
  {
    { 1, 1, 1, 1 },
    "sth-2", "sth", SYN (46), FMT (62), 0x2020,
    0,
    { 2, 0|A(ALIAS), { (1<<MACH_M32R), PIPE_O } }
  },
/* sth $src1,@($slo16,$src2) */
  {
    { 1, 1, 1, 1 },
    "sth-d", "sth", SYN (47), FMT (68), 0xa0200000,
    & fmt_68_sth_d_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_NONE } }
  },
/* sth $src1,@($src2,$slo16) */
  {
    { 1, 1, 1, 1 },
    "sth-d2", "sth", SYN (48), FMT (64), 0xa0200000,
    0,
    { 2, 0|A(ALIAS), { (1<<MACH_M32R), PIPE_NONE } }
  },
/* st $src1,@+$src2 */
  {
    { 1, 1, 1, 1 },
    "st-plus", "st", SYN (49), FMT (69), 0x2060,
    & fmt_69_st_plus_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_O } }
  },
/* st $src1,@-$src2 */
  {
    { 1, 1, 1, 1 },
    "st-minus", "st", SYN (50), FMT (69), 0x2070,
    & fmt_69_st_plus_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_O } }
  },
/* sub $dr,$sr */
  {
    { 1, 1, 1, 1 },
    "sub", "sub", SYN (0), FMT (0), 0x20,
    & fmt_0_add_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_OS } }
  },
/* subv $dr,$sr */
  {
    { 1, 1, 1, 1 },
    "subv", "subv", SYN (0), FMT (5), 0x0,
    & fmt_5_addv_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_OS } }
  },
/* subx $dr,$sr */
  {
    { 1, 1, 1, 1 },
    "subx", "subx", SYN (0), FMT (7), 0x10,
    & fmt_7_addx_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_OS } }
  },
/* trap #$uimm4 */
  {
    { 1, 1, 1, 1 },
    "trap", "trap", SYN (51), FMT (70), 0x10f0,
    & fmt_70_trap_ops[0],
    { 2, 0|A(FILL_SLOT)|A(UNCOND_CTI), { (1<<MACH_M32R), PIPE_O } }
  },
/* trap $uimm4 */
  {
    { 1, 1, 1, 1 },
    "trap.a", "trap", SYN (52), FMT (70), 0x10f0,
    & fmt_70_trap_ops[0],
    { 2, 0|A(ALIAS)|A(FILL_SLOT)|A(UNCOND_CTI), { (1<<MACH_M32R), PIPE_O } }
  },
/* unlock $src1,@$src2 */
  {
    { 1, 1, 1, 1 },
    "unlock", "unlock", SYN (45), FMT (71), 0x2050,
    & fmt_71_unlock_ops[0],
    { 2, 0, { (1<<MACH_M32R), PIPE_O } }
  },
/* push $src1 */
  {
    { 1, 1, 1, 1 },
    "push", "push", SYN (34), FMT (72), 0x207f,
    0,
    { 2, 0|A(ALIAS), { (1<<MACH_M32R), PIPE_NONE } }
  },
/* pop $dr */
  {
    { 1, 1, 1, 1 },
    "pop", "pop", SYN (31), FMT (73), 0x20ef,
    0,
    { 2, 0|A(ALIAS), { (1<<MACH_M32R), PIPE_NONE } }
  },
/* start-sanitize-m32rx */
/* satb $dr,$src2 */
  {
    { 1, 1, 1, 1 },
    "satb", "satb", SYN (53), FMT (74), 0x80000100,
    & fmt_74_satb_ops[0],
    { 2, 0, { (1<<MACH_M32RX), PIPE_NONE } }
  },
/* end-sanitize-m32rx */
/* start-sanitize-m32rx */
/* sath $dr,$src2 */
  {
    { 1, 1, 1, 1 },
    "sath", "sath", SYN (53), FMT (74), 0x80000200,
    & fmt_74_satb_ops[0],
    { 2, 0, { (1<<MACH_M32RX), PIPE_NONE } }
  },
/* end-sanitize-m32rx */
/* start-sanitize-m32rx */
/* sat $dr,$src2 */
  {
    { 1, 1, 1, 1 },
    "sat", "sat", SYN (53), FMT (75), 0x80000000,
    & fmt_75_sat_ops[0],
    { 2, 0, { (1<<MACH_M32RX), PIPE_NONE } }
  },
/* end-sanitize-m32rx */
/* start-sanitize-m32rx */
/* pcmpbz $src2 */
  {
    { 1, 1, 1, 1 },
    "pcmpbz", "pcmpbz", SYN (20), FMT (23), 0x370,
    & fmt_23_cmpz_ops[0],
    { 2, 0, { (1<<MACH_M32RX), PIPE_OS } }
  },
/* end-sanitize-m32rx */
/* start-sanitize-m32rx */
/* sadd */
  {
    { 1, 1, 1, 1 },
    "sadd", "sadd", SYN (37), FMT (76), 0x50e4,
    & fmt_76_sadd_ops[0],
    { 2, 0, { (1<<MACH_M32RX), PIPE_S } }
  },
/* end-sanitize-m32rx */
/* start-sanitize-m32rx */
/* macwu1 $src1,$src2 */
  {
    { 1, 1, 1, 1 },
    "macwu1", "macwu1", SYN (15), FMT (77), 0x50b0,
    & fmt_77_macwu1_ops[0],
    { 2, 0, { (1<<MACH_M32RX), PIPE_S } }
  },
/* end-sanitize-m32rx */
/* start-sanitize-m32rx */
/* msblo $src1,$src2 */
  {
    { 1, 1, 1, 1 },
    "msblo", "msblo", SYN (15), FMT (41), 0x50d0,
    & fmt_41_machi_ops[0],
    { 2, 0, { (1<<MACH_M32RX), PIPE_S } }
  },
/* end-sanitize-m32rx */
/* start-sanitize-m32rx */
/* mulwu1 $src1,$src2 */
  {
    { 1, 1, 1, 1 },
    "mulwu1", "mulwu1", SYN (15), FMT (78), 0x50a0,
    & fmt_78_mulwu1_ops[0],
    { 2, 0, { (1<<MACH_M32RX), PIPE_S } }
  },
/* end-sanitize-m32rx */
/* start-sanitize-m32rx */
/* maclh1 $src1,$src2 */
  {
    { 1, 1, 1, 1 },
    "maclh1", "maclh1", SYN (15), FMT (77), 0x50c0,
    & fmt_77_macwu1_ops[0],
    { 2, 0, { (1<<MACH_M32RX), PIPE_S } }
  },
/* end-sanitize-m32rx */
/* start-sanitize-m32rx */
/* sc */
  {
    { 1, 1, 1, 1 },
    "sc", "sc", SYN (37), FMT (79), 0x7401,
    & fmt_79_sc_ops[0],
    { 2, 0, { (1<<MACH_M32RX), PIPE_O } }
  },
/* end-sanitize-m32rx */
/* start-sanitize-m32rx */
/* snc */
  {
    { 1, 1, 1, 1 },
    "snc", "snc", SYN (37), FMT (79), 0x7501,
    & fmt_79_sc_ops[0],
    { 2, 0, { (1<<MACH_M32RX), PIPE_O } }
  },
/* end-sanitize-m32rx */
};

#undef A
#undef SYN
#undef FMT

CGEN_INSN_TABLE m32r_cgen_insn_table =
{
  & m32r_cgen_insn_table_entries[0],
  sizeof (CGEN_INSN),
  MAX_INSNS,
  NULL,
  m32r_cgen_asm_hash_insn, CGEN_ASM_HASH_SIZE,
  m32r_cgen_dis_hash_insn, CGEN_DIS_HASH_SIZE
};

/* The hash functions are recorded here to help keep assembler code out of
   the disassembler and vice versa.  */

unsigned int
m32r_cgen_asm_hash_insn (insn)
     const char * insn;
{
  return CGEN_ASM_HASH (insn);
}

unsigned int
m32r_cgen_dis_hash_insn (buf, value)
     const char * buf;
     unsigned long value;
{
  return CGEN_DIS_HASH (buf, value);
}

CGEN_OPCODE_DATA m32r_cgen_opcode_data = 
{
  & m32r_cgen_hw_entries[0],
  & m32r_cgen_insn_table,
};

void
m32r_cgen_init_tables (mach)
    int mach;
{
}

/* Main entry point for stuffing values in cgen_fields.  */

CGEN_INLINE void
m32r_cgen_set_operand (opindex, valuep, fields)
     int opindex;
     const long * valuep;
     CGEN_FIELDS * fields;
{
  switch (opindex)
    {
    case M32R_OPERAND_SR :
      fields->f_r2 = * valuep;
      break;
    case M32R_OPERAND_DR :
      fields->f_r1 = * valuep;
      break;
    case M32R_OPERAND_SRC1 :
      fields->f_r1 = * valuep;
      break;
    case M32R_OPERAND_SRC2 :
      fields->f_r2 = * valuep;
      break;
    case M32R_OPERAND_SCR :
      fields->f_r2 = * valuep;
      break;
    case M32R_OPERAND_DCR :
      fields->f_r1 = * valuep;
      break;
    case M32R_OPERAND_SIMM8 :
      fields->f_simm8 = * valuep;
      break;
    case M32R_OPERAND_SIMM16 :
      fields->f_simm16 = * valuep;
      break;
    case M32R_OPERAND_UIMM4 :
      fields->f_uimm4 = * valuep;
      break;
    case M32R_OPERAND_UIMM5 :
      fields->f_uimm5 = * valuep;
      break;
    case M32R_OPERAND_UIMM16 :
      fields->f_uimm16 = * valuep;
      break;
/* start-sanitize-m32rx */
    case M32R_OPERAND_IMM1 :
      fields->f_imm1 = * valuep;
      break;
/* end-sanitize-m32rx */
/* start-sanitize-m32rx */
    case M32R_OPERAND_ACCD :
      fields->f_accd = * valuep;
      break;
/* end-sanitize-m32rx */
/* start-sanitize-m32rx */
    case M32R_OPERAND_ACCS :
      fields->f_accs = * valuep;
      break;
/* end-sanitize-m32rx */
/* start-sanitize-m32rx */
    case M32R_OPERAND_ACC :
      fields->f_acc = * valuep;
      break;
/* end-sanitize-m32rx */
    case M32R_OPERAND_HI16 :
      fields->f_hi16 = * valuep;
      break;
    case M32R_OPERAND_SLO16 :
      fields->f_simm16 = * valuep;
      break;
    case M32R_OPERAND_ULO16 :
      fields->f_uimm16 = * valuep;
      break;
    case M32R_OPERAND_UIMM24 :
      fields->f_uimm24 = * valuep;
      break;
    case M32R_OPERAND_DISP8 :
      fields->f_disp8 = * valuep;
      break;
    case M32R_OPERAND_DISP16 :
      fields->f_disp16 = * valuep;
      break;
    case M32R_OPERAND_DISP24 :
      fields->f_disp24 = * valuep;
      break;

    default :
      fprintf (stderr, "Unrecognized field %d while setting operand.\n",
		       opindex);
      abort ();
  }
}

/* Main entry point for getting values from cgen_fields.  */

CGEN_INLINE long
m32r_cgen_get_operand (opindex, fields)
     int opindex;
     const CGEN_FIELDS * fields;
{
  long value;

  switch (opindex)
    {
    case M32R_OPERAND_SR :
      value = fields->f_r2;
      break;
    case M32R_OPERAND_DR :
      value = fields->f_r1;
      break;
    case M32R_OPERAND_SRC1 :
      value = fields->f_r1;
      break;
    case M32R_OPERAND_SRC2 :
      value = fields->f_r2;
      break;
    case M32R_OPERAND_SCR :
      value = fields->f_r2;
      break;
    case M32R_OPERAND_DCR :
      value = fields->f_r1;
      break;
    case M32R_OPERAND_SIMM8 :
      value = fields->f_simm8;
      break;
    case M32R_OPERAND_SIMM16 :
      value = fields->f_simm16;
      break;
    case M32R_OPERAND_UIMM4 :
      value = fields->f_uimm4;
      break;
    case M32R_OPERAND_UIMM5 :
      value = fields->f_uimm5;
      break;
    case M32R_OPERAND_UIMM16 :
      value = fields->f_uimm16;
      break;
/* start-sanitize-m32rx */
    case M32R_OPERAND_IMM1 :
      value = fields->f_imm1;
      break;
/* end-sanitize-m32rx */
/* start-sanitize-m32rx */
    case M32R_OPERAND_ACCD :
      value = fields->f_accd;
      break;
/* end-sanitize-m32rx */
/* start-sanitize-m32rx */
    case M32R_OPERAND_ACCS :
      value = fields->f_accs;
      break;
/* end-sanitize-m32rx */
/* start-sanitize-m32rx */
    case M32R_OPERAND_ACC :
      value = fields->f_acc;
      break;
/* end-sanitize-m32rx */
    case M32R_OPERAND_HI16 :
      value = fields->f_hi16;
      break;
    case M32R_OPERAND_SLO16 :
      value = fields->f_simm16;
      break;
    case M32R_OPERAND_ULO16 :
      value = fields->f_uimm16;
      break;
    case M32R_OPERAND_UIMM24 :
      value = fields->f_uimm24;
      break;
    case M32R_OPERAND_DISP8 :
      value = fields->f_disp8;
      break;
    case M32R_OPERAND_DISP16 :
      value = fields->f_disp16;
      break;
    case M32R_OPERAND_DISP24 :
      value = fields->f_disp24;
      break;

    default :
      fprintf (stderr, "Unrecognized field %d while getting operand.\n",
		       opindex);
      abort ();
  }

  return value;
}


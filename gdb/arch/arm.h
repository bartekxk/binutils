/* Common target dependent code for GDB on ARM systems.
   Copyright (C) 2002-2015 Free Software Foundation, Inc.

   This file is part of GDB.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#ifndef ARM_H
#define ARM_H

/* Register numbers of various important registers.  */

enum gdb_regnum {
  ARM_A1_REGNUM = 0,		/* first integer-like argument */
  ARM_A4_REGNUM = 3,		/* last integer-like argument */
  ARM_AP_REGNUM = 11,
  ARM_IP_REGNUM = 12,
  ARM_SP_REGNUM = 13,		/* Contains address of top of stack */
  ARM_LR_REGNUM = 14,		/* address to return to from a function call */
  ARM_PC_REGNUM = 15,		/* Contains program counter */
  ARM_F0_REGNUM = 16,		/* first floating point register */
  ARM_F3_REGNUM = 19,		/* last floating point argument register */
  ARM_F7_REGNUM = 23, 		/* last floating point register */
  ARM_FPS_REGNUM = 24,		/* floating point status register */
  ARM_PS_REGNUM = 25,		/* Contains processor status */
  ARM_WR0_REGNUM,		/* WMMX data registers.  */
  ARM_WR15_REGNUM = ARM_WR0_REGNUM + 15,
  ARM_WC0_REGNUM,		/* WMMX control registers.  */
  ARM_WCSSF_REGNUM = ARM_WC0_REGNUM + 2,
  ARM_WCASF_REGNUM = ARM_WC0_REGNUM + 3,
  ARM_WC7_REGNUM = ARM_WC0_REGNUM + 7,
  ARM_WCGR0_REGNUM,		/* WMMX general purpose registers.  */
  ARM_WCGR3_REGNUM = ARM_WCGR0_REGNUM + 3,
  ARM_WCGR7_REGNUM = ARM_WCGR0_REGNUM + 7,
  ARM_D0_REGNUM,		/* VFP double-precision registers.  */
  ARM_D31_REGNUM = ARM_D0_REGNUM + 31,
  ARM_FPSCR_REGNUM,

  ARM_NUM_REGS,

  /* Other useful registers.  */
  ARM_FP_REGNUM = 11,		/* Frame register in ARM code, if used.  */
  THUMB_FP_REGNUM = 7,		/* Frame register in Thumb code, if used.  */
  ARM_NUM_ARG_REGS = 4, 
  ARM_LAST_ARG_REGNUM = ARM_A4_REGNUM,
  ARM_NUM_FP_ARG_REGS = 4,
  ARM_LAST_FP_ARG_REGNUM = ARM_F3_REGNUM
};

#endif

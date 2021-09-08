/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2012 Regents of the University of California
 */

#define CSR_SIE			0x104
#define CSR_SIP			0x144
#define KERNEL_STACK		0x50500000

.section ".entry_function","ax"
.global _start
_start:
  /* Mask all interrupts */
  csrw CSR_SIE, zero
  csrw CSR_SIP, zero

  /* Load the global pointer */
  .option push
  .option norelax
  la gp, __global_pointer$
  .option pop
  
  /* Clear BSS for flat non-ELF images */
  la a0,__bss_start
  la a1,__BSS_END__
clear_BSS_loop:
  addi a0,a0,4
  beq a0,a1,set_up_C
  sw zero,(a0)
  j clear_BSS_loop

  /* setup C environment (set sp register)*/
set_up_C:
  lui sp,%hi(KERNEL_STACK)
  addi sp,sp,%lo(KERNEL_STACK)

  /* Jump to the code in kernel.c*/
  call main

loop:
  wfi
  j loop


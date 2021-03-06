/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2012 Regents of the University of California
 */

#include <asm.h>
#include <csr.h>

.section ".entry_function","ax"
ENTRY(_start)
  /* Mask all interrupts */
  csrw CSR_SIE, zero
  csrw CSR_SIP, zero

  /*
   * Disable FPU to detect illegal usage of
   * floating point in kernel space
   */
  li t0, SR_FS
  csrc CSR_SSTATUS, t0

  /* Load the global pointer */
  .option push
  .option norelax
  la gp, __global_pointer$
  .option pop

  // find core
  csrr t0, CSR_MHARTID
  bnez t0, set_secondary_stack

  /* Clear BSS for flat non-ELF images */
  la a3, __bss_start
  la a4, __BSS_END__
  ble a4, a3, clear_bss_done
clear_bss:
  sd zero, (a3)
  add a3, a3, RISCV_SZPTR
  blt a3, a4, clear_bss
clear_bss_done:

  /* setup C environment */
  ld sp, pid0_stack_core_m
  la tp, pid0_pcb_core_m
  j set_env_finish

set_secondary_stack:
  la tp, pid0_pcb_core_s
  ld sp, pid0_stack_core_s
  csrw CSR_SSCRATCH, x0

set_env_finish:
  la a0, riscv_dtb
  sd a1, (a0)

  mv a0,t0
  call main

loop:
  wfi
  j loop

END(_start)

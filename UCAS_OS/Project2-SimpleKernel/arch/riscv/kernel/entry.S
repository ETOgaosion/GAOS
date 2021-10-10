#include <asm.h>
#include <csr.h>

.macro SAVE_CONTEXT
  .local _restore_kernel_tpsp
  .local _save_context
  /*
   * If coming from userspace, preserve the user thread pointer and load
   * the kernel thread pointer.  If we came from the kernel, sscratch
   * will contain 0, and we should continue on the current TP.
   */
  csrrw tp, CSR_SSCRATCH, tp
  bnez tp, _save_context

_restore_kernel_tpsp:
  csrr tp, CSR_SSCRATCH
  // sd sp, PCB_KERNEL_SP(tp)
_save_context:
  sd sp, PCB_USER_SP(tp)
  ld sp, PCB_KERNEL_SP(tp)
  addi sp, sp, SWITCH_TO_SIZE

  /* TODO: save all general purpose registers here! */
  SAVE(zero,ZERO)
  SAVE(ra,RA)
  // SAVE(sp,SP)
  SAVE(gp,GP)
  // SAVE(tp,TP)
  SAVE(t0,T0)
  SAVE(t1,T1)
  SAVE(t2,T2)
  SAVE(s0,S0)
  SAVE(s1,S1)
  SAVE(a0,A0)
  SAVE(a1,A1)
  SAVE(a2,A2)
  SAVE(a3,A3)
  SAVE(a4,A4)
  SAVE(a5,A5)
  SAVE(a6,A6)
  SAVE(a7,A7)
  SAVE(s2,S2)
  SAVE(s3,S3)
  SAVE(s4,S4)
  SAVE(s5,S5)
  SAVE(s6,S6)
  SAVE(s7,S7)
  SAVE(s8,S8)
  SAVE(s9,S9)
  SAVE(s10,S10)
  SAVE(s11,S11)
  SAVE(t3,T3)
  SAVE(t4,T4)
  SAVE(t5,T5)
  SAVE(t6,T5)

  /*
   * Disable user-mode memory access as it should only be set in the
   * actual user copy routines.
   *
   * Disable the FPU to detect illegal usage of floating point in kernel
   * space.
   */
  li t0, SR_SUM | SR_FS
  csrrc a0,CSR_SSTATUS,t0

  /* TODO: save sstatus, sepc, stval, scause and sscratch on user stack */
  /* ERROR: 1. not user stack, but kernel stack.*/
  csrr a1,CSR_SEPC
  csrr a2,CSR_STVAL
  csrr a3,CSR_SCAUSE
  csrr a4,CSR_SSCRATCH
  SAVE(a0,SSTATUS)
  SAVE(a1,SEPC)
  SAVE(a2,SBADADDR)
  SAVE(a3,SCAUSE)
  SAVE(a4,TP)
.endm

.macro RESTORE_CONTEXT
  /* TODO: Restore all registers and sepc,sstatus */
  ld sp, PCB_KERNEL_SP(tp)
  addi sp, sp, SWITCH_TO_SIZE
  // restore s first
  LOAD(a0,SSTATUS)
  LOAD(a1,SEPC)
  csrw CSR_SSTATUS,a0
  csrw CSR_SEPC,a1
  // restore reg
  LOAD(zero,ZERO)
  LOAD(ra,RA)
  // LOAD(sp,SP)
  LOAD(gp,GP)
  LOAD(t0,T0)
  LOAD(t1,T1)
  LOAD(t2,T2)
  LOAD(s0,S0)
  LOAD(s1,S1)
  LOAD(a0,A0)
  LOAD(a1,A1)
  LOAD(a2,A2)
  LOAD(a3,A3)
  LOAD(a4,A4)
  LOAD(a5,A5)
  LOAD(a6,A6)
  LOAD(a7,A7)
  LOAD(s2,S2)
  LOAD(s3,S3)
  LOAD(s4,S4)
  LOAD(s5,S5)
  LOAD(s6,S6)
  LOAD(s7,S7)
  LOAD(s8,S8)
  LOAD(s9,S9)
  LOAD(s10,S10)
  LOAD(s11,S11)
  LOAD(t3,T3)
  LOAD(t4,T4)
  LOAD(t5,T5)
  LOAD(t6,T5)
  ld sp, PCB_USER_SP(tp)
.endm

ENTRY(enable_preempt)
  ld t1, current_running
  ld t0, PCB_PREEMPT_COUNT(t1)
  beq t0, zero, do_enable
  addi t0, t0, -1
  sd t0, PCB_PREEMPT_COUNT(t1)
  beq t0, zero, do_enable
  jr ra
do_enable:
  not t0, x0
  csrs CSR_SIE, t0
  jr ra
ENDPROC(enable_preempt)

ENTRY(disable_preempt)
  csrw CSR_SIE, zero
  ld t1, current_running
  ld t0, PCB_PREEMPT_COUNT(t1)
  addi t0, t0, 1
  sd t0, PCB_PREEMPT_COUNT(t1)
  jr ra
ENDPROC(disable_preempt)

ENTRY(enable_interrupt)
  li t0, SR_SIE
  csrs CSR_SSTATUS, t0
  jr ra
ENDPROC(enable_interrupt)

ENTRY(disable_interrupt)
  li t0, SR_SIE
  csrs CSR_SSTATUS, t0
  jr ra
ENDPROC(disable_interrupt)

// the address of previous pcb in a0
// the address of next pcb in a1
ENTRY(switch_to)
  // save all callee save registers on kernel stack
  mv t0,sp
  ld sp, PCB_KERNEL_SP(a0)
  /* TODO: store all callee save registers,
   * see the definition of `struct switchto_context` in sched.h*/
  SAVE_SWITCH(ra,RA)
  SAVE_SWITCH(t0,SP)  //sp
  SAVE_SWITCH(s0,S0)
  SAVE_SWITCH(s1,S1)
  SAVE_SWITCH(s2,S2)
  SAVE_SWITCH(s3,S3)
  SAVE_SWITCH(s4,S4)
  SAVE_SWITCH(s5,S5)
  SAVE_SWITCH(s6,S6)
  SAVE_SWITCH(s7,S7)
  SAVE_SWITCH(s8,S8)
  SAVE_SWITCH(s9,S9)
  SAVE_SWITCH(s10,S10)
  SAVE_SWITCH(s11,S11)
  
  // restore next
  /* TODO: restore all callee save registers,
   * see the definition of `struct switchto_context` in sched.h*/
  ld sp,PCB_KERNEL_SP(a1)
  LOAD_SWITCH(ra,RA)
  LOAD_SWITCH(s0,S0)
  LOAD_SWITCH(s1,S1)
  LOAD_SWITCH(s2,S2)
  LOAD_SWITCH(s3,S3)
  LOAD_SWITCH(s4,S4)
  LOAD_SWITCH(s5,S5)
  LOAD_SWITCH(s6,S6)
  LOAD_SWITCH(s7,S7)
  LOAD_SWITCH(s8,S8)
  LOAD_SWITCH(s9,S9)
  LOAD_SWITCH(s10,S10)
  LOAD_SWITCH(s11,S11)
  LOAD_SWITCH(sp,SP)
  mv tp,a1
  jr ra

ENDPROC(switch_to)

ENTRY(ret_from_exception)
  /* TODO: */
  RESTORE_CONTEXT
  csrw CSR_SSCRATCH,tp
  //ret
  sret
ENDPROC(ret_from_exception)

ENTRY(exception_handler_entry)
  SAVE_CONTEXT

  csrw CSR_SSCRATCH, x0

  /* Load the global pointer */
  .option push
  .option norelax
  la gp, __global_pointer$
  .option pop

  /* TODO: load ret_from_exception into $ra
   * so that we can return to ret_from_exception
   * when interrupt_help complete.
   */
  la ra,ret_from_exception
  /* TODO: call interrupt_helper
   * note: don't forget to pass parameters for it.
   */
  mv a0,sp
  ld sp,PCB_USER_SP(tp)
  csrr a1,CSR_STVEC
  csrr a2,CSR_SCAUSE
  call t0,interrupt_helper
  jr ra
ENDPROC(exception_handler_entry)

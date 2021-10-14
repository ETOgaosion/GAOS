#include <asm.h>
#include <csr.h>

ENTRY(invoke_syscall)
  /* TODO: */
  mv a7,a0
  mv a0,a1
  mv a1,a2
  mv a2,a3
  mv a3,a4
  mv a4,ra
  beqz a3,u_mode_entry
k_mode_entry:
  li a5,SR_SPP
  csrs CSR_SSTATUS,a5
  li a5,8
  csrw CSR_SCAUSE,a5
  la a5,syscall_return
  csrw CSR_SEPC,a5
  la a6,exception_handler_entry
  jr a6
u_mode_entry:
  ecall
syscall_return:
  mv ra,a4
  ret
ENDPROC(invoke_syscall)

ENTRY(sys_fork)
  call invoke_syscall
  ret
ENDPROC(sys_fork)
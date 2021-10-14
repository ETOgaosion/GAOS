#include <asm.h>
#include <csr.h>

ENTRY(invoke_syscall)
  /* TODO: */
  mv a7,a0
  mv a0,a1
  mv a1,a2
  mv a2,a3
  ecall
syscall_return:
  ret
ENDPROC(invoke_syscall)

ENTRY(sys_fork)
  call invoke_syscall
  ret
ENDPROC(sys_fork)
#include <asm.h>
#include <csr.h>
#include <sys/syscall_number.h>

ENTRY(invoke_syscall)
  /* TODO: */
  mv a7,a0
  mv a0,a1
  mv a1,a2
  mv a2,a3
  mv a3,a4
  ecall
syscall_return:
  ret
ENDPROC(invoke_syscall)

ENTRY(sys_fork)
  li a7,SYSCALL_FORK
  li a3,1
  ecall
  ret
ENDPROC(sys_fork)
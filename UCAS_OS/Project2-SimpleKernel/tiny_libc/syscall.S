#include <asm.h>
#include <csr.h>

ENTRY(invoke_syscall)
  /* TODO: */
  mv a7,a0
  mv a0,a1
  mv a1,a2
  mv a2,a3
  // ecall
  li a3,8
  csrw CSR_SCAUSE,a3
  auipc a3,0
  addi a3,a3,18
  csrw CSR_SEPC,a3
  la a4,exception_handler_entry
  jr a4
  ret
ENDPROC(invoke_syscall)

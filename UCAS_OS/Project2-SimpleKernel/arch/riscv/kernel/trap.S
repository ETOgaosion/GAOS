#include <asm.h>
#include <csr.h>

ENTRY(setup_exception)
  /* TODO: 
   * save exception_handler_entry into STVEC
   * enable global exceptions */
  la t0,exception_handler_entry
  csrw CSR_STVEC,t0
  li t0,SR_SIE
  csrs CSR_SSTATUS,t0
  li t0,SIE_SSIE | SIE_STIE | SIE_SEIE
  csrs CSR_SIE,t0
  jr ra
ENDPROC(setup_exception)

#include <asm.h>
#include <csr.h>

ENTRY(setup_exception)
  /* TODO: 
   * save exception_handler_entry into STVEC
   * enable global exceptions */
  la t0,exception_handler_entry
  csrw CSR_STVEC,t0
  bnez a0,set_eie
  li t0, SIE_STIE
  j finish_set_ie
set_eie:
  li t0, SIE_STIE | SIE_SEIE
finish_set_ie:
  csrw CSR_SIE, t0
  jr ra
ENDPROC(setup_exception)

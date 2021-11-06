#include <asm.h>
#include <csr.h>

ENTRY(setup_exception)
  /* TODO: 
   * save exception_handler_entry into STVEC
   * enable global exceptions */
  la t0,exception_handler_entry
  csrw CSR_STVEC,t0
  csrr t0, CSR_MHARTID
  beqz t0, setsie
  li t0, SIE_SSIE | SIE_STIE | SIE_SEIE
  j setup_exception_ret
setsie:
  li t0, SIE_STIE | SIE_SEIE
  csrw sie, t0
setup_exception_ret:
  jr ra
ENDPROC(setup_exception)

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
  li t0, SIE_SSIE | SIE_STIE
  j setup_exception_ret
setsie:
  li t0, SIE_STIE
setup_exception_ret:
  csrw CSR_SIE, t0
  jr ra
ENDPROC(setup_exception)

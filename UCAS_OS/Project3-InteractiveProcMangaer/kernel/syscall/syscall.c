#include <os/syscall.h>
#include <tasks.h>

long (*syscall[NUM_SYSCALLS])();

void handle_syscall(regs_context_t *regs, uint64_t interrupt, uint64_t cause)
{
    // syscall[fn](arg1, arg2, arg3)
    #if defined (PROJECT_3)
    regs->sepc = regs->sepc +4;
    #endif
    // use a7 to mark which syscall, a0~a2 are args
    regs->regs[10] = syscall[regs->regs[17]](regs->regs[10],
                                              regs->regs[11],
                                              regs->regs[12]);
}
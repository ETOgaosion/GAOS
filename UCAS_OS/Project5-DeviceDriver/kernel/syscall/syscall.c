#include <sys/syscall.h>
#include <sys/syscall_number.h>
#include <os/sched.h>
#include <tasks.h>

long (*syscall[NUM_SYSCALLS])();

void handle_syscall(regs_context_t *regs, uint64_t interrupt, uint64_t cause)
{
    // syscall[fn](arg1, arg2, arg3)
    regs->sepc = regs->sepc +4;
    // use a7 to mark which syscall, a0~a3 are args
    regs->regs[10] = syscall[regs->regs[17]](regs->regs[10],
                                             regs->regs[11],
                                             regs->regs[12],
                                             regs->regs[13],
                                             regs->regs[14]);
}

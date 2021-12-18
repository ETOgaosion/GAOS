#ifndef SMP_H
#define SMP_H

#include <os/lock.h>

extern spin_lock_t kernel_lock;

// #define NR_CPUS 2
// extern void* cpu_stack_pointer[NR_CPUS];
// extern void* cpu_pcb_pointer[NR_CPUS];
extern void smp_init();
extern void wakeup_other_hart();
extern uint64_t get_current_cpu_id();
extern void lock_kernel();
extern void unlock_kernel();

#endif /* SMP_H */

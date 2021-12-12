#ifndef TASKS_H
#define TASKS_H

#define PROJECT_5

#define USE_CLOCK_INT
#define TIE
#define EIE
#define LISTEN_PORT
//#define DEBUG_WITHOUT_NET
// #define QEMU
// #define INIT_WITH_PRIORITY
// #define SCHED_WITH_PRIORITY
// #define PRINT_PRIORITY

static inline void assert_supervisor_mode() 
{ 
   __asm__ __volatile__("csrr x0, sscratch\n"); 
} 


#endif

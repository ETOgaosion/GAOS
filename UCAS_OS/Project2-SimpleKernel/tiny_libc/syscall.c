#include <sys/syscall.h>
#include <stdint.h>
#include <os/syscall_number.h>
#include <os/sched.h>
#include <os/irq.h>
#include <tasks.h>

#define DI disable_preempt();
#define EI enable_preempt();

#if defined (TASK_1) || defined (TASK_2) || defined(TEST_SCHEDULE_1) || defined (TEST_LOCK)
#define MODE 1
#endif
#if defined (TASK_3) || defined (TASK_4) || defined (TASK_5) || defined(TEST_SCHEDULE_2) || defined(TEST_LOCK_2) || defined(TEST_TIMER)
#define MODE 0
#endif

void sys_yield()
{
    // TODO:
    invoke_syscall(SYSCALL_YIELD, IGNORE, IGNORE, IGNORE,MODE);
    //   or
    // do_scheduler();
    // ???
}

/*
// see syscall.S
// use assembly function
void sys_fork()
{
    invoke_syscall(SYSCALL_FORK,IGNORE,IGNORE,IGNORE,MODE);
}
*/

long sys_prior(long priority)
{
    return invoke_syscall(SYSCALL_PRIOR,priority,IGNORE,IGNORE,MODE);
}

long sys_getlock(int *key){
    return invoke_syscall(SYSCALL_GETLOCK,key,IGNORE,IGNORE,MODE);
}

long sys_lockop(long key, int op){
    long ret = invoke_syscall(SYSCALL_LOCKOP,key,op,IGNORE,MODE);
    if(ret == -2){
        sys_yield();
    }
    return ret;
}

void sys_write(char *buff)
{
    // TODO:
    invoke_syscall(SYSCALL_WRITE,buff,IGNORE,IGNORE,MODE);
}

char *sys_read(){
    return invoke_syscall(SYSCALL_READ,IGNORE,IGNORE,IGNORE,MODE);
}

void sys_reflush()
{
    // TODO:
    invoke_syscall(SYSCALL_REFLUSH,IGNORE,IGNORE,IGNORE,MODE);
}

void sys_move_cursor(int x, int y)
{
    // TODO:
    invoke_syscall(SYSCALL_CURSOR,x,y,IGNORE,MODE);
    //vt100_move_cursor(x,y);
}

long sys_get_timebase()
{
    // TODO:
    return invoke_syscall(SYSCALL_GET_TIMEBASE,IGNORE,IGNORE,IGNORE,MODE);
}

long sys_get_tick()
{
    // TODO:
    return invoke_syscall(SYSCALL_GET_TICK,IGNORE,IGNORE,IGNORE,MODE);
}

void sys_sleep(uint32_t time)
{
    // TODO:
    invoke_syscall(SYSCALL_SLEEP,time,IGNORE,IGNORE,MODE);
    sys_yield();
}
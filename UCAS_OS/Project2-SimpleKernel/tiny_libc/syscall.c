#include <sys/syscall.h>
#include <stdint.h>
#include <os/syscall_number.h>
#include <os/syscall.h>
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
    if(MODE){
        syscall[SYSCALL_YIELD]();
        return;
    }
    // TODO:
    invoke_syscall(SYSCALL_YIELD, IGNORE, IGNORE, IGNORE);
    //   or
    // do_scheduler();
    // ???
}

/*
// see syscall.S
// use assembly function
long sys_fork()
{
    return invoke_syscall(SYSCALL_FORK,IGNORE,IGNORE,IGNORE);
}
*/

void sys_setpriority(long priority)
{
    if(MODE){
        return syscall[SYSCALL_SET_PRIORITY](priority);
    }
    invoke_syscall(SYSCALL_SET_PRIORITY,priority,IGNORE,IGNORE);
}

long sys_getlock(int *key)
{
    if(MODE){
        return syscall[SYSCALL_GETLOCK](key);
    }
    return invoke_syscall(SYSCALL_GETLOCK,key,IGNORE,IGNORE);
}

long sys_lockop(long key, int op)
{
    if(MODE){
        return syscall[SYSCALL_LOCKOP](key,op);
    }
    return invoke_syscall(SYSCALL_LOCKOP,key,op,IGNORE);
}

void sys_write(char *buff)
{
    // TODO:
    if(MODE){
        return syscall[SYSCALL_WRITE](buff);
    }
    invoke_syscall(SYSCALL_WRITE,buff,IGNORE,IGNORE);
}

char sys_read_ch()
{
    if(MODE){
        return syscall[SYSCALL_READ_CH]();
    }
    return invoke_syscall(SYSCALL_READ_CH,IGNORE,IGNORE,IGNORE);
}

void sys_reflush()
{
    // TODO:
    if(MODE){
        return syscall[SYSCALL_REFLUSH]();
    }
    invoke_syscall(SYSCALL_REFLUSH,IGNORE,IGNORE,IGNORE);
}

void sys_move_cursor(int x, int y)
{
    // TODO:
    if(MODE){
        return syscall[SYSCALL_CURSOR](x,y);
    }
    invoke_syscall(SYSCALL_CURSOR,x,y,IGNORE);
    //vt100_move_cursor(x,y);
}

long sys_get_timebase()
{
    // TODO:
    if(MODE){
        return syscall[SYSCALL_GET_TIMEBASE]();
    }
    return invoke_syscall(SYSCALL_GET_TIMEBASE,IGNORE,IGNORE,IGNORE);
}

long sys_get_tick()
{
    // TODO:
    if(MODE){
        return syscall[SYSCALL_GET_TICK]();
    }
    return invoke_syscall(SYSCALL_GET_TICK,IGNORE,IGNORE,IGNORE);
}

void sys_sleep(uint32_t time)
{
    // TODO:
    if(MODE){
        return syscall[SYSCALL_SLEEP](time);
    }
    invoke_syscall(SYSCALL_SLEEP,time,IGNORE,IGNORE);
}
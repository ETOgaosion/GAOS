#include <sys/syscall.h>
#include <stdint.h>
#include <os/syscall_number.h>
#include <os/sched.h>

void sys_sleep(uint32_t time)
{
    // TODO:
    invoke_syscall(SYSCALL_SLEEP,time,IGNORE,IGNORE);
}

void sys_yield()
{
    // TODO:
    invoke_syscall(SYSCALL_YIELD, IGNORE, IGNORE, IGNORE);
    //   or
    // do_scheduler();
    // ???
}

long sys_getlock(){
    return invoke_syscall(SYSCALL_GETLOCK,IGNORE,IGNORE,IGNORE);
}

long sys_lockop(long key, int op){
    return invoke_syscall(SYSCALL_LOCKOP,key,op,IGNORE);
}

void sys_write(char *buff)
{
    // TODO:
    invoke_syscall(SYSCALL_WRITE,buff,IGNORE,IGNORE);
}

char *sys_read(){
    return invoke_syscall(SYSCALL_READ,IGNORE,IGNORE,IGNORE);
}

void sys_reflush()
{
    // TODO:
    invoke_syscall(SYSCALL_REFLUSH,IGNORE,IGNORE,IGNORE);
}

void sys_move_cursor(int x, int y)
{
    // TODO:
    invoke_syscall(SYSCALL_CURSOR,x,y,IGNORE);
    //vt100_move_cursor(x,y);
}

long sys_get_timebase()
{
    // TODO:
    return invoke_syscall(SYSCALL_GET_TIMEBASE,IGNORE,IGNORE,IGNORE);
}

long sys_get_tick()
{
    // TODO:
    return invoke_syscall(SYSCALL_GET_TICK,IGNORE,IGNORE,IGNORE);
}


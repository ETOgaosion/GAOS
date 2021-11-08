#include <sys/syscall.h>
#include <stdint.h>
#include <os/syscall_number.h>
#include <os/syscall.h>
#include <os/sched.h>
#include <os/irq.h>
#include <tasks.h>

#define DI disable_preempt();
#define EI enable_preempt();

#if !defined (PROJECT_3)
#define MODE 1
#endif
#if defined (PROJECT_3)
#define MODE 0
#endif

pid_t sys_spawn(task_info_t *info, void* arg, spawn_mode_t mode){
    if(MODE){
        return syscall[SYSCALL_SPAWN](info,arg,mode);
    }
    return invoke_syscall(SYSCALL_SPAWN,info,arg,mode);
}

void sys_exit(void){
    if(MODE){
        syscall[SYSCALL_EXIT]();
        return;
    }
    invoke_syscall(SYSCALL_EXIT,IGNORE,IGNORE,IGNORE);
}

int sys_kill(pid_t pid){
    if(MODE){
        return syscall[SYSCALL_KILL](pid);
    }
    return invoke_syscall(SYSCALL_KILL,pid,IGNORE,IGNORE);
}

int sys_waitpid(pid_t pid){
    if(MODE){
        return syscall[SYSCALL_WAITPID](pid);
    }
    return invoke_syscall(SYSCALL_WAITPID,pid,IGNORE,IGNORE);
}

void sys_process_show(void){
    if(MODE){
        syscall[SYSCALL_PS]();
        return;
    }
    invoke_syscall(SYSCALL_PS,IGNORE,IGNORE,IGNORE);
}

pid_t sys_getpid(){
    if(MODE){
        return syscall[SYSCALL_GETPID]();
    }
    return invoke_syscall(SYSCALL_GETPID,IGNORE,IGNORE,IGNORE);
}

void sys_yield()
{
    if(MODE){
        syscall[SYSCALL_YIELD]();
        return;
    }
    // TODO:
    invoke_syscall(SYSCALL_YIELD, IGNORE, IGNORE, IGNORE);
    //   or
    // k_scheduler();
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
        syscall[SYSCALL_SET_PRIORITY](priority);
        return;
    }
    invoke_syscall(SYSCALL_SET_PRIORITY,priority,IGNORE,IGNORE);
}

int sys_taskset(void *arg){
    if(MODE){
        syscall[SYSCALL_TASKSET](arg);
        return;
    }
    invoke_syscall(SYSCALL_TASKSET,arg,IGNORE,IGNORE);
}

int sys_lockop(int *key, int op)
{
    if(MODE){
        return syscall[SYSCALL_LOCKOP](key,op);
    }
    return invoke_syscall(SYSCALL_LOCKOP,key,op,IGNORE);
}

int sys_commop(int *key, int *args, int op)
{
    if(MODE){
        return syscall[SYSCALL_COMMOP](key,args,op);
    }
    return invoke_syscall(SYSCALL_COMMOP,key,args,op);
}

void sys_write(char *buff)
{
    // TODO:
    if(MODE){
        syscall[SYSCALL_WRITE](buff);
        return;
    }
    invoke_syscall(SYSCALL_WRITE,buff,IGNORE,IGNORE);
}

void sys_move_cursor(int x, int y)
{
    // TODO:
    if(MODE){
        syscall[SYSCALL_MOVE_CURSOR](x,y);
        return;
    }
    invoke_syscall(SYSCALL_MOVE_CURSOR,x,y,IGNORE);
    //vt100_move_cursor(x,y);
}

void sys_reflush()
{
    // TODO:
    if(MODE){
        syscall[SYSCALL_REFLUSH]();
        return;
    }
    invoke_syscall(SYSCALL_REFLUSH,IGNORE,IGNORE,IGNORE);
}

char sys_serial_read()
{
    char ch;
    while(1){
        if(MODE){
            ch = syscall[SYSCALL_SERIAL_READ]();
        }
        else{
            ch = invoke_syscall(SYSCALL_SERIAL_READ,IGNORE,IGNORE,IGNORE);
        }
        if(ch > 0 && ch < 128){
            return ch;
        }
    }
}

char sys_read_ch()
{
    if(MODE){
        return syscall[SYSCALL_SERIAL_READ]();
    }
    return invoke_syscall(SYSCALL_SERIAL_READ,IGNORE,IGNORE,IGNORE);
}

void sys_serial_write(char ch)
{
    if(MODE){
        syscall[SYSCALL_SERIAL_WRITE](ch);
        return;
    }
    invoke_syscall(SYSCALL_SERIAL_WRITE,ch,IGNORE,IGNORE);
}

void sys_screen_clear(void){
    if(MODE){
        syscall[SYSCALL_SCREEN_CLEAR]();
        return;
    }
    invoke_syscall(SYSCALL_SCREEN_CLEAR,IGNORE,IGNORE,IGNORE);
}

void sys_get_cursor(int *x, int *y){
    if(MODE){
        syscall[SYSCALL_GET_CURSOR](x,y);
        return;
    }
    invoke_syscall(SYSCALL_SLEEP,x,y,IGNORE);
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

long sys_get_wall_time(long *time_elapsed)
{
    if(MODE){
        return syscall[SYSCALL_GET_WALL_TIME]();
    }
    return invoke_syscall(SYSCALL_GET_WALL_TIME,time_elapsed,IGNORE,IGNORE);
}

void sys_sleep(long time)
{
    // TODO:
    if(MODE){
        syscall[SYSCALL_SLEEP](time);
        return;
    }
    invoke_syscall(SYSCALL_SLEEP,time,IGNORE,IGNORE);
}

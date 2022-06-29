#include <sys/syscall.h>
#include <sys/syscall_number.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdint.h>

pid_t sys_spawn(task_info_t *info, int argc, char* argv[], spawn_mode_t mode){
    return invoke_syscall(SYSCALL_SPAWN,info,argc,argv,mode,IGNORE);
}

void sys_exit(void){
    invoke_syscall(SYSCALL_EXIT,IGNORE,IGNORE,IGNORE,IGNORE,IGNORE);
}

int sys_kill(pid_t pid){
    return invoke_syscall(SYSCALL_KILL,pid,IGNORE,IGNORE,IGNORE,IGNORE);
}

int sys_waitpid(pid_t pid){
    return invoke_syscall(SYSCALL_WAITPID,pid,IGNORE,IGNORE,IGNORE,IGNORE);
}

int sys_process_show(void){
    return invoke_syscall(SYSCALL_PS,IGNORE,IGNORE,IGNORE,IGNORE,IGNORE);
}

pid_t sys_getpid(){
    return invoke_syscall(SYSCALL_GETPID,IGNORE,IGNORE,IGNORE,IGNORE,IGNORE);
}

void sys_yield()
{
    // TODO:
    invoke_syscall(SYSCALL_YIELD, IGNORE, IGNORE, IGNORE,IGNORE,IGNORE);
    //   or
    // k_schedule(,IGNORE,IGNORE);
    // ???
}

/*
// see syscall.S
// use assembly function
long sys_fork()
{
    return invoke_syscall(SYSCALL_FORK,IGNORE,IGNORE,IGNORE,IGNORE,IGNORE);
}
*/

void sys_setpriority(long priority)
{
    invoke_syscall(SYSCALL_SET_PRIORITY,priority,IGNORE,IGNORE,IGNORE,IGNORE);
}

int sys_taskset(void *arg){
    return invoke_syscall(SYSCALL_TASKSET,arg,IGNORE,IGNORE,IGNORE,IGNORE);
}


int sys_mthread_create(int *thread, void (*start_routine)(void*), void *arg){
    return invoke_syscall(SYSCALL_MTHREAD_CREATE,thread,start_routine,arg,IGNORE,IGNORE);
}

int sys_lockop(int *key, int op)
{
    return invoke_syscall(SYSCALL_LOCKOP,key,op,IGNORE,IGNORE,IGNORE);
}

int sys_commop(int *key, int *args, int op)
{
    return invoke_syscall(SYSCALL_COMMOP,key,args,op,IGNORE,IGNORE);
}

void* shmpageget(int key)
{
        return invoke_syscall(SYSCALL_SHMPGET, key, IGNORE, IGNORE, IGNORE,IGNORE);
}

void shmpagedt(void *addr)
{
        invoke_syscall(SYSCALL_SHMPDT, (uintptr_t)addr, IGNORE, IGNORE, IGNORE,IGNORE);
}

void sys_write(char *buff)
{
    // TODO:
    invoke_syscall(SYSCALL_WRITE_SCREEN,buff,IGNORE,IGNORE,IGNORE,IGNORE);
}

void sys_move_cursor(int x, int y)
{
    // TODO:
    invoke_syscall(SYSCALL_MOVE_CURSOR,x,y,IGNORE,IGNORE,IGNORE);
    //vt100_move_cursor(x,y,IGNORE,IGNORE);
}

void sys_reflush()
{
    // TODO:
    invoke_syscall(SYSCALL_REFLUSH,IGNORE,IGNORE,IGNORE,IGNORE,IGNORE);
}

char sys_serial_read()
{
    char ch;
    while(1){
        ch = invoke_syscall(SYSCALL_SERIAL_READ,IGNORE,IGNORE,IGNORE,IGNORE,IGNORE);
        if(ch > 0 && ch < 128){
            return ch;
        }
    }
}

char sys_read_ch()
{
    return invoke_syscall(SYSCALL_SERIAL_READ,IGNORE,IGNORE,IGNORE,IGNORE,IGNORE);
}

void sys_serial_write(char ch)
{
    invoke_syscall(SYSCALL_SERIAL_WRITE,ch,IGNORE,IGNORE,IGNORE,IGNORE);
}

void sys_screen_clear(void){
    invoke_syscall(SYSCALL_SCREEN_CLEAR,IGNORE,IGNORE,IGNORE,IGNORE,IGNORE);
}

void sys_get_cursor(int *x, int *y){
    invoke_syscall(SYSCALL_SLEEP,x,y,IGNORE,IGNORE,IGNORE);
}

long sys_get_timebase()
{
    // TODO:
    return invoke_syscall(SYSCALL_GET_TIMEBASE,IGNORE,IGNORE,IGNORE,IGNORE,IGNORE);
}

long sys_get_tick()
{
    // TODO:
    return invoke_syscall(SYSCALL_GET_TICK,IGNORE,IGNORE,IGNORE,IGNORE,IGNORE);
}

long sys_get_wall_time(long *time_elapsed)
{
    return invoke_syscall(SYSCALL_GET_WALL_TIME,time_elapsed,IGNORE,IGNORE,IGNORE,IGNORE);
}

void sys_sleep(long time)
{
    // TODO:
    invoke_syscall(SYSCALL_SLEEP,time,IGNORE,IGNORE,IGNORE,IGNORE);
}

int sys_fsop(int op,int option)
{
    return invoke_syscall(SYSCALL_FSOP,op,option,IGNORE,IGNORE,IGNORE);
}

int sys_dirop(int op, char *dirname, int option)
{
    return invoke_syscall(SYSCALL_DIROP,op,dirname,option,IGNORE,IGNORE);
}

int sys_fileop(int op, char *mul_char, int mul_int, int size){
    return invoke_syscall(SYSCALL_FILEOP,op,mul_char,mul_int,size,IGNORE);
}

int sys_linkop(int op, char *src, char *dst)
{
    return invoke_syscall(SYSCALL_LINKOP,op,src,dst,IGNORE,IGNORE);
}

int sys_lseek(int fd, int offset, int whence, int r_or_w)
{
    return invoke_syscall(SYSCALL_LSEEK, fd, offset, whence, r_or_w, IGNORE);
}

long sys_net_recv(uintptr_t addr, size_t length, int num_packet, size_t* frLength, int port){
    return invoke_syscall(SYSCALL_NET_RECV,addr,length,num_packet,frLength,port);
}

void sys_net_send(uintptr_t addr, size_t length){
    invoke_syscall(SYSCALL_NET_SEND,addr,length,IGNORE,IGNORE,IGNORE);
}

void sys_net_irq_mode(int mode){
    invoke_syscall(SYSCALL_NET_IRQ_MODE,mode,IGNORE,IGNORE,IGNORE,IGNORE);
}
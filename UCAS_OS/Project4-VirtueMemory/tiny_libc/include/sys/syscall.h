/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *            Copyright (C) 2018 Institute of Computing Technology, CAS
 *               Author : Han Shukai (email : hanshukai@ict.ac.cn)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *                       System call related processing
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnisched to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * */

#ifndef INCLUDE_SYSCALL_H_
#define INCLUDE_SYSCALL_H_

#include <os.h>

// the last parameter shows with/without return value
extern long invoke_syscall(long, long, long, long, long);

extern pid_t sys_spawn(task_info_t *info, int argc, char* argv[], spawn_mode_t mode);
extern void sys_exit(void);
extern void sys_sleep(long);
extern int sys_kill(pid_t pid);
extern int sys_waitpid(pid_t pid);
extern int sys_process_show(void);
extern pid_t sys_getpid();
extern void sys_yield();
extern long sys_fork();
extern void sys_setpriority(long);
extern int sys_taskset(void *);
extern int sys_lockop(int *key, int op);
extern int sys_commop(int *key, int *args, int op);
extern void sys_write(char *);
extern char sys_serial_read();
extern char sys_read_ch();
extern void sys_serial_write(char ch);
extern void sys_move_cursor(int, int);
extern void sys_reflush();
extern void sys_screen_clear(void);
extern void sys_get_cursor(int *x, int *y);
extern long sys_get_timebase();
extern long sys_get_tick();
extern long sys_get_wall_time(long *);



#endif

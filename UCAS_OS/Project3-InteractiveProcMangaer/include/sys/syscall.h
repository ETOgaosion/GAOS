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

#include <stdint.h>
#include <os.h>

// the last parameter shows with/without return value
extern long invoke_syscall(long, long, long, long);

pid_t sys_spawn(task_info_t *info, void* arg, spawn_mode_t mode);
void sys_exit(void);
void sys_sleep(long);
int sys_kill(pid_t pid);
int sys_waitpid(pid_t pid);
void sys_process_show(void);
pid_t sys_getpid();
void sys_yield();
extern long sys_fork();
void sys_setpriority(long);
int sys_taskset(void *);
int sys_lockop(int *key, int op);
int sys_commop(int *key, int *args, int op);
void sys_write(char *);
char sys_serial_read();
char sys_read_ch();
void sys_serial_write(char ch);
void sys_move_cursor(int, int);
void sys_reflush();
void sys_screen_clear(void);
void sys_get_cursor(int *x, int *y);
long sys_get_timebase();
long sys_get_tick();
long sys_get_wall_time(long *);



#endif

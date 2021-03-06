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
 * furnished to do so, subject to the following conditions:
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

#ifndef OS_SYSCALL_NUMBER_H_
#define OS_SYSCALL_NUMBER_H_

#define IGNORE 0
#define NUM_SYSCALLS 64

/* define */
#define SYSCALL_SPAWN 0
#define SYSCALL_EXIT 1
#define SYSCALL_SLEEP 2
#define SYSCALL_KILL 3
#define SYSCALL_WAITPID 4
#define SYSCALL_PS 5
#define SYSCALL_GETPID 6
#define SYSCALL_YIELD 7
#define SYSCALL_FORK 8
#define SYSCALL_SET_PRIORITY 9
#define SYSCALL_TASKSET 10
#define SYSCALL_MTHREAD_CREATE 11

#define SYSCALL_LOCKOP 13
// proc comm, [0,3] is for Semaphore, [4,6] is for barrier, [7,10] is for mailbox
#define SYSCALL_COMMOP 14

#define SYSCALL_SHMPGET 15
#define SYSCALL_SHMPDT 16

#define SYSCALL_WRITE_SCREEN 20

#define SYSCALL_MOVE_CURSOR 23
#define SYSCALL_REFLUSH 24
#define SYSCALL_SERIAL_READ 25
#define SYSCALL_SERIAL_WRITE 26
#define SYSCALL_READ_SHELL_BUFF 27
#define SYSCALL_SCREEN_CLEAR 28

#define SYSCALL_GET_CURSOR 29

#define SYSCALL_GET_TIMEBASE 30
#define SYSCALL_GET_TICK 31
#define SYSCALL_GET_WALL_TIME 32

#define SYSCALL_FSOP 33
#define SYSCALL_DIROP 34
#define SYSCALL_FILEOP 35
#define SYSCALL_LINKOP 36
#define SYSCALL_LSEEK 37

#define SYSCALL_NET_RECV 43
#define SYSCALL_NET_SEND 44
#define SYSCALL_NET_IRQ_MODE 45

#endif

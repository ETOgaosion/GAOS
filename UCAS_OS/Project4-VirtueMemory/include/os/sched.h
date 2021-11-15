/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *            Copyright (C) 2018 Institute of Computing Technology, CAS
 *               Author : Han Shukai (email : hanshukai@ict.ac.cn)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *        Process scheduling related content, such as: scheduler, process blocking,
 *                 process wakeup, process creation, process kill, etc.
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

#ifndef INCLUDE_SCHEDULER_H_
#define INCLUDE_SCHEDULER_H_
 
#include <context.h>

#include <type.h>
#include <os/list.h>
#include <os/mm.h>
#include <os/time.h>
#include <asm/regs.h>

#define NUM_MAX_TASK 16
#define MAX_LOCK_PER_PCB 20

#define P_OVERLAY 0
#define P_WAIT 1
#define P_NO_WAIT 2

/* used to save register infomation */
typedef struct regs_context
{
    /* Saved main processor registers.*/
    reg_t regs[32];

    /* Saved special registers. */
    reg_t sstatus;
    reg_t sepc;
    reg_t sbadaddr;
    reg_t scause;
    reg_t sie;
} regs_context_t;

/* used to save register infomation in switch_to */
typedef struct switchto_context
{
    /* Callee saved registers.*/
    reg_t regs[14];
} switchto_context_t;
typedef struct prior
{
    long priority;
    uint64_t last_sched_time;
} prior_t;

/*MODIFIED*/
typedef enum {
    TASK_BLOCKED,
    TASK_RUNNING,
    TASK_READY,
    TASK_ZOMBIE,
    TASK_EXITED,
} task_status_t;

typedef enum {
    ENTER_ZOMBIE_ON_EXIT,
    AUTO_CLEANUP_ON_EXIT,
} spawn_mode_t;

typedef enum {
    KERNEL_PROCESS,
    KERNEL_THREAD,
    USER_PROCESS,
    USER_THREAD,
} task_type_t;

/* Process Control Block */
typedef struct pcb
{
    /* register context */
    // this must be this order!! The order is defined in regs.h
    reg_t kernel_sp;
    reg_t user_sp;

    ptr_t kernel_stack_base;
    ptr_t user_stack_base;

    /* process id */
    pid_t pid;

    /* kernel/user thread/process */
    task_type_t type;

    /* BLOCK | READY | RUNNING | ZOMBIE */
    task_status_t status;
    spawn_mode_t mode;

    /* cursor position */
    int cursor_x;
    int cursor_y;

    /* priority */
    prior_t sched_prior;
    
    // count the number of disable_preempt
    // enable_preempt enables CSR_SIE only when preempt_count == 0
    reg_t preempt_count;

    /* previous, next pointer */
    list_node_t list;
    
    struct pcb *wait_parent;

    /* timer */
    list_node_t timer_list;
    timer_t timer;

    // locks and mbox owned
    long lock_keys[MAX_LOCK_PER_PCB];
    int owned_lock_num;

    int mbox_keys[MAX_LOCK_PER_PCB];
    int owned_mbox_num;

    // use mask to point to schedule core
    int core_mask;
} pcb_t;

/* task information, used to init PCB */
typedef struct task_info
{
    ptr_t entry_point;
    task_type_t type;
} task_info_t;

typedef struct taskset_arg{
    int pid;
    int mask;
} taskset_arg_t;

extern void ret_from_exception();
extern void __global_pointer$();

/* ready queue to run */
extern list_head ready_queue;
extern list_head blocked_queue;

extern task_info_t **tasks;
extern long tasks_num;

/* current running task PCB */
extern pcb_t * volatile current_running_core_m;
extern pcb_t * volatile current_running_core_s;
extern pcb_t ** volatile current_running;
// extern pcb_t * volatile (*current_running)[NR_CPUS];
extern pid_t process_id;

extern pcb_t pcb[NUM_MAX_TASK];
// extern pcb_t kernel_pcb[NR_CPUS];
extern pcb_t pid0_pcb_core_m;
extern const ptr_t pid0_stack_core_m;
extern pcb_t pid0_pcb_core_s;
extern const ptr_t pid0_stack_core_s;

extern pcb_t bubble_pcb;
extern const ptr_t bubble_stack;

extern void init_pcb_stack(
    ptr_t kernel_stack, ptr_t user_stack, ptr_t entry_point,
    pcb_t *pcb, void *arg);

extern void init_pcb_block(pcb_t *pcb);

int find_pcb(void);

extern pid_t k_exec(const char* file_name, int argc, char* argv[], spawn_mode_t mode);
extern void k_show_exec();
pid_t k_spawn(task_info_t *task, void* arg, spawn_mode_t mode);
void k_exit(void);
int k_kill(pid_t pid);
int k_waitpid(pid_t pid);
void k_process_show();
pid_t k_getpid();

int k_taskset(void *arg);

extern void switch_to(pcb_t *prev, pcb_t *next);
void k_scheduler();
void k_sleep(uint32_t);

void k_block(list_node_t *, list_head *queue);
void k_unblock(list_head *queue, int way);

long k_fork(void);
void copy_pcb_stack(ptr_t kid_kernel_stack, ptr_t kid_user_stack,pcb_t *kid, ptr_t src_kernel_stack, ptr_t src_user_stack, pcb_t *src);

void set_priority(long priority);
uint64_t cal_priority(uint64_t cur_time, uint64_t idle_time, long priority);

pcb_t *choose_sched_task(list_head *queue);

pcb_t *dequeue(list_head *queue, int field);
extern pid_t do_exec(const char* file_name, int argc, char* argv[], spawn_mode_t mode);
extern void do_show_exec();
 
#endif

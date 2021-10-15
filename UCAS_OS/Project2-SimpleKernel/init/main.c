/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *            Copyright (C) 2018 Institute of Computing Technology, CAS
 *               Author : Han Shukai (email : hanshukai@ict.ac.cn)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *         The kernel's entry, where most of the initialization work is done.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so, subject to the following conditions:
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

#include <common.h>
#include <os/irq.h>
#include <os/mm.h>
#include <os/sched.h>
#include <screen.h>
#include <sbi.h>
#include <stdio.h>
#include <os/time.h>
#include <os/syscall.h>
#include <test.h>
#include <os/lock.h>
#include <csr.h>
#include <tasks.h>
#include <ticks.h>


extern void ret_from_exception();
extern void __global_pointer$();
extern void kp_ret_from_exception();
extern task_info_t **tasks;
extern long tasks_num;

static void init_pcb_stack(
    ptr_t kernel_stack, ptr_t user_stack, ptr_t entry_point,
    pcb_t *pcb)
{
    regs_context_t *pt_regs =
        (regs_context_t *)(kernel_stack - sizeof(regs_context_t));
    
    /* TODO: initialization registers
     * note: sp, gp, ra, sepc, sstatus
     * gp should be __global_pointer$
     * To run the task in user mode,
     * you should set corresponding bits of sstatus(SPP, SPIE, etc.).
     */
    for(int i=0;i<32;i++){
        pt_regs->regs[i]=0;
    }
    pt_regs->regs[1] = entry_point;
    pt_regs->regs[3] = (reg_t)__global_pointer$;
    pt_regs->regs[4] = (reg_t)pcb;
    pt_regs->sepc = entry_point;
    pt_regs->scause = 0;
    pt_regs->sbadaddr = 0;
    pt_regs->sie = 0;
    if(pcb->type == USER_PROCESS || pcb->type == USER_THREAD){
        pt_regs->regs[2] = user_stack;
        pt_regs->sstatus = 0;
    }
    else{
        pt_regs->regs[2] = kernel_stack - sizeof(regs_context_t) - sizeof(switchto_context_t);
        pt_regs->sstatus = pt_regs->sstatus | SR_SPP | SR_SPIE;
    }

    // set sp to simulate return from switch_to
    /* TODO: you should prepare a stack, and push some values to
     * simulate a pcb context.
     */
    pcb->kernel_sp = kernel_stack- sizeof(regs_context_t) - sizeof(switchto_context_t);
    switchto_context_t *stored_switchto_k = (switchto_context_t *) pcb->kernel_sp;
    if(pcb->type == USER_PROCESS || pcb->type == USER_THREAD){
        stored_switchto_k->regs[0] = (reg_t)&ret_from_exception;
    }
    else{
        stored_switchto_k->regs[0] = entry_point;
    }
    stored_switchto_k->regs[1] = pcb->kernel_sp;
    for(int i=2;i<4;i++){
        stored_switchto_k->regs[i] = pt_regs->regs[i+7];
    }
    for(int i=4;i<14;i++){
        stored_switchto_k->regs[i] = pt_regs->regs[i+15];
    }
}

static void init_pcb()
{
     /* initialize all of your pcb and add them into ready_queue
     * TODO:
     */

    /* remember to initialize `current_running`
     * TODO:
     */
    init_list_head(&ready_queue);
    #ifdef TEST_SCHEDULE_1
        tasks = sched1_tasks;
        tasks_num = num_sched1_tasks;
    #endif
    #ifdef TASK_1
        tasks = sched1_tasks;
        tasks_num = num_sched1_tasks;
    #endif
    #ifdef TEST_LOCK
        tasks = lock_tasks;
        tasks_num = num_lock_tasks;
    #endif
    #ifdef TASK_2
        tasks_num = num_sched1_tasks + num_lock_tasks;
        tasks = (task_info_t **)kmalloc(sizeof(task_info_t *) * tasks_num);
        for (int i = 0; i < num_sched1_tasks; i++)
        {
            tasks[i] = sched1_tasks[i];
        }
        for (int i = 0; i < num_lock_tasks; i++)
        {
            tasks[i+num_sched1_tasks] = lock_tasks[i];
        }
    #endif
    #ifdef TEST_TIMER
        tasks = timer_tasks;
        tasks_num = num_timer_tasks;
    #endif
    #ifdef TEST_SCHEDULE_2
        tasks = sched2_tasks;
        tasks_num = num_sched2_tasks;
    #endif
    #ifdef TASK_3
        tasks_num = num_timer_tasks + num_sched2_tasks;
        tasks = (task_info_t **)kmalloc(sizeof(task_info_t *) * tasks_num);
        for (int i = 0; i < num_timer_tasks; i++)
        {
            tasks[i] = timer_tasks[i];
        }
        for (int i = 0; i < num_sched2_tasks; i++)
        {
            tasks[i+num_timer_tasks] = sched2_tasks[i];
        }
    #endif
    #ifdef TEST_LOCK_2
        tasks = lock2_tasks;
        tasks_num = num_lock2_tasks;
    #endif
    #ifdef TASK_4
        tasks_num = num_sched2_tasks + num_lock2_tasks;
        tasks = (task_info_t **)kmalloc(sizeof(task_info_t *) * tasks_num);
        for (int i = 0; i < num_sched2_tasks; i++)
        {
            tasks[i] = sched2_tasks[i];
        }
        for (int i = 0; i < num_lock2_tasks; i++)
        {
            tasks[i+num_sched2_tasks] = lock2_tasks[i];
        }
    #endif
    #ifdef TASK_5
        tasks = fork_prior_tasks;
        tasks_num = num_fork_prior_tasks;
    #endif
    for(int i=0;i<tasks_num;i++){
        // use allocPage in mm.c, first time allocate 1 page only
        pcb[i].kernel_sp = allocPage(1);
        pcb[i].user_sp = allocPage(1);
        #if !defined (TASK_4) && !defined (TASK_5) && !defined (USE_CLOCK_INT) // no preempt
        pcb[i].preempt_count = 1;
        #endif
        #if defined (TASK_4) || defined (TASK_5) || defined (USE_CLOCK_INT) // enable preempt
        pcb[i].preempt_count = 0;
        #endif
        pcb[i].list.prev = NULL;
        pcb[i].list.next = NULL;
        pcb[i].pid = i+1;
        pcb[i].type = tasks[i]->type;
        pcb[i].status = TASK_READY;
        pcb[i].cursor_x = 0;
        pcb[i].cursor_y = 0;
        pcb[i].timer.initialized = 0;
        pcb[i].priority = 0;
        init_pcb_stack(pcb[i].kernel_sp,pcb[i].user_sp,tasks[i]->entry_point,&pcb[i]);
        list_add_tail(&(pcb[i].list),&ready_queue);
    }
    // help initialize pid0
    switchto_context_t *stored_switchto_k = (switchto_context_t *) pid0_pcb.kernel_sp;
    stored_switchto_k->regs[1] = pid0_pcb.kernel_sp;
    current_running = &pid0_pcb;
}

static void init_syscall(void)
{
    // initialize system call table.
    for(int i=0;i<NUM_SYSCALLS;i++){
        syscall[i] = (long (*)())&unknown_syscall; // only print register info
    }
    syscall[SYSCALL_YIELD]          = (long (*)())&do_scheduler;
    syscall[SYSCALL_FORK]           = (long (*)())&do_fork;
    syscall[SYSCALL_SET_PRIORITY]   = (long (*)())&set_priority;
    syscall[SYSCALL_GETLOCK]        = (long (*)())&do_mutex_lock_init;
    syscall[SYSCALL_LOCKOP]         = (long (*)())&do_mutex_lock_op;
    syscall[SYSCALL_WRITE]          = (long (*)())&screen_write;
    syscall[SYSCALL_READ_CH]        = (long (*)())&sbi_console_getchar;
    syscall[SYSCALL_CURSOR]         = (long (*)())&screen_move_cursor;
    syscall[SYSCALL_REFLUSH]        = (long (*)())&screen_reflush;
    syscall[SYSCALL_GET_TIMEBASE]   = (long (*)())&get_timer;
    syscall[SYSCALL_GET_TICK]       = (long (*)())&get_ticks;
    syscall[SYSCALL_SLEEP]          = (long (*)())&do_sleep;
}

// jump from bootloader.
// The beginning of everything >_< ~~~~~~~~~~~~~~
int main()
{
    // init Process Control Block (-_-!)
    init_pcb();
    printk("> [INIT] PCB initialization succeeded.\n\r");

    // init interrupt (^_^)
    init_exception();
    printk("> [INIT] Interrupt processing initialization succeeded.\n\r");

    // init system call table (0_0)
    init_syscall();
    printk("> [INIT] System call initialized successfully.\n\r");

    // fdt_print(riscv_dtb);

    // init screen (QAQ)
    init_screen();
    printk("> [INIT] SCREEN initialization succeeded.\n\r");

    // TODO:
    // Setup timer interrupt and enable all interrupt

    // read CPU frequency
    time_base = sbi_read_fdt(TIMEBASE);
    printk("time_base:%d\n",time_base);

    while (1) {
        // (QAQQQQQQQQQQQ)
        // If you do non-preemptive scheduling, you need to use it
        // to surrender control do_scheduler();
        #if defined TASK_4 || defined (TASK_5) || defined (USE_CLOCK_INT)
        reset_irq_timer();
        enable_interrupt();
        __asm__ __volatile__("wfi\n\r":::);
        #endif
        #if !defined (TASK_4) && !defined (TASK_5) && !defined (USE_CLOCK_INT)
        do_scheduler();
        #endif
    };
    return 0;
}


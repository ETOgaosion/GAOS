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
 * persons to whom the Software is furnisched to do so, subject to the following conditions:
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
#include <os/comm.h>
#include <csr.h>
#include <tasks.h>
#include <ticks.h>
#include <os/stdio.h>
#include <string.h>

extern void ret_from_exception();
extern void __global_pointer$();
extern void kp_ret_from_exception();
task_info_t **tasks;
long tasks_num;

void init_pcb_stack(
    ptr_t kernel_stack, ptr_t user_stack, ptr_t entry_point,
    pcb_t *pcb, void *arg)
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
    pt_regs->regs[10]= (reg_t)*(int *)arg;
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
    init_list_head(&ready_queue);
    kmemset(pcb,0,sizeof(pcb));
    #ifdef PROJECT_3
    #ifdef TASK_1
        tasks = shell_tasks;
        tasks_num = num_shell_tasks;
    #endif
    #endif
    int init_ticks = get_ticks();
    for(int i=0;i<tasks_num;i++){
        // use allocPage in mm.c, first time allocate 1 page only
        // user stack is below kernel stack for security
        pcb[i].user_sp = allocPage(1,2*i);
        pcb[i].user_stack_base = pcb[i].user_sp - PAGE_SIZE;
        pcb[i].kernel_sp = allocPage(1,2*i+1);
        pcb[i].kernel_stack_base = pcb[i].kernel_sp - PAGE_SIZE;
        #if !defined (USE_CLOCK_INT) // no preempt
        pcb[i].preempt_count = 1;
        #endif
        #if defined (USE_CLOCK_INT) // enable preempt
        pcb[i].preempt_count = 0;
        #endif
        init_list_head(&pcb[i].list);
        pcb[i].wait_parent = NULL;
        pcb[i].pid = i+1;
        pcb[i].owned_lock_num = 0;
        pcb[i].owned_mbox_num = 0;
        pcb[i].type = tasks[i]->type;
        pcb[i].status = TASK_READY;
        pcb[i].cursor_x = 0;
        pcb[i].cursor_y = 0;
        pcb[i].timer.initialized = 0;
        #if defined (INIT_WITH_PRIORITY)
        pcb[i].sched_prior.priority = i;
        #endif
        #ifndef INIT_WITH_PRIORITY
        pcb[i].sched_prior.priority = 0;
        #endif
        pcb[i].sched_prior.last_sched_time = init_ticks;
        init_pcb_stack(pcb[i].kernel_sp,pcb[i].user_sp,tasks[i]->entry_point,&pcb[i],NULL);
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
    syscall[SYSCALL_SPAWN]          = (long (*)())&k_spawn;
    syscall[SYSCALL_EXIT]           = (long (*)())&k_exit;
    syscall[SYSCALL_SLEEP]          = (long (*)())&k_sleep;
    syscall[SYSCALL_KILL]           = (long (*)())&k_kill;
    syscall[SYSCALL_WAITPID]        = (long (*)())&k_waitpid;
    syscall[SYSCALL_PS]             = (long (*)())&k_process_show;
    syscall[SYSCALL_GETPID]         = (long (*)())&k_getpid;
    syscall[SYSCALL_YIELD]          = (long (*)())&k_scheduler;
    syscall[SYSCALL_FORK]           = (long (*)())&k_fork;
    syscall[SYSCALL_SET_PRIORITY]   = (long (*)())&set_priority;
    syscall[SYSCALL_LOCKOP]         = (long (*)())&k_mutex_lock_op;
    syscall[SYSCALL_COMMOP]         = (long (*)())&k_commop;
    
    syscall[SYSCALL_WRITE]          = (long (*)())&screen_write;
    syscall[SYSCALL_MOVE_CURSOR]    = (long (*)())&screen_move_cursor;
    syscall[SYSCALL_REFLUSH]        = (long (*)())&screen_reflush;
    syscall[SYSCALL_SERIAL_READ]    = (long (*)())&sbi_console_getchar;
    syscall[SYSCALL_SERIAL_WRITE]   = (long (*)())&screen_putchar;
    syscall[SYSCALL_SCREEN_CLEAR]   = (long (*)())&screen_clear;
    syscall[SYSCALL_GET_CURSOR]     = (long (*)())&get_cursor;
    syscall[SYSCALL_GET_TIMEBASE]   = (long (*)())&get_timer;
    syscall[SYSCALL_GET_TICK]       = (long (*)())&get_ticks;
    syscall[SYSCALL_GET_WALL_TIME]  = (long (*)())&get_wall_time;
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
        // to surrender control k_scheduler();
        #if defined (USE_CLOCK_INT)
        reset_irq_timer();
        enable_interrupt();
        __asm__ __volatile__("wfi\n\r":::);
        #endif
        #if !defined (USE_CLOCK_INT)
        k_scheduler();
        #endif
    };
    return 0;
}

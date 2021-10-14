#include <os/mm.h>
#include <os/lock.h>
#include <os/sched.h>
#include <os/list.h>
#include <os/irq.h>
#include <screen.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#define FIFO

pcb_t pcb[NUM_MAX_TASK];
const ptr_t pid0_stack = INIT_KERNEL_STACK + PAGE_SIZE;
pcb_t pid0_pcb = {
    .pid = 0,
    .kernel_sp = (ptr_t)pid0_stack - sizeof(regs_context_t) - sizeof(switchto_context_t),
    .user_sp = (ptr_t)pid0_stack + PAGE_SIZE,
    .preempt_count = 0
};

LIST_HEAD(ready_queue);
LIST_HEAD(blocked_queue);

/* current running task PCB */
pcb_t * volatile current_running;

/* global process id */
pid_t process_id = 1;

pcb_t *dequeue(list_head *queue, int field){
    // plain and simple way
    #ifdef FIFO
    pcb_t *ret;
    if(field == 0){
        ret = list_entry(queue->next,pcb_t,list);
    }
    else if(field == 1){
        ret = list_entry(queue->next,pcb_t,timer_list);
    }
    list_del(queue->next);
    return ret;
    #endif
}

void do_scheduler()
{
    // TODO schedule
    // Modify the current_running pointer.
    pcb_t *curr = current_running;
    if(curr->status == TASK_RUNNING && curr->pid != 0){
        if(list_empty(&ready_queue) || curr->priority == 0){
            list_add_tail(&(curr->list), &ready_queue);
        }
        else{
            list_head *insert_point = list_fetch(&ready_queue,curr->priority);
            list_add(&(curr->list), insert_point);
        }
        curr->status = TASK_READY;
    }
    if(list_empty(&ready_queue)){
        printk("No valid task to run!\n");
        while (1) ;
        assert(0);
    }
    if (!list_empty(&blocked_queue))
    {
        check_timer();
    }
    pcb_t *next_pcb = dequeue(&ready_queue,0);
    next_pcb->status = TASK_RUNNING;
    current_running = next_pcb;
    process_id = next_pcb->pid;
    
    // restore the current_runnint's cursor_x and cursor_y
    load_curpcb_cursor();

    // TODO: switch_to current_running
    switch_to(curr,next_pcb);
}

void do_sleep(uint32_t sleep_time)
{
    // TODO: sleep(seconds)
    // note: you can assume: 1 second = `timebase` ticks
    // 1. block the current_running
    do_block(&current_running->list,&blocked_queue);
    // 2. create a timer which calls `do_unblock` when timeout
    unblock_args_t *args = (unblock_args_t *)kmalloc(sizeof(unblock_args_t));
    args->queue = &blocked_queue;
    args->way = 1;
    create_timer(sleep_time*get_time_base(),(void (*)(void *))&do_unblock,(void *)args);
    // 3. reschedule because the current_running is blocked.
    // must restore context, so see at sys_sleep()
    do_scheduler();
}

void do_block(list_node_t *pcb_node, list_head *queue)
{
    // TODO: block the pcb task into the block queue
    list_add_tail(pcb_node,queue);
    current_running->status = TASK_BLOCKED;
}

void do_unblock(void *args)
{
    // TODO: unblock the `pcb` from the block queue
    unblock_args_t *actual_arg = (unblock_args_t *)args;
    pcb_t *fetch_pcb = dequeue(actual_arg->queue,0);
    fetch_pcb->status = TASK_READY;
    if(actual_arg->way){
        list_add_tail(&fetch_pcb->list,&ready_queue);
    }
    else{
        list_add_tail(&fetch_pcb->list,&ready_queue);
    }
}

long do_fork(void)
{
    pcb_t *curr = current_running, *kid = (pcb_t *)kmalloc(sizeof(pcb_t));
    kid->kernel_sp = allocPage(1);
    kid->user_sp = allocPage(1);
    kid->preempt_count = curr->preempt_count;
    kid->list.prev = NULL;
    kid->list.next = NULL;
    kid->pid = tasks_num++;
    kid->type = curr->type;
    kid->status = TASK_READY;
    kid->cursor_x = curr->cursor_x;
    kid->cursor_y = curr->cursor_y;
    kid->timer.initialized = curr->timer.initialized;
    copy_pcb_stack(kid->kernel_sp,kid->user_sp,kid,curr->kernel_sp,curr->user_sp,curr);
    list_add_tail(&(kid->list),&ready_queue);
}

void copy_pcb_stack(ptr_t kid_kernel_stack, ptr_t kid_user_stack,pcb_t *kid, ptr_t src_kernel_stack, ptr_t src_user_stack, pcb_t *src)
{
    regs_context_t *kid_pt_regs = (regs_context_t *)(kid_kernel_stack - sizeof(regs_context_t));
    regs_context_t *src_pt_regs = (regs_context_t *)(src_kernel_stack - sizeof(regs_context_t));
    for(int i=0;i<32;i++){
        kid_pt_regs->regs[i]=src_pt_regs->regs[i];
    }
    kid_pt_regs->sepc = src_pt_regs->sepc;
    kid_pt_regs->scause = src_pt_regs->scause;
    kid_pt_regs->sbadaddr = src_pt_regs->sbadaddr;
    kid_pt_regs->sie = src_pt_regs->sie;
    kid_pt_regs->sstatus = src_pt_regs->sstatus;

    // set sp to simulate return from switch_to
    /* TODO: you should prepare a stack, and push some values to
     * simulate a pcb context.
     */
    kid->kernel_sp = kid_kernel_stack- sizeof(regs_context_t) - sizeof(switchto_context_t);
    switchto_context_t *kid_stored_switchto_k = (switchto_context_t *) kid->kernel_sp;
    switchto_context_t *src_stored_switchto_k = (switchto_context_t *) src->kernel_sp;
    // kid's ra, after do scheduler of the last task, kid shall go to ret_from_exception
    kid_stored_switchto_k->regs[0] = (reg_t)&ret_from_exception;
    // kid's ksp, copy from src, but shall move to it's own page
    kid_stored_switchto_k->regs[1] = kid_kernel_stack - (PAGE_SIZE - src_stored_switchto_k->regs[1] % PAGE_SIZE);
    memcpy((void *)kid_stored_switchto_k->regs[1], (void *)src_stored_switchto_k->regs[1], PAGE_SIZE - src_stored_switchto_k->regs[1] % PAGE_SIZE);
    for(int i=2;i<14;i++){
        kid_stored_switchto_k->regs[i] = src_stored_switchto_k->regs[i];
    }
    // kid's usp, copy from src, but shall move to it's own page
    kid->user_sp = kid_user_stack - (PAGE_SIZE - src->user_sp % PAGE_SIZE);
    memcpy((void *)kid->user_sp, (void *)src->user_sp, (PAGE_SIZE - src->user_sp % PAGE_SIZE));
}

void set_priority(long priority){
    current_running->priority = priority;
}
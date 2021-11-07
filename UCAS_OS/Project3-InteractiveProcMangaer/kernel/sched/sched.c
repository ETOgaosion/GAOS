#include <os/mm.h>
#include <os/lock.h>
#include <os/sched.h>
#include <os/list.h>
#include <os/irq.h>
#include <screen.h>
#include <stdio.h>
#include <assert.h>
#include <os/string.h>
#include <tasks.h>
#include <os/smp.h>

int cursor_x,cursor_y;

#define print1(str) cursor_x = (*current_running)->cursor_x;\
        cursor_y = (*current_running)->cursor_y;\
        vt100_move_cursor(1,1);\
        printk("                         ");\
        vt100_move_cursor(1,1);\
        printk(str);\
        vt100_move_cursor(cursor_x,cursor_y);

#define print2(str, val) cursor_x = (*current_running)->cursor_x;\
        cursor_y = (*current_running)->cursor_y;\
        vt100_move_cursor(1,1);\
        printk("                         ");\
        vt100_move_cursor(1,1);\
        printk(str, val);\
        vt100_move_cursor(cursor_x,cursor_y);

pcb_t pcb[NUM_MAX_TASK];
const ptr_t pid0_stack_core_m = INIT_KERNEL_STACK + PAGE_SIZE;
const ptr_t pid0_stack_core_s = INIT_KERNEL_STACK + 3 * PAGE_SIZE;
pcb_t pid0_pcb_core_m = {
    .pid = 0,
    .kernel_sp = (ptr_t)pid0_stack_core_m + PAGE_SIZE - sizeof(regs_context_t) - sizeof(switchto_context_t),
    .user_sp = (ptr_t)pid0_stack_core_m,
    .preempt_count = 0
};
pcb_t pid0_pcb_core_s = {
    .pid = 0,
    .kernel_sp = (ptr_t)pid0_stack_core_s + PAGE_SIZE - sizeof(regs_context_t) - sizeof(switchto_context_t),
    .user_sp = (ptr_t)pid0_stack_core_s,
    .preempt_count = 0
};

LIST_HEAD(ready_queue);
LIST_HEAD(blocked_queue);

/* current running task PCB */
pcb_t ** volatile current_running;
pcb_t * volatile current_running_core_m;
pcb_t * volatile current_running_core_s;

pcb_t bubble_pcb;
const ptr_t bubble_stack = INIT_KERNEL_STACK + 5 * PAGE_SIZE;

/* global process id */
pid_t process_id = 1;

pcb_t *dequeue(list_head *queue, int field){
    // plain and simple way
    pcb_t *ret;
    switch (field)
    {
    case 0:
        ret = list_entry(queue->next,pcb_t,list);
        list_del(&(ret->list));
        break;
    case 1:
        ret = list_entry(queue->next,pcb_t,timer_list);
        list_del(&(ret->timer_list));
        break;
    case 2:
        ret = list_entry(queue->next,pcb_t,timer_list);
        list_del(&(ret->timer_list));
        kmemset(&ret->timer,0,sizeof(ret->timer));
        list_init_with_null(&ret->timer_list);
        break;
    case 3:
        ret = choose_sched_task(queue);
        list_del(&(ret->list));
    
    default:
        break;
    }
    return ret;
}

pcb_t *block_current_task()
{
    assert_supervisor_mode();
    // TODO schedule
    // Modify the (*current_running) pointer.
    pcb_t *curr = (*current_running);
    if(curr->status == TASK_RUNNING && curr->pid != 0){
        if(list_is_empty(&ready_queue) || curr->sched_prior.priority == 0){
            list_add_tail(&(curr->list), &ready_queue);
        }
        else{
            list_head *insert_point = list_fetch(&ready_queue,curr->sched_prior.priority);
            list_add(&(curr->list), insert_point);
        }
        curr->status = TASK_READY;
    }
    return curr;
}

void switch_to_next_task(pcb_t *curr)
{
    if(list_is_empty(&ready_queue)){
        if(get_current_cpu_id() == 0){
            (*current_running) = &bubble_pcb;
        }
        else{
            (*current_running) = &bubble_pcb;
        }
        if(kernel_lock.flag == LOCKED){
            unlock_kernel();
        }
        switch_to(curr,(*current_running));
        return;
    }
    if (!list_is_empty(&blocked_queue))
    {
        check_timer();
    }
    #ifdef SCHED_WITH_PRIORITY
    pcb_t *next_pcb = dequeue(&ready_queue,3);
    #endif
    #ifndef SCHED_WITH_PRIORITY
    pcb_t *next_pcb = dequeue(&ready_queue,0);
    #endif
    next_pcb->status = TASK_RUNNING;
    (*current_running) = next_pcb;
    process_id = next_pcb->pid;
    
    // restore the current_runnint's cursor_x and cursor_y
    load_curpcb_cursor();

    // TODO: switch_to (*current_running)
    if(curr){
        switch_to(curr,next_pcb);
    }
    else{
        load_next_task(next_pcb);
    }
}

void k_scheduler()
{
    pcb_t *curr = block_current_task();
    switch_to_next_task(curr);

}

void k_sleep(uint32_t sleep_time)
{
    // TODO: sleep(seconds)
    // note: you can assume: 1 second = `timebase` ticks
    // 1. block the (*current_running)
    k_block(&(*current_running)->list,&blocked_queue);
    // 2. create a timer which calls `k_unblock` when timeout
    create_timer(sleep_time*get_time_base(),(void (*)(void *, int))&k_unblock,&(*current_running)->list);
    // 3. reschedule because the (*current_running) is blocked.
    // must restore context, so see at sys_sleep()
    k_scheduler();
}

void k_block(list_node_t *pcb_node, list_head *queue)
{
    // TODO: block the pcb task into the block queue
    list_add(pcb_node,queue);
    (*current_running)->status = TASK_BLOCKED;
}

void k_unblock(list_head *queue, int way)
{
    // TODO: unblock the `pcb` from the block queue
    pcb_t *fetch_pcb = (pcb_t *)kmalloc(sizeof(pcb_t), (*current_running)->pid * 2 - 1);
    switch (way)
    {
    case 1:
        fetch_pcb = dequeue(queue->prev,0);
        list_add_tail(&fetch_pcb->list,&ready_queue);
        break;
    case 2:
        fetch_pcb = dequeue(queue->prev,0);
        list_add(&fetch_pcb->list,&ready_queue);
        break;
    case 3:
        fetch_pcb = dequeue(queue->prev,0);
        break;
    case 4:
        fetch_pcb = dequeue(queue->prev,2);
        list_add_tail(&fetch_pcb->list,&ready_queue);
        break;

    default:
        break;
    }
    fetch_pcb->status = TASK_READY;
}

int find_pcb(void)
{
    for(int i = 1; i < NUM_MAX_TASK; i++){
        if(pcb[i].pid == 0){
            return i;
        }
    }
    return -1;
}

long k_fork(void)
{
    int pcb_i = find_pcb();
    if(pcb_i == -1){
        return -1;
    }
    // pcb_t *curr = (*current_running), *kid = (pcb_t *)kmalloc(sizeof(pcb_t), (*current_running)->pid * 2 - 1);
    // pcb_t *curr = (*current_running), *kid = &pcb[tasks_num];
    pcb_t *curr = (*current_running), *kid = &pcb[pcb_i];
    kid->user_sp = allocPage(1,2*pcb_i);
    kid->kernel_sp = allocPage(1,2*pcb_i + 1);
    kid->preempt_count = curr->preempt_count;
    init_list_head(&kid->list);
    kid->wait_parent = (*current_running);
    kid->pid = pcb_i;
    kid->owned_lock_num = 0;
    kid->owned_mbox_num = 0;
    kid->type = curr->type;
    kid->status = TASK_READY;
    kid->cursor_x = curr->cursor_x;
    kid->cursor_y = curr->cursor_y + 1;
    kid->timer.initialized = curr->timer.initialized;
    kid->sched_prior.last_sched_time = 0;
    kid->sched_prior.priority = 0;
    copy_pcb_stack(kid->kernel_sp,kid->user_sp,kid,curr->kernel_sp,curr->user_sp,curr);
    list_add_tail(&(kid->list),&ready_queue);
    return kid->pid;
}

void copy_pcb_stack(ptr_t kid_kernel_stack, ptr_t kid_user_stack,pcb_t *kid, ptr_t src_kernel_stack, ptr_t src_user_stack, pcb_t *src)
{
    // set sp to simulate return from switch_to
    /* TODO: you should prepare a stack, and push some values to
     * simulate a pcb context.
     */
    kid->kernel_sp = kid_kernel_stack- sizeof(regs_context_t) - sizeof(switchto_context_t);
    switchto_context_t *kid_stored_switchto_k = (switchto_context_t *) kid->kernel_sp;
    switchto_context_t *src_stored_switchto_k = (switchto_context_t *) src->kernel_sp;
    regs_context_t *kid_pt_regs = (regs_context_t *)(kid_kernel_stack - sizeof(regs_context_t));
    regs_context_t *src_pt_regs = (regs_context_t *)(src_kernel_stack + SWITCH_TO_SIZE);
    
    kid->user_sp = kid->user_sp - src->user_sp + src_pt_regs->regs[8];
    kmemcpy((void *)kid->user_sp, (void *)src->user_sp, (PAGE_SIZE - src->user_sp % PAGE_SIZE));
    
    // kid's ra, after do scheduler of the last task, kid shall go to ret_from_exception
    kid_stored_switchto_k->regs[0] = (reg_t)&ret_from_exception;
    // kid's s0
    kid_stored_switchto_k->regs[2] = kid_kernel_stack - (PAGE_SIZE - src_stored_switchto_k->regs[2] % PAGE_SIZE);
    // kid's ksp, copy from src, but shall move to it's own page
    kid_stored_switchto_k->regs[1] = kid_kernel_stack - (PAGE_SIZE - src_stored_switchto_k->regs[1] % PAGE_SIZE);
    kmemcpy((void *)kid_stored_switchto_k->regs[1], (void *)src_stored_switchto_k->regs[1], PAGE_SIZE - src_stored_switchto_k->regs[1] % PAGE_SIZE - sizeof(regs_context_t) - sizeof(switchto_context_t));
    for(int i=2;i<14;i++){
        kid_stored_switchto_k->regs[i] = src_stored_switchto_k->regs[i];
    }
    // kid's usp, copy from src, but shall move to it's own page
    
    for(int i=0;i<32;i++){
        kid_pt_regs->regs[i]=src_pt_regs->regs[i];
    }
    // kid's s0
    kid_pt_regs->regs[8] = kid->user_sp - src->user_sp + src_pt_regs->regs[8];
    kid_pt_regs->regs[10] = 0;
    kid_pt_regs->sepc = src_pt_regs->sepc;
    kid_pt_regs->scause = src_pt_regs->scause;
    kid_pt_regs->sbadaddr = src_pt_regs->sbadaddr;
    kid_pt_regs->sie = src_pt_regs->sie;
    kid_pt_regs->sstatus = src_pt_regs->sstatus;
}

void set_priority(long priority){
    (*current_running)->sched_prior.priority = priority;
}

uint64_t cal_priority(uint64_t cur_time, uint64_t idle_time, long priority){
    uint64_t mid_div = cur_time/100, mul_res = 1;
    while(mid_div > 10){
        mid_div /= 10;
        mul_res *= 10;
    }
    uint64_t cal_res = cur_time - idle_time + priority * mul_res;
    #ifdef PRINT_PRIORITY
    if(priority == 0){
        int cursor_x = (*current_running)->cursor_x;
        int cursor_y = (*current_running)->cursor_y;
        vt100_move_cursor(1,1);
        printk("priority calculation:\n");
        vt100_move_cursor(1,2);
        printk("cur_time: %lu, idle_time: %lu, priority argument: %ld, mul_res:%lu, cal_res:%lu\n",cur_time,idle_time,priority,mul_res,cal_res);
        vt100_move_cursor(cursor_x,cursor_y);
    }
    else{
        int cursor_x = (*current_running)->cursor_x;
        int cursor_y = (*current_running)->cursor_y;
        vt100_move_cursor(1,priority+3);
        printk("cur_time: %lu, idle_time: %lu, priority argument: %ld, mul_res:%lu, cal_res:%lu\n",cur_time,idle_time,priority,mul_res,cal_res);
        vt100_move_cursor(cursor_x,cursor_y);
    }
    #endif
    return cal_res;
}

pcb_t *choose_sched_task(list_head *queue){
    uint64_t cur_time = get_ticks();
    uint64_t max_priority = 0;
    uint64_t cur_priority = 0;
    list_head *list_iterator = &(*(queue->next));
    pcb_t *max_one = NULL;
    if(list_iterator->next == queue){
        max_one = list_entry(list_iterator,pcb_t,list);
    }
    else{
        pcb_t *pcb_iterator;
        while (list_iterator->next != queue)
        {
            pcb_iterator = list_entry(list_iterator,pcb_t,list);
            cur_priority = cal_priority(cur_time, pcb_iterator->sched_prior.last_sched_time, pcb_iterator->sched_prior.priority);
            if(max_priority < cur_priority){
                max_priority = cur_priority;
                max_one = pcb_iterator;
            }
            list_iterator = list_iterator->next;
        }
    }
    max_one->sched_prior.last_sched_time = cur_time;
    return max_one;
}

pid_t k_spawn(task_info_t *task, void* arg, spawn_mode_t mode)
{
    int pcb_i = find_pcb();
    if(pcb_i == -1){
        return -1;
    }
    // pcb_t *new = (pcb_t *)kmalloc(sizeof(pcb_t), (*current_running)->pid * 2 - 1);
    pcb_t *new = &pcb[pcb_i];
    new->user_sp = allocPage(1, 2*pcb_i);
    new->user_stack_base = new->user_sp - PAGE_SIZE;
    new->kernel_sp = allocPage(1, 2*pcb_i + 1);
    new->kernel_stack_base = new->kernel_sp - PAGE_SIZE;
    #if !defined (USE_CLOCK_INT) // no preempt
    new->preempt_count = 1;
    #endif
    #if defined (USE_CLOCK_INT) // enable preempt
    new->preempt_count = 0;
    #endif
    init_list_head(&new->list);
    new->wait_parent = NULL;
    new->pid = pcb_i + 1;
    new->owned_lock_num = 0;
    new->owned_mbox_num = 0;
    new->type = task->type;
    new->status = TASK_READY;
    new->mode = mode;
    new->cursor_x = 0;
    new->cursor_y = 0;
    #if defined (INIT_WITH_PRIORITY)
    new->sched_prior.priority = i;
    #endif
    #ifndef INIT_WITH_PRIORITY
    new->sched_prior.priority = 0;
    #endif
    new->sched_prior.last_sched_time = 0;
    init_pcb_stack(new->kernel_sp,new->user_sp,task->entry_point,new,arg);
    list_add_tail(&(new->list),&ready_queue);
    return new->pid;
}

void k_exit(void)
{
    k_kill((*current_running)->pid);
}

int k_kill(pid_t pid)
{
    int pcb_i = pid - 1;
    if(pcb_i < 1 || pcb_i >= NUM_MAX_TASK){
        print1("wrong pid!")
        return -1;
    }
    if(pcb[pcb_i].pid == 0){
        print2("No task running on [%d]",pid)
        return -1;
    }

    // realease lock
    for(int i = 0; i < pcb[pcb_i].owned_lock_num; i++){
        k_mutex_lock_release(pcb[pcb_i].lock_keys[i] - 1, pcb[pcb_i].pid);
    }
    if(pcb[pcb_i].mode == ENTER_ZOMBIE_ON_EXIT){
        // wake up parent
        if(pcb[pcb_i].wait_parent){
            pcb_t *parent = pcb[pcb_i].wait_parent;
            if(!parent->timer.initialized){
                k_unblock(&parent->list,1);
            }
        }
        pcb[pcb_i].status = TASK_ZOMBIE;
    }
    else if(pcb[pcb_i].mode == AUTO_CLEANUP_ON_EXIT){
        pcb[pcb_i].pid = 0;
        pcb[pcb_i].status = TASK_EXITED;
    }
    // give up its sons
    for(int i=0; i < NUM_MAX_TASK; i++){
        if(pcb[i].wait_parent == &pcb[pcb_i]){
            pcb[i].wait_parent = pcb[pcb_i].wait_parent;
        }
    }
    if(pcb[pcb_i].list.next)
        list_del(&pcb[pcb_i].list);
    if((*current_running)->pid == pid || (*current_running)->pid == 0 || pcb[pcb_i].mode == AUTO_CLEANUP_ON_EXIT){
        switch_to_next_task(NULL);
    }
    return 0;
}

int k_waitpid(pid_t pid)
{
    int pcb_i = pid - 1;
    if(pcb_i < 1 || pcb_i >= NUM_MAX_TASK){
        return -1;
    }
    if(pcb[pcb_i].pid == 0){
        int cursor_x = (*current_running)->cursor_x;
        int cursor_y = (*current_running)->cursor_y;
        vt100_move_cursor(1,1);
        print1("                         ")
        vt100_move_cursor(1,1);
        print2("No task running on [%d]",pid)
        return -1;
    }
    if(pcb[pcb_i].status != TASK_ZOMBIE && pcb[pcb_i].status != TASK_EXITED ){
        pcb[pcb_i].wait_parent = (*current_running);
        (*current_running)->status = TASK_BLOCKED;
        k_block(&(*current_running)->list,&blocked_queue);
        k_scheduler();
    }
    if(pcb[pcb_i].status == TASK_ZOMBIE){
        kmemset((void *)pcb[pcb_i].user_stack_base,0,2*PAGE_SIZE);
        pcb[pcb_i].status = TASK_EXITED;
        pcb[pcb_i].pid = 0;
        freePage(pcb[pcb_i].user_stack_base,2);
    }
    return 0;
}

void k_process_show()
{
    prints("\nProcesses:\nprocess id: 0 (shell), process status: TASK_RUNNING");
    for (int i = 1; i < NUM_MAX_TASK; i++)
    {
        if(pcb[i].pid != 0){
            switch (pcb[i].status)
            {
            case TASK_BLOCKED:
                prints("\n\rprocess id: %d, process status: BLOCKED",pcb[i].pid);
                break;
            case TASK_EXITED:
                prints("\n\rprocess id: %d, process status: EXITED",pcb[i].pid);
                break;
            case TASK_RUNNING:
                prints("\n\rprocess id: %d, process status: RUNNING",pcb[i].pid);
                break;
            case TASK_ZOMBIE:
                prints("\n\rprocess id: %d, process status: ZOMBIE",pcb[i].pid);
                break;
            default:
                break;
            }
        }
    }
    
}

pid_t k_getpid()
{
    return (*current_running)->pid;
}
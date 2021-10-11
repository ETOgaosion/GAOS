#include <os/mm.h>
#include <os/lock.h>
#include <os/sched.h>
#include <os/list.h>
#include <os/irq.h>
#include <screen.h>
#include <stdio.h>
#include <assert.h>

#define FIFO

pcb_t pcb[NUM_MAX_TASK];
const ptr_t pid0_stack = INIT_KERNEL_STACK + PAGE_SIZE;
pcb_t pid0_pcb = {
    .pid = 0,
    .kernel_sp = (ptr_t)pid0_stack,
    .user_sp = (ptr_t)pid0_stack,
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

void do_scheduler(void)
{
    // TODO schedule
    // Modify the current_running pointer.
    pcb_t *curr = current_running;
    if(curr->status == TASK_RUNNING && curr->pid != 0){
        list_add_tail(&(curr->list), &ready_queue);
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
    vt100_move_cursor(current_running->cursor_x,
                      current_running->cursor_y);
    screen_cursor_x = current_running->cursor_x;
    screen_cursor_y = current_running->cursor_y;

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
    create_timer(sleep_time*get_time_base(),(void (*)(void *))&do_unblock,(void *)&blocked_queue);
    // 3. reschedule because the current_running is blocked.
    // [TASK 4 do this]
    do_scheduler();
    // do_unblock(&blocked_queue);
}

void do_block(list_node_t *pcb_node, list_head *queue)
{
    // TODO: block the pcb task into the block queue
    list_add_tail(pcb_node,queue);
    current_running->status = TASK_BLOCKED;
}

void do_unblock(list_head *queue)
{
    // TODO: unblock the `pcb` from the block queue
    pcb_t *fetch_pcb = dequeue(queue,0);
    fetch_pcb->status = TASK_READY;
    list_add_tail(&fetch_pcb->list,&ready_queue);
}

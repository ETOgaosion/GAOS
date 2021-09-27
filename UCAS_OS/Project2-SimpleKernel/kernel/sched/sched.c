#include <os/mm.h>
#include <os/lock.h>
#include <os/sched.h>
#include <os/list.h>
#include <os/time.h>
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

/* current running task PCB */
pcb_t * volatile current_running;

/* global process id */
pid_t process_id = 1;

pcb_t *select_ready(list_head *queue){
    // plain and simple way
    #ifdef FIFO
    pcb_t *ret = container_of(queue->next,pcb_t,list);
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
    pcb_t *next_pcb = select_ready(&ready_queue);
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
    current_running->status = TASK_BLOCKED;
    // 2. create a timer which calls `do_unblock` when timeout
    // 3. reschedule because the current_running is blocked.
}

void do_block(list_node_t *pcb_node, list_head *queue)
{
    // TODO: block the pcb task into the block queue
}

void do_unblock(list_node_t *pcb_node)
{
    // TODO: unblock the `pcb` from the block queue
}

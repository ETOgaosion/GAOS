#include <os/time.h>
#include <os/mm.h>
#include <os/irq.h>
#include <type.h>
#include <os/string.h>

LIST_HEAD(timers);

uint64_t time_elapsed = 0;
uint32_t time_base = 0;

uint64_t get_ticks()
{
    __asm__ __volatile__(
        "rdtime %0"
        : "=r"(time_elapsed));
    return time_elapsed;
}

uint64_t get_timer()
{
    return get_ticks() / time_base;
}

uint64_t get_time_base()
{
    return time_base;
}

uint32_t get_wall_time(uint64_t *time_elapsed){
    *time_elapsed = get_ticks();
    return get_time_base();
}

void latency(uint64_t time)
{
    uint64_t begin_time = get_timer();

    while (get_timer() - begin_time < time);
    return;
}

void create_timer(uint64_t timeout_ticks, timer_ret timeout_func, list_head *queue){
    (*current_running)->timer.initialized = 1;
    (*current_running)->timer.init_tick = get_ticks();
    (*current_running)->timer.timeout_tick = timeout_ticks + (*current_running)->timer.init_tick;
    (*current_running)->timer.timeout_func = timeout_func;
    (*current_running)->timer.queue = queue;
    list_head *list_iterator = timers.next;
    pcb_t *pcb_iterator = NULL;
    while (list_iterator != &timers)
    {
        pcb_iterator = list_entry(list_iterator,pcb_t,timer_list);
        if(pcb_iterator->timer.timeout_tick > (*current_running)->timer.timeout_tick){
            break;
        }
        list_iterator = list_iterator->next;
    }
    list_add(&((*current_running)->timer_list),list_iterator->prev);
}

void check_timer(void){
    pcb_t *pcb_check = NULL;
    list_node_t *iterator = &(*(timers.next));
    while (!list_is_empty(&timers) && iterator != &timers)
    {
        pcb_check = list_entry(iterator,pcb_t,timer_list);
        if(get_ticks() >= pcb_check->timer.timeout_tick){
            iterator = iterator->next;
            list_del(iterator->prev);
            pcb_check->timer.timeout_func(pcb_check->timer.queue,1);
            kmemset((void *)&pcb_check->timer,0,sizeof(pcb_check->timer));
            list_init_with_null(&pcb_check->timer_list);
        }
        else{
            break;
        }
    }
}

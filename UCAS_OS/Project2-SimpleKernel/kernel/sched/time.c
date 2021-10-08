#include <os/time.h>
#include <os/mm.h>
#include <os/irq.h>
#include <type.h>

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

void latency(uint64_t time)
{
    uint64_t begin_time = get_timer();

    while (get_timer() - begin_time < time);
    return;
}

void create_timer(uint64_t timeout_ticks, timer_ret timeout_func, void *parameter){
    current_running->timer.init_tick = get_ticks();
    current_running->timer.timeout_tick = timeout_ticks + current_running->timer.init_tick;
    current_running->timer.timeout_func = timeout_func;
    current_running->timer.parameter = parameter;
    list_add_tail(&(current_running->timer_list),&timers);
}

void check_timer(void){
    pcb_t *pcb_check;
    list_node_t *iterator = timers.next;
    while (!list_empty(&timers) && iterator != &timers)
    {
        pcb_check = dequeue(&timers,1);
        if(get_ticks() >= pcb_check->timer.timeout_tick){
            list_del(iterator);
            pcb_check->timer.timeout_func(pcb_check->timer.parameter);
        }
        iterator = iterator->next;
    }
}
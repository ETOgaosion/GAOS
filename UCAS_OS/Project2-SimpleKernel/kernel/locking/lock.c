#include <os/lock.h>
#include <os/sched.h>
#include <atomic.h>

int first_time = 0;
long global_lock_id = 0;
mutex_lock_t *locks[LOCK_NUM];

long do_mutex_lock_init(void)
{
    /* TODO */
    if(global_lock_id == LOCK_NUM){
        return -1;
    }
    if(!first_time){
        for(int i=0; i<LOCK_NUM; i++){
            locks[i] = (mutex_lock_t *)kmalloc(sizeof(mutex_lock_t));
            locks[i]->initialized = 0;
        }
        first_time = 1;
    }
    locks[global_lock_id]->lock_id = global_lock_id;
    locks[global_lock_id]->initialized = 1;
    locks[global_lock_id]->lock.flag = UNLOCKED;
    locks[global_lock_id]->lock.guard = UNGUARDED;
    init_list_head(&(locks[global_lock_id]->block_queue));
    global_lock_id++;
    return global_lock_id - 1;
}

long do_mutex_lock_acquire(long key)
{
    /* TODO */
    if(!locks[key]->initialized){
        return -1;
    }
    while (atomic_cmpxchg_d(UNGUARDED, GUARDED, (ptr_t)&(locks[key]->lock.guard)) == GUARDED)
    {
        ;
    }
    if(locks[key]->lock.flag == 0){
        locks[key]->lock.flag = 1;
        locks[key]->lock.guard = 0;
        return locks[key]->lock_id;
    }
    else{
        do_block(&current_running->list,&locks[key]->block_queue);
        locks[key]->lock.guard = 0;
        do_scheduler();
        return -2;
    }
}

long do_mutex_lock_release(long key)
{
    /* TODO */
    if(!locks[key]->initialized){
        return -1;
    }
    while (atomic_cmpxchg_d(UNGUARDED, GUARDED, (ptr_t)&(locks[key]->lock.guard)) == GUARDED)
    {
        ;
    }
    if(list_empty(&locks[key]->block_queue)){
        locks[key]->lock.flag = 0;
    }
    else{
        do_unblock(&locks[key]->block_queue);
    }
    locks[key]->lock.guard = 0;
    return locks[key]->lock_id;
}

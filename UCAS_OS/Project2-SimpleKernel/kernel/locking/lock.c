#include <os/lock.h>
#include <os/sched.h>
#include <atomic.h>

int do_mutex_lock_init(mutex_lock_t *lock)
{
    /* TODO */
    lock->lock_id = global_lock_id;
    global_lock_id++;
    lock->lock.flag = UNLOCKED;
    lock->lock.guard = UNGUARDED;
    init_list_head(&lock->block_queue);
    return lock->lock_id;
}

int do_mutex_lock_acquire(mutex_lock_t *lock)
{
    /* TODO */
    while (atomic_cmpxchg_d(UNGUARDED, GUARDED, (ptr_t)&(lock->lock.guard)) == GUARDED)
    {
        ;
    }
    if(lock->lock.flag == 0){
        lock->lock.flag = 1;
        lock->lock.guard = 0;
    }
    else{
        do_block(&current_running->list,&lock->block_queue);
        lock->lock.guard = 0;
        do_scheduler();
    }
    return lock->lock_id;
}

int do_mutex_lock_release(mutex_lock_t *lock)
{
    /* TODO */
    while (atomic_cmpxchg_d(UNGUARDED, GUARDED, (ptr_t)&(lock->lock.guard)) == GUARDED)
    {
        ;
    }
    if(list_empty(&lock->block_queue)){
        lock->lock.flag = 0;
    }
    else{
        do_unblock(&lock->block_queue);
    }
    lock->lock.guard = 0;
    return lock->lock_id;
}

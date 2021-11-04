#include <os/lock.h>
#include <os/sched.h>

int first_time = 1;
mutex_lock_t *locks[LOCK_NUM];

static inline void assert_supervisor_mode() 
{ 
   __asm__ __volatile__("csrr x0, sscratch\n"); 
}

long do_mutex_lock_op(long *key,int op){
    assert_supervisor_mode();
    if(op == 0){
        return do_mutex_lock_init(key);
    }
    else if(op == 1){
        return do_mutex_lock_acquire(*key - 1);
    }
    else if(op == 2){
        return do_mutex_lock_release(*key - 1);
    }
    else if(op == 3){
        return do_mutex_lock_destroy(key);
    }
    else if(op == 4){
        return do_mutex_lock_trylock(key);
    }
    return -1;
}

static inline int find_lock(){
    int found = 0;
    for(int i = 0; i < LOCK_NUM; i++){
        if(!locks[i]->initialized){
            found = 1;
            return i;
        }
    }
    if(!found){
        return -1;
    }
}

long do_mutex_lock_init(int *key)
{
    assert_supervisor_mode();
    /* TODO */
    if(first_time){
        for(int i=0; i<LOCK_NUM; i++){
            locks[i] = (mutex_lock_t *)kmalloc(sizeof(mutex_lock_t),current_running->pid * 2 - 1);
            locks[i]->initialized = 0;
        }
        first_time = 0;
    }
    if(*key > 0){
        return -2;
    }
    if(*key < 0){
        return -3;
    }
    int lock_i = find_lock();
    if(lock_i < 0){
        return -1;
    }
    locks[lock_i]->lock_id = lock_i + 1;
    locks[lock_i]->initialized = 1;
    locks[lock_i]->lock.flag = UNLOCKED;
    locks[lock_i]->lock.guard = UNGUARDED;
    init_list_head(&(locks[lock_i]->block_queue));
    *key = lock_i + 1;
    return 0;
}

long do_mutex_lock_acquire(long key)
{
    assert_supervisor_mode();
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
        current_running->lock_keys[current_running->owned_lock_num++] = key + 1;
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
    if(list_is_empty(&locks[key]->block_queue)){
        locks[key]->lock.flag = 0;
    }
    else{
        unblock_args_t *args = (unblock_args_t *)kmalloc(sizeof(unblock_args_t),current_running->pid * 2 - 1);
        args->queue = locks[key]->block_queue.next;
        args->way = 2;
        do_unblock(args);
    }
    locks[key]->lock.guard = 0;
    return locks[key]->lock_id;
}

long do_mutex_lock_destroy(long *key){
    if(!locks[*key - 1]->initialized){
        return -1;
    }
    kmemset(locks[*key - 1], 0, sizeof(locks[*key]));
    *key = 0;
    return 0;
}

long do_mutex_lock_trylock(int *key){
    if(*key > 0){
        return -2;
    }
    if(*key < 0){
        return -3;
    }
    int lock_i = find_lock();
    if(lock_i < 0){
        return -1;
    }
    return 0;
}
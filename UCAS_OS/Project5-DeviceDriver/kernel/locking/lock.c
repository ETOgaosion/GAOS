#include <os/lock.h>
#include <os/sched.h>
#include <os/string.h>

int first_time = 1;
mutex_lock_t *locks[LOCK_NUM];

void spin_lock_init(spin_lock_t *lock){
    lock->flag = UNLOCKED;
}

int spin_lock_try_acquire(spin_lock_t *lock){
    return atomic_swap_d(LOCKED,&lock->flag);
}

void spin_lock_acquire(spin_lock_t *lock){
    while(spin_lock_try_acquire(lock) == LOCKED) ;
}

void spin_lock_release(spin_lock_t *lock){
    lock->flag = UNLOCKED;
}

static inline void assert_supervisor_mode() 
{ 
   __asm__ __volatile__("csrr x0, sscratch\n"); 
}

long k_mutex_lock_op(int *key,int op){
    assert_supervisor_mode();
    int operator = (*current_running)->pid;
    if(op == 0){
        return k_mutex_lock_init(key, operator);
    }
    else if(op == 1){
        return k_mutex_lock_acquire(*key - 1, operator);
    }
    else if(op == 2){
        return k_mutex_lock_release(*key - 1, operator);
    }
    else if(op == 3){
        return k_mutex_lock_destroy(key, operator);
    }
    else if(op == 4){
        return k_mutex_lock_trylock(key, operator);
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

long k_mutex_lock_init(int *key, int operator)
{
    assert_supervisor_mode();
    /* TODO */
    if(first_time){
        for(int i=0; i<LOCK_NUM; i++){
            locks[i] = (mutex_lock_t *)kmalloc(sizeof(mutex_lock_t));
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

long k_mutex_lock_acquire(int key, int operator)
{
    assert_supervisor_mode();
    /* TODO */
    if(!locks[key]->initialized){
        return -1;
    }
    while (atomic_cmpxchg(UNGUARDED, GUARDED, (ptr_t)&(locks[key]->lock.guard)) == GUARDED)
    {
        ;
    }
    if(locks[key]->lock.flag == 0){
        locks[key]->lock.flag = 1;
        locks[key]->lock.guard = 0;
        pcb[operator-1].lock_keys[pcb[operator-1].owned_lock_num++] = key + 1;
        return locks[key]->lock_id;
    }
    else{
        k_block(&(*current_running)->list,&locks[key]->block_queue);
        locks[key]->lock.guard = 0;
        k_schedule();
        return -2;
    }
}

long k_mutex_lock_release(int key, int operator)
{
    /* TODO */
    if(!locks[key]->initialized){
        return -1;
    }
    if(pcb[operator-1].owned_lock_num){
        pcb[operator-1].lock_keys[--(*current_running)->owned_lock_num] = 0;
    }
    while (atomic_cmpxchg(UNGUARDED, GUARDED, (ptr_t)&(locks[key]->lock.guard)) == GUARDED)
    {
        ;
    }
    if(list_is_empty(&locks[key]->block_queue)){
        locks[key]->lock.flag = 0;
    }
    else{
        k_unblock(locks[key]->block_queue.next,2);
    }
    locks[key]->lock.guard = 0;
    return locks[key]->lock_id;
}

long k_mutex_lock_destroy(int *key, int operator){
    if(!locks[*key - 1]->initialized){
        return -1;
    }
    while(pcb[operator-1].owned_lock_num){
        pcb[operator-1].lock_keys[--(*current_running)->owned_lock_num] = 0;
    }
    memset(locks[*key - 1], 0, sizeof(locks[*key]));
    *key = 0;
    return 0;
}

long k_mutex_lock_trylock(int *key, int operator){
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
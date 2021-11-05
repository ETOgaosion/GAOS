#include <os/comm.h>
#include <os/sched.h>
#include <string.h>

int sem_first_time = 1;
int cond_first_time = 1;
int barrier_first_time = 1;

Semaphore_t *sem_list[COMM_NUM];
cond_t *cond_list[COMM_NUM];
barrier_t *barrier_list[COMM_NUM];

static int find_free(int type){
    int find = 0;
    switch (type)
    {
    case 0:
        for(int i = 0; i < COMM_NUM; i++){
            if(!sem_list[i]->sem_info.initialized){
                find = 1;
                return i;
            }
        }
        break;
    case 1:
        for(int i = 0; i < COMM_NUM; i++){
            if(!cond_list[i]->cond_info.initialized){
                find = 1;
                return i;
            }
        }
        break;
    case 2:
        for(int i = 0; i < COMM_NUM; i++){
            if(!barrier_list[i]->barrier_info.initialized){
                find = 1;
                return i;
            }
        }
        break;
    
    default:
        break;
    }
    if(!find){
        return -1;
    }
}

long do_commop(int *key, void *arg, int op){
    int operator = current_running->pid;
    switch (op)
    {
    case 0:
        return do_semaphore_init(key, *(int *)arg, operator);
        break;
    case 1:
        return do_semaphore_p(*key - 1, operator);
        break;
    case 2:
        return do_semaphore_v(*key - 1, operator);
        break;
    case 3:
        return do_semaphore_destroy(key, operator);
        break;
    case 4:
        return do_barrier_init(key, *(int *)arg, operator);
        break;
    case 5:
        return do_barrier_wait(*key - 1, operator);
        break;
    case 6:
        return do_barrier_destroy(key, operator);
        break;

    default:
        break;
    }
}

long do_semaphore_init(int *key, int sem, int operator){
    if(sem_first_time){
        for(int i = 0; i < COMM_NUM; i++){
            sem_list[i] = (Semaphore_t *)kmalloc(sizeof(Semaphore_t),current_running->pid * 2 - 1);
            sem_list[i]->sem_info.initialized = 0;
        }
    }
    if(*key > 0){
        return -2;
    }
    if(*key < 0){
        return -3;
    }
    int sem_i = find_free(0);
    if(sem_i < 0){
        return -1;
    }
    sem_list[sem_i]->sem_info.id = sem_i + 1;
    sem_list[sem_i]->sem_info.initialized = 1;
    sem_list[sem_i]->sem = sem;
    init_list_head(&sem_list[sem_i]->wait_queue);
    *key = sem_i + 1;
    return 0;
}

long do_semaphore_p(int key, int operator){
    if(!sem_list[key]->sem_info.initialized){
        return -1;
    }
    sem_list[key]->sem--;
    if(sem_list[key]->sem < 0){
        do_block(&current_running->list,&sem_list[key]->wait_queue);
        do_scheduler();
    }
}

long do_semaphore_v(int key, int operator){
    if(!sem_list[key]->sem_info.initialized){
        return -1;
    }
    sem_list[key]->sem++;
    if(sem_list[key]->sem <= 0 && !list_is_empty(&sem_list[key]->wait_queue)){
        unblock_args_t *args = (unblock_args_t *)kmalloc(sizeof(unblock_args_t),current_running->pid * 2 - 1);
        args->queue = sem_list[key]->wait_queue.next;
        args->way = 2;
        do_unblock(args);
    }
}

long do_semaphore_destroy(int *key, int operator){
    if(!sem_list[*key - 1]->sem_info.initialized){
        return -1;
    }
    kmemset(sem_list[*key - 1], 0, sizeof(sem_list[*key - 1]));
    *key = 0;
    return 0;
}

long do_cond_init(int *key, int operator){
    if(cond_first_time){
        for(int i = 0; i < COMM_NUM; i++){
            cond_list[i] = (cond_t *)kmalloc(sizeof(cond_t),current_running->pid * 2 - 1);
            cond_list[i]->cond_info.initialized = 0;
        }
    }
    if(*key > 0){
        return -2;
    }
    if(*key < 0){
        return -3;
    }
    int cond_i = find_free(1);
    if(cond_i < 0){
        return -1;
    }
    cond_list[cond_i]->cond_info.id = cond_i + 1;
    cond_list[cond_i]->cond_info.initialized = 1;
    cond_list[cond_i]->num_wait = 0;
    init_list_head(&cond_list[cond_i]->wait_queue);
    *key = cond_i + 1;
    return 0;
}

long do_cond_wait(int key, int lock_id, int operator){
    if(!cond_list[key]->cond_info.initialized){
        return -1;
    }
    cond_list[key]->num_wait++;
    do_block(&current_running->list,&cond_list[key]->wait_queue);
    do_mutex_lock_release(lock_id,operator);
    do_scheduler();
    do_mutex_lock_acquire(lock_id,operator);
}

long do_cond_signal(int key, int operator){
    if(!cond_list[key]->cond_info.initialized){
        return -1;
    }
    if(cond_list[key]->num_wait > 0){
        unblock_args_t *args = (unblock_args_t *)kmalloc(sizeof(unblock_args_t),current_running->pid * 2 - 1);
        args->queue = cond_list[key]->wait_queue.next;
        args->way = 2;
        do_unblock(args);
        cond_list[key]->num_wait--;
    }
}

long do_cond_broadcast(int key, int operator){
    if(!cond_list[key]->cond_info.initialized){
        return -1;
    }
    while(cond_list[key]->num_wait > 0){
        unblock_args_t *args = (unblock_args_t *)kmalloc(sizeof(unblock_args_t),current_running->pid * 2 - 1);
        args->queue = cond_list[key]->wait_queue.next;
        args->way = 2;
        do_unblock(args);
        cond_list[key]->num_wait--;
    }
}

long do_cond_destroy(int *key, int operator){
    if(!cond_list[*key - 1]->cond_info.initialized){
        return -1;
    }
    kmemset(cond_list[*key - 1], 0, sizeof(cond_list[*key]));
    *key = 0;
    return 0;
}

long do_barrier_init(int *key, int total, int operator){
    if(barrier_first_time){
        for(int i = 0; i < COMM_NUM; i++){
            barrier_list[i] = (barrier_t *)kmalloc(sizeof(barrier_t),current_running->pid * 2 - 1);
            barrier_list[i]->barrier_info.initialized = 0;
        }
    }
    if(*key > 0){
        return -2;
    }
    if(*key < 0){
        return -3;
    }
    int barrier_i = find_free(2);
    if(barrier_i < 0){
        return -1;
    }
    barrier_list[barrier_i]->barrier_info.id = barrier_i + 1;
    barrier_list[barrier_i]->barrier_info.initialized = 1;
    barrier_list[barrier_i]->count = 0;
    barrier_list[barrier_i]->total = total;
    do_mutex_lock_init(&barrier_list[barrier_i]->mutex_id,operator);
    do_cond_init(&barrier_list[barrier_i]->cond_id,operator);
    *key = barrier_i + 1;
    return 0;
}

long do_barrier_wait(int key, int operator){
    if(!barrier_list[key]->barrier_info.initialized){
        return -1;
    }
    do_mutex_lock_acquire(barrier_list[key]->mutex_id - 1,operator);
    if((++barrier_list[key]->count) == barrier_list[key]->total){
        barrier_list[key]->count = 0;
        do_cond_broadcast(barrier_list[key]->cond_id - 1,operator);
    }
    else{
        do_cond_wait(barrier_list[key]->cond_id - 1, barrier_list[key]->mutex_id - 1,operator);
    }
    do_mutex_lock_release(barrier_list[key]->mutex_id - 1,operator);
}

long do_barrier_destroy(int *key, int operator){
    if(!barrier_list[*key - 1]->barrier_info.initialized){
        return -1;
    }
    do_mutex_lock_destroy(&barrier_list[*key - 1]->mutex_id,operator);
    do_cond_destroy(&barrier_list[*key - 1]->cond_id,operator);
    kmemset(barrier_list[*key - 1], 0, sizeof(barrier_list[*key - 1]));
    *key = 0;
    return 0;
}
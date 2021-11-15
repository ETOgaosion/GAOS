#include <os/comm.h>
#include <os/sched.h>
#include <os/string.h>
#include <screen.h>

int sem_first_time = 1;
int cond_first_time = 1;
int barrier_first_time = 1;
int mbox_first_time = 1;
int print_loc = 3;

Semaphore_t *sem_list[COMM_NUM];
cond_t *cond_list[COMM_NUM];
barrier_t *barrier_list[COMM_NUM];
mbox_t *mbox_list[COMM_NUM];

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
    case 3:
        for(int i = 0; i < COMM_NUM; i++){
            if(!mbox_list[i]->mailbox_info.initialized){
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

int k_commop(void *key_id, void *arg, int op){
    int operator = (*current_running)->pid;
    int *key = (int *)key_id;
    switch (op)
    {
    case 0:
        return k_semaphore_init(key, *(int *)arg, operator);
        break;
    case 1:
        return k_semaphore_p(*key - 1, operator);
        break;
    case 2:
        return k_semaphore_v(*key - 1, operator);
        break;
    case 3:
        return k_semaphore_destroy(key, operator);
        break;
    case 4:
        return k_barrier_init(key, *(int *)arg, operator);
        break;
    case 5:
        return k_barrier_wait(*key - 1, operator);
        break;
    case 6:
        return k_barrier_destroy(key, operator);
        break;
    case 7:
        return k_mbox_open((char *)key_id, operator);
    case 8:
        return k_mbox_close(operator);
    case 9:
        return k_mbox_send(*key - 1, (mbox_arg_t *)arg, operator);
    case 10:
        return k_mbox_recv(*key - 1, (mbox_arg_t *)arg, operator);
    case 11:
        return k_mbox_try_send(*key - 1, (mbox_arg_t *)arg, operator);
    case 12:
        return k_mbox_try_recv(*key - 1, (mbox_arg_t *)arg, operator);

    default:
        break;
    }
}

int k_semaphore_init(int *key, int sem, int operator){
    if(sem_first_time){
        for(int i = 0; i < COMM_NUM; i++){
            sem_list[i] = (Semaphore_t *)kmalloc(sizeof(Semaphore_t),pcb[operator-1].pid * 2 - 1);
            sem_list[i]->sem_info.initialized = 0;
        }
        sem_first_time = 0;
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

int k_semaphore_p(int key, int operator){
    if(!sem_list[key]->sem_info.initialized){
        return -1;
    }
    sem_list[key]->sem--;
    if(sem_list[key]->sem < 0){
        k_block(&(*current_running)->list,&sem_list[key]->wait_queue);
        k_scheduler();
    }
}

int k_semaphore_v(int key, int operator){
    if(!sem_list[key]->sem_info.initialized){
        return -1;
    }
    sem_list[key]->sem++;
    if(sem_list[key]->sem <= 0 && !list_is_empty(&sem_list[key]->wait_queue)){
        k_unblock(sem_list[key]->wait_queue.next,2);
    }
}

int k_semaphore_destroy(int *key, int operator){
    if(!sem_list[*key - 1]->sem_info.initialized){
        return -1;
    }
    kmemset(sem_list[*key - 1], 0, sizeof(sem_list[*key - 1]));
    *key = 0;
    return 0;
}

int k_cond_init(int *key, int operator){
    if(cond_first_time){
        for(int i = 0; i < COMM_NUM; i++){
            cond_list[i] = (cond_t *)kmalloc(sizeof(cond_t),pcb[operator-1].pid * 2 - 1);
            cond_list[i]->cond_info.initialized = 0;
        }
        cond_first_time = 0;
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

int k_cond_wait(int key, int lock_id, int operator){
    if(!cond_list[key]->cond_info.initialized){
        return -1;
    }
    cond_list[key]->num_wait++;
    k_block(&(*current_running)->list,&cond_list[key]->wait_queue);
    k_mutex_lock_release(lock_id,operator);
    k_scheduler();
    k_mutex_lock_acquire(lock_id,operator);
    return 0;
}

int k_cond_signal(int key, int operator){
    if(!cond_list[key]->cond_info.initialized){
        return -1;
    }
    if(cond_list[key]->num_wait > 0){
        k_unblock(cond_list[key]->wait_queue.next,2);
        cond_list[key]->num_wait--;
    }
    return 0;
}

int k_cond_broadcast(int key, int operator){
    if(!cond_list[key]->cond_info.initialized){
        return -1;
    }
    while(cond_list[key]->num_wait > 0){
        k_unblock(cond_list[key]->wait_queue.next,2);
        cond_list[key]->num_wait--;
    }
    return 0;
}

int k_cond_destroy(int *key, int operator){
    if(!cond_list[*key - 1]->cond_info.initialized){
        return -1;
    }
    kmemset(cond_list[*key - 1], 0, sizeof(cond_list[*key]));
    *key = 0;
    return 0;
}

int k_barrier_init(int *key, int total, int operator){
    if(barrier_first_time){
        for(int i = 0; i < COMM_NUM; i++){
            barrier_list[i] = (barrier_t *)kmalloc(sizeof(barrier_t),pcb[operator-1].pid * 2 - 1);
            barrier_list[i]->barrier_info.initialized = 0;
        }
        barrier_first_time = 0;
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
    barrier_list[barrier_i]->mutex_id = 0;
    barrier_list[barrier_i]->cond_id = 0;
    k_mutex_lock_init(&barrier_list[barrier_i]->mutex_id,operator);
    k_cond_init(&barrier_list[barrier_i]->cond_id,operator);
    *key = barrier_i + 1;
    return 0;
}

int k_barrier_wait(int key, int operator){
    if(!barrier_list[key]->barrier_info.initialized){
        return -1;
    }
    k_mutex_lock_acquire(barrier_list[key]->mutex_id - 1,operator);
    if((++barrier_list[key]->count) == barrier_list[key]->total){
        barrier_list[key]->count = 0;
        k_cond_broadcast(barrier_list[key]->cond_id - 1,operator);
    }
    else{
        k_cond_wait(barrier_list[key]->cond_id - 1, barrier_list[key]->mutex_id - 1,operator);
    }
    k_mutex_lock_release(barrier_list[key]->mutex_id - 1,operator);
    return 0;
}

int k_barrier_destroy(int *key, int operator){
    if(!barrier_list[*key - 1]->barrier_info.initialized){
        return -1;
    }
    k_mutex_lock_destroy(&barrier_list[*key - 1]->mutex_id,operator);
    k_cond_destroy(&barrier_list[*key - 1]->cond_id,operator);
    kmemset(barrier_list[*key - 1], 0, sizeof(barrier_list[*key - 1]));
    *key = 0;
    return 0;
}

int k_mbox_open(char *name, int operator){
    if(mbox_first_time){
        for(int i = 0; i < COMM_NUM; i++){
            mbox_list[i] = (mbox_t *)kmalloc(sizeof(mbox_t),pcb[operator-1].pid * 2 - 1);
            mbox_list[i]->mailbox_info.initialized = 0;
        }
        mbox_first_time = 0;
    }
    if(name == NULL){
        return -2;
    }
    for(int i = 0; i < COMM_NUM; i++){
        if(mbox_list[i]->mailbox_info.initialized && kstrcmp(mbox_list[i]->name,name) == 0){
            mbox_list[i]->cited_pid[mbox_list[i]->cited_num++] = operator;
            pcb[operator-1].mbox_keys[pcb[operator-1].owned_mbox_num++] = i + 1;
            return i + 1;
        }
    }
    int mbox_i = find_free(3);
    if(mbox_i < 0){
        return -1;
    }
    mbox_list[mbox_i]->mailbox_info.id = mbox_i + 1;
    mbox_list[mbox_i]->mailbox_info.initialized = 1;
    kstrcpy(mbox_list[mbox_i]->name,name);
    kmemset(mbox_list[mbox_i]->buff,0,sizeof(mbox_list[mbox_i]->buff));
    mbox_list[mbox_i]->read_head = 0;
    mbox_list[mbox_i]->write_tail = 0;
    mbox_list[mbox_i]->used_units = 0;
    mbox_list[mbox_i]->mutex_id = 0;
    mbox_list[mbox_i]->full_cond_id = 0;
    mbox_list[mbox_i]->empty_cond_id = 0;
    k_mutex_lock_init(&mbox_list[mbox_i]->mutex_id,operator);
    k_cond_init(&mbox_list[mbox_i]->full_cond_id,operator);
    k_cond_init(&mbox_list[mbox_i]->empty_cond_id,operator);
    kmemset(mbox_list[mbox_i]->cited_pid,0,sizeof(mbox_list[mbox_i]->cited_pid));
    mbox_list[mbox_i]->cited_num = 0;
    mbox_list[mbox_i]->cited_pid[mbox_list[mbox_i]->cited_num++] = operator;
    pcb[operator-1].mbox_keys[pcb[operator-1].owned_mbox_num++] = mbox_i + 1;
    return mbox_i + 1;
}

int k_mbox_close(int operator){
    for(int i = 0; i < pcb[operator-1].owned_mbox_num; i++){
        mbox_list[pcb[operator-1].mbox_keys[i] - 1]->cited_num--;
        if(mbox_list[pcb[operator-1].mbox_keys[i] - 1]->cited_num == 0){
            k_cond_destroy(&mbox_list[pcb[operator-1].mbox_keys[i] - 1]->full_cond_id,operator);
            k_cond_destroy(&mbox_list[pcb[operator-1].mbox_keys[i] - 1]->empty_cond_id,operator);
            k_mutex_lock_destroy(&mbox_list[pcb[operator-1].mbox_keys[i] - 1]->mutex_id,operator);
            kmemset(mbox_list[pcb[operator-1].mbox_keys[i] - 1],0,sizeof(mbox_list[pcb[operator-1].mbox_keys[i] - 1]));
        }
    }
    return 0;
}

int k_mbox_send(int key, mbox_arg_t *arg, int operator){
    if(!mbox_list[key]->mailbox_info.initialized){
        return -1;
    }
    if(arg->sleep_operation == 1 && k_mbox_try_send(key,arg,operator) < 0){
        return -2;
    }
    k_mutex_lock_acquire(mbox_list[key]->mutex_id - 1,operator);
    int blocked_time = 0;
    while(arg->msg_length > MBOX_MSG_MAX_LEN - mbox_list[key]->used_units){
        blocked_time++;
        k_cond_wait(mbox_list[key]->full_cond_id - 1, mbox_list[key]->mutex_id - 1, operator);
    }
    int left_space = MBOX_MSG_MAX_LEN - (mbox_list[key]->write_tail + arg->msg_length);
    if(left_space < 0){
        kmemcpy(mbox_list[key]->buff + mbox_list[key]->write_tail,arg->msg,MBOX_MSG_MAX_LEN - mbox_list[key]->write_tail);
        kmemcpy(mbox_list[key]->buff,arg->msg + MBOX_MSG_MAX_LEN - mbox_list[key]->write_tail, -left_space);
        mbox_list[key]->write_tail = -left_space;
    }
    else{
        kmemcpy(mbox_list[key]->buff + mbox_list[key]->write_tail,arg->msg,arg->msg_length);
        mbox_list[key]->write_tail += arg->msg_length;
    }
    mbox_list[key]->used_units += arg->msg_length;
    k_cond_broadcast(mbox_list[key]->empty_cond_id - 1,operator);
    k_mutex_lock_release(mbox_list[key]->mutex_id - 1,operator);
    return blocked_time;
}

int k_mbox_recv(int key, mbox_arg_t *arg, int operator){
    if(!mbox_list[key]->mailbox_info.initialized){
        return -1;
    }
    if(arg->sleep_operation == 1 && k_mbox_try_recv(key,arg,operator) < 0){
        return -2;
    }
    k_mutex_lock_acquire(mbox_list[key]->mutex_id - 1,operator);
    int blocked_time = 0;
    while(arg->msg_length > mbox_list[key]->used_units){
        blocked_time++;
        k_cond_wait(mbox_list[key]->empty_cond_id - 1, mbox_list[key]->mutex_id - 1, operator);
    }
    int left_space = MBOX_MSG_MAX_LEN - (mbox_list[key]->read_head + arg->msg_length);
    if(left_space < 0){
        kmemcpy(arg->msg,mbox_list[key]->buff + mbox_list[key]->read_head,MBOX_MSG_MAX_LEN - mbox_list[key]->read_head);
        kmemcpy(arg->msg + MBOX_MSG_MAX_LEN - mbox_list[key]->read_head,mbox_list[key]->buff, -left_space);
        mbox_list[key]->read_head = -left_space;
    }
    else{
        kmemcpy(arg->msg,mbox_list[key]->buff + mbox_list[key]->read_head,arg->msg_length);
        mbox_list[key]->read_head += arg->msg_length;
    }
    mbox_list[key]->used_units -= arg->msg_length;
    k_cond_broadcast(mbox_list[key]->full_cond_id - 1,operator);
    k_mutex_lock_release(mbox_list[key]->mutex_id - 1,operator);
    return blocked_time;
}

int k_mbox_try_send(int key, mbox_arg_t *arg, int operator){
    if(!mbox_list[key]->mailbox_info.initialized){
        return -1;
    }
    k_mutex_lock_acquire(mbox_list[key]->mutex_id - 1,operator);
    if(arg->msg_length > MBOX_MSG_MAX_LEN - mbox_list[key]->used_units){
        k_mutex_lock_release(mbox_list[key]->mutex_id - 1,operator);
        return -2;
    }
    k_mutex_lock_release(mbox_list[key]->mutex_id - 1,operator);
    return 0;
}

int k_mbox_try_recv(int key, mbox_arg_t *arg, int operator){
    if(!mbox_list[key]->mailbox_info.initialized){
        return -1;
    }
    k_mutex_lock_acquire(mbox_list[key]->mutex_id - 1,operator);
    if(arg->msg_length > mbox_list[key]->used_units){
        k_mutex_lock_release(mbox_list[key]->mutex_id - 1,operator);
        return -2;
    }
    k_mutex_lock_release(mbox_list[key]->mutex_id - 1,operator);
    return 0;
}
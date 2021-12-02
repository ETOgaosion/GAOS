#include <stdatomic.h>
#include <mthread.h>
#include <sys/syscall.h>


int mutex_get(mthread_mutex_t *key){
    long res = sys_lockop(&key->id,0);
    if(res == -1 || res == -3){
        return EINVAL;
    }
    else if(res == -2){
        return EBUSY;
    }
    return 0;
}

int mutex_op(mthread_mutex_t *handle, int op){
    return sys_lockop(&handle->id,op);
}

int mthread_mutex_init(void* handle){
    return mutex_get((mthread_mutex_t *)handle);
}

int mthread_mutex_lock(void* handle){
    return mutex_op((mthread_mutex_t *)handle,1);
}

int mthread_mutex_unlock(void* handle){
    return mutex_op((mthread_mutex_t *)handle,2);
}

int mthread_mutex_destroy(void *handle){
    return mutex_op((mthread_mutex_t *)handle,3);
}

int mthread_mutex_trylock(void *handle){
    return mutex_op((mthread_mutex_t *)handle,4);
}


int sem_op(mthread_semaphore_t *handle, int op){
    return sys_commop(&handle->id,&handle->val,op);
}

int mthread_semaphore_init(void* handle, int val)
{
    mthread_semaphore_t *sem = (mthread_semaphore_t *)handle;
    sem->val = val;
    return sem_op((mthread_semaphore_t *)handle, 0);
}

int mthread_semaphore_down(void* handle)
{
    // TODO:
    return sem_op((mthread_semaphore_t *)handle, 1);
}

int mthread_semaphore_up(void* handle)
{
    // TODO:
    return sem_op((mthread_semaphore_t *)handle, 2);
}

int mthread_semaphore_destroy(void* handle)
{
    // TODO:
    return sem_op((mthread_semaphore_t *)handle, 3);
}


int barrier_op(mthread_barrier_t *handle, int op){
    return sys_commop(&handle->id,&handle->total,op+4);
}

int mthread_barrier_init(void* handle, int total)
{
    // TODO:
    mthread_barrier_t *barrier = (mthread_barrier_t *)handle;
    barrier->total = total;
    return barrier_op((mthread_barrier_t *)handle,0);
}

int mthread_barrier_wait(void* handle)
{
    // TODO:
    return barrier_op((mthread_barrier_t *)handle,1);
}

int mthread_barrier_destroy(void* handle)
{
    // TODO:
    return barrier_op((mthread_barrier_t *)handle,2);
}

int mthread_create(mthread_t *thread, void (*start_routine)(void*), void *arg)
{
    return sys_mthread_create((int *)thread, start_routine, arg);
}

int mthread_join(mthread_t thread)
{
    return 0;
}
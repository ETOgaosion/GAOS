#include <stdatomic.h>
#include <mthread.h>
#include <sys/syscall.h>


int mthread_mutex_init(void* handle)
{
    /* TODO: */
    return 0;
}
int mthread_mutex_lock(void* handle) 
{
    /* TODO: */
    return 0;
}
int mthread_mutex_unlock(void* handle)
{
    /* TODO: */
    return 0;
}

int mthread_barrier_init(void* handle, unsigned count)
{
    // TODO:
}
int mthread_barrier_wait(void* handle)
{
    // TODO:
}
int mthread_barrier_destroy(void* handle)
{
    // TODO:
}

int mthread_semaphore_init(void* handle, int val)
{
    // TODO:
}
int mthread_semaphore_up(void* handle)
{
    // TODO:
}
int mthread_semaphore_down(void* handle)
{
    // TODO:
}
int mthread_semaphore_destroy(void* handle)
{
    // TODO:
}

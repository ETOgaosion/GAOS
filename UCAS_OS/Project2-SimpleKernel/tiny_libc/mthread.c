#include <stdatomic.h>
#include <mthread.h>
#include <sys/syscall.h>
#include <os/lock.h>


int mthread_mutex_init(mutex_lock_t *lock)
{
    /* TODO: */
    do_mutex_lock_init(lock);
    return 0;
}
int mthread_mutex_lock(mutex_lock_t *lock)
{
    /* TODO: */
    do_mutex_lock_acquire(lock);
    return 0;
}
int mthread_mutex_unlock(mutex_lock_t *lock)
{
    /* TODO: */
    do_mutex_lock_release(lock);
    return 0;
}

#include <stdatomic.h>
#include <mthread.h>
#include <sys/syscall.h>
#include <os/lock.h>


#define int mthread_mutex_t

int mutex_get(int key){}
int mutex_op(int handle, int op){}

int mthread_mutex_init(void* handle){}
int mthread_mutex_lock(void* handle){}
int mthread_mutex_unlock(void* handle){}
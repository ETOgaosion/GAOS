#include <stdatomic.h>
#include <mthread.h>

int mutex_get(mthread_mutex_t *key){
    long res = sys_getlock();
    if(res < 0){
        return EINVAL;
    }
    key->data = res;
    return 0;
}

int mutex_op(mthread_mutex_t *handle, int op){
    long res;
    res = sys_lockop(handle->data,op);
    if(op == 0){
        if(res == -1){
            return EINVAL;
        }
        else if(res == -2){
            return EBUSY;
        }
        else{
            return 0;
        }
    }
    else{
        if(res == -1){
            return EINVAL;
        }
        else{
            return 0;
        }
    }
}

int mthread_mutex_init(void* handle){
    return mutex_get((mthread_mutex_t *)handle);
}
int mthread_mutex_lock(void* handle){
    return mutex_op((mthread_mutex_t *)handle,0);
}
int mthread_mutex_unlock(void* handle){
    return mutex_op((mthread_mutex_t *)handle,1);
}
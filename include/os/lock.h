/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *            Copyright (C) 2018 Institute of Computing Technology, CAS
 *               Author : Han Shukai (email : hanshukai@ict.ac.cn)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *                                   Thread Lock
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnisched to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * */

#ifndef INCLUDE_LOCK_H_
#define INCLUDE_LOCK_H_

#include <os/list.h>
#include <atomic.h>
#include <stdarg.h>

#define LOCK_NUM 32

typedef enum {
    UNLOCKED,
    LOCKED,
} lock_status_t;

typedef enum {
    UNGUARDED,
    GUARDED,
} guard_status_t;

typedef struct spin_lock{
    volatile lock_status_t flag;
} spin_lock_t;

typedef struct double_spin_lock
{
    volatile lock_status_t flag;
    volatile guard_status_t guard;
} double_spin_lock_t;

typedef struct mutex_lock
{
    int lock_id;
    int initialized;
    double_spin_lock_t lock;
    list_head block_queue;
} mutex_lock_t;

/* init lock */
// for kernel_lock
void spin_lock_init(spin_lock_t *lock);
int spin_lock_try_acquire(spin_lock_t *lock);
void spin_lock_acquire(spin_lock_t *lock);
void spin_lock_release(spin_lock_t *lock);

long k_mutex_lock_op(int *key,int op);
long k_mutex_lock_init(int *key, int operator);
long k_mutex_lock_acquire(int key, int operator);
long k_mutex_lock_release(int key, int operator);
long k_mutex_lock_destroy(int *key, int operator);
long k_mutex_lock_trylock(int *key, int operator);

#endif

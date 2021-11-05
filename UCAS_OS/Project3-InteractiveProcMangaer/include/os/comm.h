#ifndef COMM_H
#define COMM_H

#include <os/list.h>
#include <os/lock.h>

#define COMM_NUM 32

typedef struct basic_info{
    int id;
    int initialized;
} basic_info_t;

typedef struct Semaphore{
    basic_info_t sem_info;
    int sem;
    list_head wait_queue;
} Semaphore_t;

typedef struct cond{
    basic_info_t cond_info;
    int num_wait;
    list_head wait_queue;
} cond_t;

typedef struct barrier{
    basic_info_t barrier_info;
    int count;
    int total;
    int mutex_id;
    int cond_id;
} barrier_t;


long do_commop(int *key, void *arg, int op);

long do_semaphore_init(int *key, int sem, int operator);
long do_semaphore_p(int key, int operator);
long do_semaphore_v(int key, int operator);
long do_semaphore_destroy(int *key, int operator);

long do_cond_init(int *key, int operator);
// long do_cond_wait(int key, int lock_id, int operator);
long do_cond_wait(int key, int lock_id, int operator);
long do_cond_signal(int key, int operator);
long do_cond_broadcast(int key, int operator);
long do_cond_destroy(int *key, int operator);

long do_barrier_init(int *key, int total, int operator);
long do_barrier_wait(int key, int operator);
long do_barrier_destroy(int *key, int operator);

#endif
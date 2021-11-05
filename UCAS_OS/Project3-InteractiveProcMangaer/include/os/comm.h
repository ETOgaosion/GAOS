#ifndef COMM_H
#define COMM_H

#include <os/list.h>
#include <os/lock.h>

#define COMM_NUM 32
#define MBOX_NAME_LEN 64
#define MBOX_MSG_MAX_LEN 128
#define MBOX_MAX_USER 10
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

typedef struct mbox{
    basic_info_t mailbox_info;
    char name[MBOX_NAME_LEN];
    char buff[MBOX_MSG_MAX_LEN];
    int read_head, write_tail;
    int used_units;
    int mutex_id;
    int full_cond_id;
    int empty_cond_id;
    int cited_num;
    int cited_pid[MBOX_MAX_USER];
} mbox_t;

typedef struct mbox_arg{
    int valid;
    void *msg;
    int msg_length;
} mbox_arg_t;

long k_commop(void *key_id, void *arg, int op);

long k_semaphore_init(int *key, int sem, int operator);
long k_semaphore_p(int key, int operator);
long k_semaphore_v(int key, int operator);
long k_semaphore_destroy(int *key, int operator);

long k_cond_init(int *key, int operator);
// long k_cond_wait(int key, int lock_id, int operator);
long k_cond_wait(int key, int lock_id, int operator);
long k_cond_signal(int key, int operator);
long k_cond_broadcast(int key, int operator);
long k_cond_destroy(int *key, int operator);

long k_barrier_init(int *key, int total, int operator);
long k_barrier_wait(int key, int operator);
long k_barrier_destroy(int *key, int operator);

long k_mbox_open(char *name, int operator);
long k_mbox_close(int operator);
long k_mbox_send(int key, mbox_arg_t *arg, int operator);
long k_mbox_recv(int key, mbox_arg_t *arg, int operator);

#endif
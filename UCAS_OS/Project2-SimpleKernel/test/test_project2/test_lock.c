// #define MTHREAD_MUTEX_LOCK
#define MUTEX_LOCK

#include <test2.h>

#ifdef MTHREAD_MUTEX_LOCK
#include <mthread.h>
#endif
#ifdef MUTEX_LOCK
#include <os/lock.h>
#endif

#include <stdbool.h>
#include <stdio.h>
#include <sys/syscall.h>

static int is_init = FALSE;
static char blank[] = {"                                                          "};

/* if you want to use mutex lock, you need define MUTEX_LOCK */
// #define MTHREAD_MUTEX_LOCK
#ifdef MTHREAD_MUTEX_LOCK
static mthread_mutex_t mutex_lock;
#endif

#ifdef MUTEX_LOCK
static mutex_lock_t mutex_lock;
#endif

void lock_task1(void)
{
        int print_location = 3;
        while (1)
        {
                int i;
                if (!is_init)
                {

#ifdef MTHREAD_MUTEX_LOCK
                        mthread_mutex_init(&mutex_lock);
#endif
#ifdef MUTEX_LOCK
                        do_mutex_lock_init(&mutex_lock);
#endif
                        is_init = TRUE;
                }

                sys_move_cursor(1, print_location);
                printf("%s", blank);

                sys_move_cursor(1, print_location);
                printf("> [LOCK TASK 1] Applying for a lock.\n");
                
                sys_yield();

#ifdef MTHREAD_MUTEX_LOCK
                mthread_mutex_lock(&mutex_lock);
#endif
#ifdef MUTEX_LOCK
                do_mutex_lock_acquire(&mutex_lock);
#endif

                for (i = 0; i < 20; i++)
                {
                        sys_move_cursor(1, print_location);
                        printf("> [LOCK TASK 1] Has acquired lock and running.(%d)\n", i);
                        sys_yield();
                }

                sys_move_cursor(1, print_location);
                printf("%s", blank);

                sys_move_cursor(1, print_location);
                printf("> [LOCK TASK 1] Has acquired lock and exited.\n");

#ifdef MTHREAD_MUTEX_LOCK
                mthread_mutex_unlock(&mutex_lock);
#endif
#ifdef MUTEX_LOCK
                do_mutex_lock_release(&mutex_lock);
#endif
                sys_yield();
        }
}

void lock_task2(void)
{
        int print_location = 4;
        while (1)
        {
                int i;
                if (!is_init)
                {

#ifdef MTHREAD_MUTEX_LOCK
                        mthread_mutex_init(&mutex_lock);
#endif
#ifdef MUTEX_LOCK
                        do_mutex_lock_init(&mutex_lock);
#endif
                        is_init = TRUE;
                }

                sys_move_cursor(1, print_location);
                printf("%s", blank);

                sys_move_cursor(1, print_location);
                printf("> [LOCK TASK 2] Applying for a lock.\n");
                
                sys_yield();

#ifdef MTHREAD_MUTEX_LOCK
                mthread_mutex_lock(&mutex_lock);
#endif
#ifdef MUTEX_LOCK
                do_mutex_lock_acquire(&mutex_lock);
#endif

                for (i = 0; i < 20; i++)
                {
                        sys_move_cursor(1, print_location);
                        printf("> [LOCK TASK 2] Has acquired lock and running.(%d)\n", i);
                        sys_yield();
                }

                sys_move_cursor(1, print_location);
                printf("%s", blank);

                sys_move_cursor(1, print_location);
                printf("> [LOCK TASK 2] Has acquired lock and exited.\n");

#ifdef MTHREAD_MUTEX_LOCK
                mthread_mutex_unlock(&mutex_lock);
#endif
#ifdef MUTEX_LOCK
                do_mutex_lock_release(&mutex_lock);
#endif
                sys_yield();
        }
}

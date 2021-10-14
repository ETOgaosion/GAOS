#include <test2.h>
#include <mthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <tasks.h>

static int is_init = false;
static char blank[] = {"                                             "};

/* if you want to use mutex lock, you need define MUTEX_LOCK */
#define MUTEX_LOCK
static mthread_mutex_t mutex_lock = {.data = 0};

void lock_task1(void)
{
        int print_location = 3;
        while (1)
        {
                int i;
                if (!is_init)
                {

#ifdef MUTEX_LOCK
                        is_init = (mthread_mutex_init(&mutex_lock) == 0);
#endif
                }

                sys_move_cursor(1, print_location);
                printf("%s", blank);

                sys_move_cursor(1, print_location);
                printf("> [TASK] Applying for a lock.\n");
                
                #ifndef TASK_4
                sys_yield();
                #endif

#ifdef MUTEX_LOCK
                mthread_mutex_lock(&mutex_lock);
#endif

                for (i = 0; i < 20; i++)
                {
                        sys_move_cursor(1, print_location);
                        printf("> [TASK] Has acquired lock and running.(%d)\n", i);
                        #ifndef TASK_4
                        sys_yield();
                        #endif
                }

                sys_move_cursor(1, print_location);
                printf("%s", blank);

                sys_move_cursor(1, print_location);
                printf("> [TASK] Has acquired lock and exited.\n");

#ifdef MUTEX_LOCK
                mthread_mutex_unlock(&mutex_lock);
#endif
                
                #ifndef TASK_4
                sys_yield();
                #endif
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

#ifdef MUTEX_LOCK
                        is_init = (mthread_mutex_init(&mutex_lock) == 0);
#endif
                }

                sys_move_cursor(1, print_location);
                printf("%s", blank);

                sys_move_cursor(1, print_location);
                printf("> [TASK] Applying for a lock.\n");
                
                #ifndef TASK_4
                sys_yield();
                #endif

#ifdef MUTEX_LOCK
                mthread_mutex_lock(&mutex_lock);
#endif
                
                for (i = 0; i < 20; i++)
                {
                        sys_move_cursor(1, print_location);
                        printf("> [TASK] Has acquired lock and running.(%d)\n", i);
                        #ifndef TASK_4
                        sys_yield();
                        #endif
                }

                sys_move_cursor(1, print_location);
                printf("%s", blank);

                sys_move_cursor(1, print_location);
                printf("> [TASK] Has acquired lock and exited.\n");

#ifdef MUTEX_LOCK
                mthread_mutex_unlock(&mutex_lock);
#endif
                
                #ifndef TASK_4
                sys_yield();
                #endif
        }
}

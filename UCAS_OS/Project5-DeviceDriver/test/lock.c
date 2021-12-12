#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <mthread.h>

static mthread_mutex_t mutex_lock = {.id = 0};

static char blank[] = {"                                             "};

void try_lock(int argc, int **argv){
    int print_location = **argv;

    while (1)
    {
        int i;

        sys_move_cursor(1, print_location);
        printf("%s", blank);

        sys_move_cursor(1, print_location);
        printf("> [TASK] Applying for a lock.\n");

        mthread_mutex_lock(&mutex_lock);

        for (i = 0; i < 20; i++)
        {
            sys_move_cursor(1, print_location);
            printf("> [TASK] Has acquired lock and running.(%d)\n", i);
        }

        sys_move_cursor(1, print_location);
        printf("%s", blank);

        sys_move_cursor(1, print_location);
        printf("> [TASK] Has acquired lock and exited.\n");

        mthread_mutex_unlock(&mutex_lock);
    }

}

int main(int argc, char* argv[])
{
    int print_location = 1;
    mthread_mutex_init(&mutex_lock);
    if (argc >= 1) {
        print_location = (int) atoi((char *)argv);
    }

    mthread_t lock[3];
    for(int i = 0; i < 3; i++){
        mthread_create(&lock[i], try_lock, &print_location);
        print_location++;
    }
    while (1) ;
    
    
    return 0;
}

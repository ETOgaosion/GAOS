#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <test2.h>
#include <sys/syscall.h>
#include <tasks.h>

//#define MAX_INPUT_NUM 10

static char blank[] = {"                                             "};

#define clear sys_move_cursor(1,print_location);\
        printf("%s",blank);

void fork_priority_task(void)
{
    long pid = 0;
    int print_location = 6;
    long priority = 0;
    int inc_num = 0;
    unsigned char in_ch;
    while((in_ch = (unsigned char)sys_read_ch()) == (unsigned char)-1){
        clear
        sys_move_cursor(1,print_location);
        printf(">[TASK 5] This is father process(%d)\n",inc_num++);
    }
    priority = in_ch;
    pid = sys_fork();
    if(pid == 0){
        int kid_num = inc_num;
        int kid_print_location = print_location;
        sys_move_cursor(1,kid_print_location+1);
        printf(">[TASK 5] set priority of children process...\n");
        clear
        sys_setpriority(priority);
        sys_move_cursor(1,kid_print_location + 2);
        printf(">[TASK 5] priority is: (%d)\n",priority);
        sys_setpriority(priority);
        while(1){
            clear
            sys_move_cursor(1,kid_print_location + 3);
            printf(">[TASK 5] This is children process.(%d)\n",kid_num++);
            #ifndef USE_CLOCK_INT
            sys_yield();
            #endif
        }
    }
    else{
        while(1){
            #ifndef USE_CLOCK_INT
            sys_yield();
            #endif
            clear
            sys_move_cursor(1,print_location);
            printf(">[TASK 5] This is father process(%d)\n",inc_num++);
        }
    }
    
}
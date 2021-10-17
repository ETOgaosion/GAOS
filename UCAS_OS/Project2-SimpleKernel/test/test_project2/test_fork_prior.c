#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <test2.h>
#include <sys/syscall.h>
#include <tasks.h>

//#define MAX_INPUT_NUM 10

#define clear(print_location) sys_move_cursor(1,print_location);\
        printf("%s",blank);

void fork_priority_task(void)
{
    long pid = 0;
    register int kids_num = 0;
    int print_location = 6;
    long priority = 0;
    int inc_num = 0;
    unsigned char in_ch;
    while(1){
        #ifndef USE_CLOCK_INT
        sys_yield();
        #endif
        while((in_ch = (unsigned char)sys_read_ch()) == (unsigned char)-1 || in_ch < '0' || in_ch > '9'){
            sys_move_cursor(1,print_location);
            printf(">[TASK 5] This is father process(%d)\n",inc_num++);
            #ifndef USE_CLOCK_INT
            sys_yield();
            #endif
        }
        kids_num++;
        priority = in_ch - '0';
        pid = sys_fork();
        if(pid == 0){
            register int kid_inc_num = inc_num;
            register int kid_print_location = print_location + 4 * (kids_num - 1) + 1;
            sys_move_cursor(1,kid_print_location + 1);
            printf(">[TASK 5] This is kid (%d)\n",kids_num);
            sys_move_cursor(1,kid_print_location + 2);
            printf(">[TASK 5] set priority of children process...\n");
            sys_setpriority(priority);
            sys_move_cursor(1,kid_print_location + 3);
            printf(">[TASK 5] priority is: (%d)\n",priority);
            sys_setpriority(priority);
            while(1){
                sys_move_cursor(1,kid_print_location + 4);
                printf(">[TASK 5] This is children process.(%d)\n",kid_inc_num++);
                #ifndef USE_CLOCK_INT
                sys_yield();
                #endif
            }
        }
    }
}
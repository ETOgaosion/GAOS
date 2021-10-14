#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <test2.h>
#include <sys/syscall.h>
#include <tasks.h>

#define MAX_INPUT_NUM 10

static int priority_read = 0;
static char blank[] = {"                                             "};

#define clear sys_move_cursor(1,print_location);\
        printf("%s",blank);

void fork_priority_task(void)
{
    long pid = 0;
    int inc_num = 0;
    int print_location = 6;
    long priority = 0;
    pid = sys_fork();
    if(pid == 0){
        while (priority_read == 0) ;
        print_location++;
        sys_move_cursor(1,print_location);
        printf(">[TASK 5] set priority of children process...\n");
        sys_setpriority(priority);
        clear
        sys_move_cursor(1,print_location);
        while(1){
            printf(">[TASK 5] This is children process.(%d)\n",inc_num++);
        }
    }
    else{
        char input[MAX_INPUT_NUM];
        int in_pos = 0;
        char in_ch;
        while(1){
            in_pos = 0;
            for(int i = 0; i<MAX_INPUT_NUM; i++){
                input[i] = 0;
            }
            while ((in_ch = sys_read_ch())!='\n' && in_pos<MAX_INPUT_NUM)
            {
                clear
                sys_move_cursor(1,print_location);
                printf(">[TASK 5] This is father process(%d)\n",inc_num++);
                if(in_ch >= '0' || in_ch <= '9'){
                    input[in_pos++] = in_ch;
                }
            }
            priority = atol(input);
            priority_read = 1;
        }
    }
    
}
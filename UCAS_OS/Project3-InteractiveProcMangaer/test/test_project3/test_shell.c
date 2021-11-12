/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *            Copyright (C) 2018 Institute of Computing Technology, CAS
 *               Author : Han Shukai (email : hanshukai@ict.ac.cn)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *                  The shell acts as a task running in user mode.
 *       The main function is to make system calls through the user's output.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so, subject to the following conditions:
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

#include <test.h>
#include <test3.h>
#include <string.h>
#include <os.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define SHELL_BEGIN 25
#define SHELL_INPUT_MAX_WORDS 100
#define SHELL_CMD_MAX_LENGTH 20
#define SHELL_ARG_NUM 3
#define SHELL_ARG_MAX_LENGTH 20
#define SUPPORTED_CMD_NUM 5
#define CURRENT_TASK_NUM 6
#define MAX_CMD_IN_LINES 15

#define BEGIN cmd_in_length = 0;\
    sys_screen_clear();\
    sys_move_cursor(1, SHELL_BEGIN);\
    printf("========================== Welcome Aboard ==========================");

typedef int (*function)(void *arg0, void *arg1, void *arg2);

int cmd_in_length = 0;

struct task_info task_test_waitpid = {
    (uintptr_t)&wait_exit_task, USER_PROCESS};
struct task_info task_test_semaphore = {
    (uintptr_t)&semaphore_add_task1, USER_PROCESS};
struct task_info task_test_barrier = {
    (uintptr_t)&test_barrier, USER_PROCESS};
    
struct task_info strserver_task = {(uintptr_t)&strServer, USER_PROCESS};
struct task_info strgenerator_task = {(uintptr_t)&strGenerator, USER_PROCESS};

struct task_info task_test_multicore = {(uintptr_t)&test_multicore, USER_PROCESS};
struct task_info task_test_affinity = {(uintptr_t)&test_affinity, USER_PROCESS};

static struct task_info *test_tasks[16] = {&task_test_waitpid,
                                           &task_test_semaphore,
                                           &task_test_barrier,
                                           &task_test_multicore,
                                           &strserver_task, &strgenerator_task};
void panic(char *error);
static int shell_help(void *cmd_str, void *arg1, void*arg2);
static int shell_exec(void *pid_str, void *mode_str, void *arg2);
static int shell_kill(void *pid_str, void *arg1, void *arg2);
static void shell_ps(void *arg0, void *arg1, void *arg2);
static void shell_clear(void *arg0, void *arg1, void *arg2);

static struct {
    char *cmd_full_name;
    char *cmd_alias;
    char *description;
    char *format;
    function handler;
    int max_arg_num;
} cmd_table[SUPPORTED_CMD_NUM] = {
    {"help", "h", "Print description of command [cmd] or all supported commands(no args or error cmd)", "help ([cmd])", (int (*)(void *,void *, void *))&shell_help, 1},
    {"exec", "spawn", "Execute task [pid](start from 1) in testset with chosen or default mode:\n\t- mode 1: ENTER_ZOMBIE_ON_EXIT\n\t- mode 2: AUTO_CLEANUP_ON_EXIT", "exec [pid] ([mode])", (int (*)(void *,void *, void *))&shell_exec, 2},
    {"kill", "k", "Kill process [pid](start from 1)", "kill [pid]",(int (*)(void *,void *, void *))&shell_kill, 1},
    {"ps", "ps", "Display all process", "ps", (int (*)(void *,void *, void *))&shell_ps, 0},
    {"clear", "clr", "Clear the screen", "clear", (int (*)(void *,void *, void *))&shell_clear, 0}
};

char cmd_not_found[] = "command not found";
char arg_num_error[] = "arg number or format doesn't match";
char cmd_error[] = "there are errors during the execution of cmd, please check and re-enter";

void panic(char *error){
    printf("\n%s, please retry!",error);
    cmd_in_length++;
}

static int shell_help(void *cmd_str, void *arg1, void*arg2){
    char *cmd = (char *)cmd_str;
    if(cmd[0] != 0){
        for (int i = 0; i < SUPPORTED_CMD_NUM; i++)
        {
            if(strcmp(cmd, cmd_table[i].cmd_full_name)==0 || strcmp(cmd, cmd_table[i].cmd_alias)==0){
                printf("\ncommand: %s, alias: %s, discription: %s, format: %s, max arg number: %d;", cmd_table[i].cmd_full_name, cmd_table[i].cmd_alias, cmd_table[i].description, cmd_table[i].format, cmd_table[i].max_arg_num);
                cmd_in_length++;
                return 0;
            }
        }
        panic(cmd_not_found);
        return -1;
    }
    printf("\nall commands are listed below:");
    cmd_in_length++;
    for (int i = 0; i < SUPPORTED_CMD_NUM; i++)
    {
        printf("\ncommand: %s, alias: %s, discription: %s, format: %s, max arg number: %d;", cmd_table[i].cmd_full_name, cmd_table[i].cmd_alias, cmd_table[i].description, cmd_table[i].format, cmd_table[i].max_arg_num);
        cmd_in_length++;
    }
    return 0;
}

static int shell_exec(void *pid_str, void *mode_str, void *arg2)
{
    int pid = atoi((char *)pid_str);
    int mode = atoi((char *)mode_str);
    if(pid < 1 || pid > CURRENT_TASK_NUM){
        panic(arg_num_error);
        return -1;
    }
    printf("\ntask[%d] will be started soon!",pid);
    cmd_in_length++;
    return (int)sys_spawn(test_tasks[pid - 1], NULL, mode);
}

static int shell_kill(void *pid_str, void *arg1, void *arg2)
{
    int pid = atoi((char *)pid_str);
    if(pid < 1 || pid > CURRENT_TASK_NUM){
        panic(arg_num_error);
        return -1;
    }
    printf("\ntask[%d] will be killed soon!",pid);
    cmd_in_length++;
    return sys_kill(pid);
}

static void shell_ps(void *arg0, void *arg1, void *arg2)
{
    sys_process_show();
}

static void shell_clear(void *arg0, void *arg1, void *arg2){
    sys_screen_clear();
    BEGIN
}

void test_shell()
{
    // TODO:
    BEGIN
    char input_buffer[SHELL_INPUT_MAX_WORDS] = {0};
    char cmd[SHELL_CMD_MAX_LENGTH] = {0};
    char arg[SHELL_ARG_NUM][SHELL_ARG_MAX_LENGTH] = {0};
    memset(arg,0,SHELL_ARG_NUM * SHELL_ARG_MAX_LENGTH);
    char ch;
    int input_length = 0, arg_idx = 0, cmd_res = -1;
    bool cmd_found = false;
    while (1)
    {
        printf("\n[master@GAOS] > ");
        // TODO: call syscall to read UART port
        // 3-^C 4-^D, 8-^H(backspace), 9-^I(\t) 10-^J(new line), 13-^M(\r), 24-^X(cancel), 127-Del, 32~126 readable char
        // TODO: parse input
        // note: backspace maybe 8('\b') or 127(delete)
        while(input_length < SHELL_INPUT_MAX_WORDS){
            ch = sys_serial_read();
            if(ch == 3 || ch == 4 || ch == 24){
                printf("^%c",'A' - 1 + ch);
                goto clear_and_next;
            }
            else if(ch == 9){
                // now we assume tab is space
                sys_serial_write(32);
                input_buffer[input_length++] = 32;
            }
            else if(ch == 10 || ch == 13){
                cmd_in_length++;
                if(input_length == 0){
                    goto clear_and_next;
                }
                break;
            }
            else if(ch > 31 && ch < 127){
                sys_serial_write(ch);
                input_buffer[input_length++] = ch;
            }
            else if(ch == 8 || ch == 127){
                if(input_length > 0){
                    sys_serial_write(ch);
                    input_buffer[--input_length] = 0;
                }
            }
        }
        cmd_in_length++;
        
        // process with input
        // symbol for calculator and pipe will be done through extra work
        char *parse = input_buffer;
        parse = strtok(cmd, parse, ' ', SHELL_CMD_MAX_LENGTH);
        while((parse = strtok(arg[arg_idx++], parse, ' ', SHELL_ARG_MAX_LENGTH)) != NULL) ;
        
        // TODO: ps, exec, kill, clear
        // check whether the command is valid
        for (int i = 0; i < SUPPORTED_CMD_NUM; i++)
        {
            if(strcmp(cmd,cmd_table[i].cmd_full_name)==0 || strcmp(cmd, cmd_table[i].cmd_alias)==0){
                cmd_found = true;
                cmd_res = cmd_table[i].handler(arg[0],arg[1],arg[2]);
                break;
            }
        }
        
        if(!cmd_found){
            panic(cmd_not_found);
        }
        if(cmd_res < 0){
            panic(cmd_error);
        }

        // clear and prepare for next input
clear_and_next:
        if(cmd_in_length > MAX_CMD_IN_LINES){
            BEGIN
        }
        memset(input_buffer,0,sizeof(input_buffer));
        memset(cmd,0,sizeof(cmd));
        memset(arg,0,SHELL_ARG_NUM * SHELL_ARG_MAX_LENGTH);
        input_length = 0;
        arg_idx = 0;
        cmd_res = -1;
        cmd_found = false;
    }
}

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
#include <string.h>
#include <os.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <fs.h>

#define BEGIN cmd_in_length = 0;\
    sys_screen_clear();\
    sys_move_cursor(1, SHELL_BEGIN);\
    printf("========================== Welcome Aboard ==========================");

typedef int (*function)(int argc, char *argv[]);

int cmd_in_length = 0;

typedef struct sys_taskset_arg{
    int pid;
    int mask;
} sys_taskset_arg_t;

char *task_names[] = {"fly","consensus","lock","mailbox","rw","swappage","recv","send","multiportrecv","fs"};

void panic(char *error);
static int shell_help(int argc, char *argv[]);
static int shell_exec(int argc, char *argv[]);
static int shell_kill(int argc, char *argv[]);
static int shell_taskset(int argc, char *argv[]);
static int shell_ps(int argc, char *argv[]);
static int shell_clear(int argc, char *argv[]);
static int shell_mkfs(int argc, char *argv[]);
static int shell_statfs(int argc, char *argv[]);
static int shell_mkdir(int argc, char *argv[]);
static int shell_rmdir(int argc, char *argv[]);
static int shell_cd(int argc, char *argv[]);
static int shell_ls(int argc, char *argv[]);
static int shell_pwd(int argc, char *argv[]);
static int shell_touch(int argc, char *argv[]);
static int shell_cat(int argc, char *argv[]);
static int shell_rm(int argc, char *argv[]);
static int shell_ln(int argc, char *argv[]);

#define HELP 0
#define EXEC 1
#define KILL 2
#define TASKSET 3
#define PS 4
#define CLEAR 5
#define MKFS 6
#define STATFS 7
#define MKDIR 8
#define RMDIR 9
#define CD 10
#define LS 11
#define PWD 12
#define TOUCH 13
#define CAT 14
#define RM 15
#define LN 16

static struct {
    char *cmd_full_name;
    char *cmd_alias;
    char *description;
    char *format;
    function handler;
    int max_arg_num;
    int min_arg_num;
} cmd_table[SUPPORTED_CMD_NUM] = {
    {"help", "h", "Print description of command [cmd] or all supported commands(no args or error cmd)", "help ([cmd])", &shell_help, SHELL_ARG_MAX_NUM,0},
    {"exec", "spawn", "Execute task [pid](start from 1) in testset with chosen or default mode:\n\t- mode 1: ENTER_ZOMBIE_ON_EXIT\n\t- mode 2: AUTO_CLEANUP_ON_EXIT", "exec [pid] ([-m mode]) ([-a args])", &shell_exec, SHELL_ARG_MAX_LENGTH,1},
    {"kill", "k", "Kill process [pid](start from 1)", "kill [pid]",&shell_kill, 1, 1},
    {"taskset","ts","Start a task's or set some task's running core","taskset [mask] [taskid]/taskset -p [mask] [taskid]",&shell_taskset,3,3},
    {"ps", "ps", "Display all process", "ps", &shell_ps, 0,0},
    {"clear", "clr", "Clear the screen", "clear", &shell_clear, 0,0},
    {"mkfs","mkfs", "Initial file system", "mkfs", &shell_mkfs, 0,0},
    {"statfs", "stfs", "Show info of file system", "statfs", &shell_statfs, 0,0},
    {"mkdir", "md", "Create a directory", "mkdir [name]", &shell_mkdir, 1,1},
    {"rmdir", "rmd", "Remove a directory", "rmdir [name]", &shell_rmdir, 1,1},
    {"cd", "c", "Change a directory", "cd [name]", &shell_cd, 1,1},
    {"ls", "l", "Show content of directory", "ls [-option] [name]", &shell_ls, 2,0},
    {"pwd", "p", "Show current path", "pwd", &shell_pwd, 0,0},
    {"touch", "t", "Create a file", "touch [name]", &shell_touch, 1,1},
    {"cat", "cat", "Print content of file to console", "cat [name]", &shell_cat, 1,1},
    {"rm", "rm", "Remove a file", "rm [name]", &shell_rm, 1,1},
    {"ln", "ln", "Link file to another directory", "ln [-s] <path1> <path2>", &shell_ln, 3,2}
};

char cmd_not_found[] = "command not found";
char arg_num_error[] = "arg number or format doesn't match, or we cannot find what you ask";
char cmd_error[] = "there are errors during the execution of cmd, please check and re-enter";

void panic(char *error){
    printf("\n%s, please retry!",error);
    cmd_in_length++;
}

static int shell_help(int argc, char *argv[]){
    if(argc > 0){
        for (int i = 0; i < argc; i++)
        {
            char *cmd = argv[i];
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
        
    }
    else{
        printf("\nall commands are listed below:");
        cmd_in_length++;
        for (int i = 0; i < SUPPORTED_CMD_NUM; i++)
        {
            printf("\ncommand: %s, alias: %s, discription: %s, format: %s, max arg number: %d;", cmd_table[i].cmd_full_name, cmd_table[i].cmd_alias, cmd_table[i].description, cmd_table[i].format, cmd_table[i].max_arg_num);
            cmd_in_length++;
        }
    }
    return 0;
}

static int shell_exec(int argc, char *argv[])
{
    if(argc < cmd_table[EXEC].min_arg_num){
        panic(arg_num_error);
        return -1;
    }
    int task_found = 0;
    char *first_arg = (char *)argv;
    for(int i = 0; i < CURRENT_TASK_NUM; i++){
        if(strcmp(first_arg,task_names[i]) == 0){
            task_found = 1;
            break;
        }
    }
    if(!task_found){
        panic(arg_num_error);
        return -1;
    }
    int mode;
    if(argc == 1){
        mode = 0;
        return (int)sys_spawn(first_arg, 0, NULL, mode);
    }
    char *second_arg = (char *)argv +SHELL_ARG_MAX_LENGTH;
    if(!second_arg || strcmp(second_arg,"-a") == 0){
        mode = ENTER_ZOMBIE_ON_EXIT;
    }
    else if(!strcmp(second_arg,"-m")){
        if(argc <3){
            panic(arg_num_error);
            return -1;
        }
        mode = strtol((char *)argv + 2*SHELL_ARG_MAX_LENGTH,NULL,10);
    }
    if(mode != 0 && mode != 1){
        panic(arg_num_error);
        return -1;
    }
    printf("\ntask[%s] will be started soon!",first_arg);
    cmd_in_length++;
    if(strcmp(second_arg,"-a") == 0){
        argc -= 2;
        return (int)sys_spawn(first_arg, argc, (char *)argv + 2 * SHELL_ARG_MAX_LENGTH, mode);
    }
    else if(strcmp(second_arg,"-m") == 0){
        if(strcmp(((char *)argv + 3 * SHELL_ARG_MAX_LENGTH),"-a") == 0){
            argc -= 4;
            return (int)sys_spawn(first_arg, argc, (char *)argv + 4 * SHELL_ARG_MAX_LENGTH, mode);
        }
    }
    else{
        panic(arg_num_error);
        return -1;
    }
    return 0;
}

static int shell_kill(int argc, char *argv[])
{
    if(argc < cmd_table[KILL].min_arg_num){
        panic(arg_num_error);
        return -1;
    }
    char *first_arg = (char *)argv;
    int pid = strtol(first_arg,NULL,10);
    if(pid < 1 || pid > CURRENT_TASK_NUM){
        panic(arg_num_error);
        return -1;
    }
    printf("\ntask[%d] will be killed soon!",pid);
    cmd_in_length++;
    return sys_kill(pid);
}

static int shell_taskset(int argc, char *argv[])
{
    if(argc < cmd_table[TASKSET].min_arg_num){
        panic(arg_num_error);
        return -1;
    }
    int mask, pid;
    char *first_arg = (char *)argv;
    char *second_arg = (char *)argv + SHELL_ARG_MAX_LENGTH;
    char *third_arg = (char *)argv + 2*SHELL_ARG_MAX_LENGTH;
    if(strcmp(first_arg,"-p") == 0){
        mask = strtol(second_arg,NULL,16);
        pid = strtol(third_arg,NULL,10);
        if(pid < 1 || pid > CURRENT_TASK_NUM){
            panic(arg_num_error);
            return -1;
        }
        taskset_arg_t taskset_args = {.mask = mask, .pid = pid};
        printf("\ntask[%d] mask will be set soon!",pid);
        return sys_taskset((void *)&taskset_args);
    }
    else{
        mask = strtol(first_arg,NULL,16);
        if(shell_exec(argc - 1,argv + strlen(first_arg)) < 0){
            return -1;
        }
        taskset_arg_t taskset_args = {.mask = mask, .pid = pid};
        printf("\ntask[%d] mask will be set soon!",pid);
        return sys_taskset((void *)&taskset_args);
    }
}

static int shell_ps(int argc, char *argv[])
{
    cmd_in_length += sys_process_show();
    return 0;
}

static int shell_clear(int argc, char *argv[]){
    sys_screen_clear();
    BEGIN
    return 0;
}

static int shell_mkfs(int argc, char *argv[])
{
    int option = 0;
    if(argc != 0){
        option = strtol((char *)argv,NULL,16);
    }
    return mkfs(option);
}

static int shell_statfs(int argc, char *argv[])
{
    return statfs();
}

static int shell_mkdir(int argc, char *argv[])
{
    if(argc < cmd_table[MKDIR].min_arg_num){
        panic(arg_num_error);
        return -1;
    }
    char *dirname = (char *)argv;
    return mkdir(dirname);
}

static int shell_rmdir(int argc, char *argv[])
{
    if(argc < cmd_table[RMDIR].min_arg_num){
        panic(arg_num_error);
        return -1;
    }
    char *dirname = (char *)argv;
    return rmdir(dirname);
}

static int shell_cd(int argc, char *argv[])
{
    if(argc < cmd_table[CD].min_arg_num){
        panic(arg_num_error);
        return -1;
    }
    char *dirname = (char *)argv;
    return cd(dirname);
}

static int shell_ls(int argc, char *argv[])
{
    char filename[SHELL_ARG_MAX_LENGTH] = {0};
    int option = 0;
    if(argc != 1 && argc != 2){
        option = -1;
    }
    char *first_arg = (char *)argv;
    if(argc == 1){
        if(first_arg[0] == '-'){
            option = first_arg[1] - 'a';
        }
        else{
            memcpy((uint8_t *)filename, first_arg, SHELL_ARG_MAX_LENGTH);
        }
    }
    char *second_arg = (char *)argv + SHELL_ARG_MAX_LENGTH;
    if(argc == 2){
        if(first_arg[0] != '-'){
            panic(arg_num_error);
        }
        else{
            option = first_arg[1] - 'a';
            memcpy((uint8_t *)filename, second_arg, SHELL_ARG_MAX_LENGTH);
        }
    }
    return ls(filename,option);
}

static int shell_pwd(int argc, char *argv[])
{
    if(argc < cmd_table[PWD].min_arg_num){
        panic(arg_num_error);
        return -1;
    }
    char filename[SHELL_ARG_MAX_LENGTH];
    pwd(filename);
    printf("%s\n",filename);
    return 0;
}

static int shell_touch(int argc, char *argv[])
{
    if(argc < cmd_table[TOUCH].min_arg_num){
        panic(arg_num_error);
        return -1;
    }
    char *filename = (char *)argv;
    return touch(filename);
}

static int shell_cat(int argc, char *argv[])
{
    if(argc < cmd_table[CAT].min_arg_num){
        panic(arg_num_error);
        return -1;
    }
    char *filename = (char *)argv;
    return cat(filename);
}

static int shell_rm(int argc, char *argv[])
{
    if(argc < cmd_table[RM].min_arg_num){
        panic(arg_num_error);
        return -1;
    }
    char *filename = (char *)argv;
    return frm(filename);
}

static int shell_ln(int argc, char *argv[])
{
    if(argc < cmd_table[RM].min_arg_num){
        panic(arg_num_error);
        return -1;
    }
    char *first_arg = (char *)argv;
    char *second_arg = (char *)argv + SHELL_ARG_MAX_LENGTH;
    if(argc == 2){
        return ln(0,first_arg,second_arg);
    }
    if(strcmp(first_arg,"-s") || (strcmp(first_arg,"-h"))){
        panic(arg_num_error);
        return -1;
    }
    char *third_arg = (char *)argv + 2 * SHELL_ARG_MAX_LENGTH;
    return ln(first_arg[1] - 'h' > 0 ? 1 : 0, second_arg, third_arg);
}

// #define DEBUG_WITHOUT_INIT_FS

int main()
{
    // TODO:
    BEGIN
    char input_buffer[SHELL_INPUT_MAX_WORDS] = {0};
    char cmd[SHELL_CMD_MAX_LENGTH] = {0};
    char arg[SHELL_ARG_MAX_NUM][SHELL_ARG_MAX_LENGTH] = {0};
    memset(arg,0,SHELL_ARG_MAX_NUM * SHELL_ARG_MAX_LENGTH);
    char ch;
    int input_length = 0, arg_idx = 0, cmd_res = -1;
    bool cmd_found = false;
    while (1)
    {
        printf("\n[master@GAOS] @");
        char dirname[SHELL_ARG_MAX_LENGTH];
        #ifndef DEBUG_WITHOUT_INIT_FS
        pwd(dirname);
        #endif
        printf("%s",dirname);
        printf(" > ");
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
                printf("\n");
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
        while((parse = strtok(arg[arg_idx++], parse, ' ', SHELL_ARG_MAX_LENGTH)) != NULL && arg_idx < SHELL_ARG_MAX_NUM) ;
        
        // TODO: ps, exec, kill, clear
        // check whether the command is valid
        for (int i = 0; i < SUPPORTED_CMD_NUM; i++)
        {
            if(strcmp(cmd,cmd_table[i].cmd_full_name)==0 || strcmp(cmd, cmd_table[i].cmd_alias)==0){
                cmd_found = true;
                cmd_res = cmd_table[i].handler(arg_idx - 1,arg);
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
        memset(arg,0,SHELL_ARG_MAX_NUM * SHELL_ARG_MAX_LENGTH);
        input_length = 0;
        arg_idx = 0;
        cmd_res = -1;
        cmd_found = false;
    }
    return 0;
}

#ifndef OS_H
#define OS_H

#include <stdint.h>

typedef int32_t pid_t;
typedef uint64_t ptr_t;

typedef enum {
    ENTER_ZOMBIE_ON_EXIT,
    AUTO_CLEANUP_ON_EXIT,
} spawn_mode_t;
typedef struct taskset_arg{
    int pid;
    int mask;
} taskset_arg_t;

typedef enum {
    KERNEL_PROCESS,
    KERNEL_THREAD,
    USER_PROCESS,
    USER_THREAD,
} task_type_t;
typedef struct task_info
{
    ptr_t entry_point;
    task_type_t type;
} task_info_t;

#endif // OS_H

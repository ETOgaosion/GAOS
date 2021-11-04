#include "test.h"

// shell
struct task_info task3 = {(ptr_t)&test_shell,USER_THREAD};
struct task_info *shell_tasks[16] = {&task3};
int num_shell_tasks = 1;
# UCAS_OS lab4

author: Gao Ziyuan   &nbsp;&nbsp;   Stu. Num:2019K8009929026

---

TASK 1\~5 has been finisched

## How to run

### Prerequisites

Have `riscv` cross compiler and `qemu`, or with Xilinx develop board.

### foo way

Directly run `./run_qemu.sh`, input password, and you will see bootloader.

Then input `loadboot`, and enter the only kernel `0`, and you can see three tasks seem to be run at the same time, while two others are run alternatively.

### DIY task

You shall modify into `test/` file folder, head files, and `include/tasks.h`

- the head in original `init/main.c` (running 2 tasks) is like:

```h
#ifndef TASKS_H
#define TASKS_H

// #define TEST_SCHEDULE_1
// #define TASK_1
// #define TEST_LOCK
// #define TASK_2
// #define TEST_TIMER
// #define TEST_SCHEDULE_2
// #define TASK_3
// #define TEST_LOCK_2
// #define TEST_SCHEDULE_2
// #define TASK_4
#define TASK_5


#define USE_CLOCK_INT
// #define INIT_WITH_PRIORITY

#endif

```

you can delete the comment mark to run different tasks.

and as you can see, there are other options:

- use clock int, this will open clock interrupt to schedule tasks preemptly, as a normal mature kernel.

```h
#define USE_CLOCK_INT
```

- init tasks with priority, this will automatically initialize tasks with increasing priority, and you can see the results obviously.

```h
#define INIT_WITH_PRIORITY
```

and back to terminal, run:

```
> make clean

> make

> ./run_qemu.sh

> loadboot

> 0
```

### Task 5 run guide

In task 5, you can add at most 15 children processes(set max number for security reason), after enter the program, you can press `0~9` to set priority of processes, you can view the kid process number, priority, and increasing number of each process in lines below.

If you want to see the calculation results of each process and know the principle behind, please enter the `include/task.h`, uncomment the `// #define PRINT_PRIORITY`

## Report

lab report is in report.md or report.pdf

实验报告位于`report/report.md`和`report/report.pdf`
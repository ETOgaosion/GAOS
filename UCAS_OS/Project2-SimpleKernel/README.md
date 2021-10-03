# UCAS_OS lab2

author: Gao Ziyuan   &nbsp;&nbsp;   Stu. Num:2019K8009929026

---

TASK 1\~2 has been finished

## How to run

### Prerequisites

Have `riscv` cross compiler and `qemu`, or with Xilinx develop board.

### foo way

Directly run `./run_qemu.sh`, input password, and you will see bootloader.

Then input `loadboot`, and enter the only kernel `0`, and you can see three tasks seem to be run at the same time, while two others are run alternatively.

### DIY task

You shall modify into `test/` file folder, head file, and `init/main.c`

- the head in original `init/main.c` (running 2 tasks) is like:

```C
// #define SCHEDULED_1
// #define TASK_1
// #define LOCK
#define TASK_2
```

- If only hope to see task 1, modify the head in `init/main.c`:

```C
#define SCHEDULED_1
// #define TASK_1
// #define LOCK
// #define TASK_2
```

- or only task 2:

```C
// #define SCHEDULED_1
// #define TASK_1
#define LOCK
// #define TASK_2
```

and back to terminal, run:

```
> make clean

> make

> ./run_qemu.sh
```

## Report

lab report is in report.md or report.pdf

实验报告位于`report/report.md`和`report/report.pdf`
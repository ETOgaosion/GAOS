# UCAS_OS lab3

author: Gao Ziyuan   &nbsp;&nbsp;   Stu. Num:2019K8009929026

---

C core has been finisched

## How to run

### Prerequisites

Have `riscv` cross compiler and `qemu`, or with Xilinx develop board.

### foo way

Directly run `./run_qemu.sh`, input password, and you will see bootloader.

Then input `loadboot`, and enter the only kernel `0`, and you can see three tasks seem to be run at the same time, while two others are run alternatively.

### DIY task

You shall modify into `test/test3` file folder, and try to add data structure into `test_shell`

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
## Report

lab report is in report.md or report.pdf

实验报告位于`report/report.md`和`report/report.pdf`
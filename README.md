# UCAS_OS lab - GAOS

---

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
#define USE_CLOCK_INT
// #define INIT_WITH_PRIORITY

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

lab report is in `report.md` or `report.pdf`

## Completion

final score: 91

|Project|Core|Function|
|:-:|:-:|:-|
|1 - bootloader|C|load large kernel from SD<br>multiple OS kernel start|
|2 - simple kernel|C|time interrupt/preempt process schedule<br>syscall and exception handler<br>fork process<br>mutex lock<br>priority scheduling|
|3 - complex kernel|C|implement a simple shell<br>barrier, semaphore and mailbox<br>dual-core start<br>taskset (process bind cpu)|
|4 - virtual memory|A|virtual memory isolation<br>page fault error handling<br>swap page between memory and SD<br>thread|
|5 - device driver|C-|web card interrupt send and recv packages<br>listen on multi-port|
|6 - file system|A-|command:<br>touch, cat, open read, write, close, ln<br>ls, rm, lseek|

core evaluation: S-simple, A, C-Complex
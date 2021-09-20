# Project 1

author: Gao Ziyuan   &nbsp;&nbsp;   Stu. Num:2019K8009929026

---

C-core has been finished

You can load any number of kernels with my Bootloader!

## How to run

### Prerequisites

Have `riscv` cross compiler and `qemu`, or with Xilinx develop board.

### foo way

Directly run `./run_qemu.sh`, input password, and you will see bootloader.

Then input `loadboot`, and follow the instruction, to choose which kernel you want to load. 5 kernels were provided, where 0~3 is kernel0~3, only different in version tag, and 4~5 is large kernel with a 2048 game. All kernel can be correctly load!

### DIY kernel

If you want more kernel and larger kernel, please add them to this directory.

And modify the `Makefile`, line 28:

```Makefile
	./createimage --extended bootblock kernel0 kernel1 kernel2 kernel_large_50200000 kernel_large_50201000 # other kernel, !attention! you must enter their correct name
```

And then in terminal opened in this directory, enter:

```sh
>>> make clean

>>> make

>>> ./run_qemu.sh

### loadboot
```

## Report

lab report is in report.md or report.pdf

实验报告位于`report.md`和`report.pdf`
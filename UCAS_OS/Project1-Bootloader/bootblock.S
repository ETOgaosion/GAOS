#include <asm/sbidef.h>
#include <asm/sbiasm.h>

// #define BOOT_LOADER_SIG_OFFSET 0x1fe
// #define OS_SIZE_LOC (BOOT_LOADER_SIG_OFFSET - 2)
// #define OS_SIZE_LOC 0x1fc

.equ os_num, 0x502001fc
.equ kn0_os_size_loc, 0x502001fa

// 1. kernel address (move kernel to here ~)
.equ kernel, 0x50200000
// .equ kernel, 0x50201000

// 2. kernel main address (jmp here to start kernel main!)
.equ kernel_main, 0x50200000
// .equ kernel_main, 0x50201000

.equ kernel_stack, 0x50500000

// 3. move bootblock to total_os_size
// .equ bbl_new_addr, 0x5efff000
// .equ bbl_new_entrance,0x5efff026
# see by objdump

.text
.global main

main:
	// fence on all memory and I/O
	fence

	// print a \n to avoid system output error...
	PRINT_N

	// 1) task1 call BIOS print string "It's bootblock!"
	lui a0,%hi(msg)
	addi a0,a0,%lo(msg)
	SBI_CALL SBI_CONSOLE_PUTSTR

	PRINT_0

	// print number of kernels
	la a0,os_num
	lh a0,(a0)
	addi a0,a0,47
	mv t0,a0
	SBI_CALL SBI_CONSOLE_PUTCHAR

	PRINT_N

	// C core: read input char
read_input:
	SBI_CALL SBI_CONSOLE_GETCHAR
	blt a0,zero,read_input
	bgt a0,t0,read_input
	addi a1,zero,48
	sub a0,a0,a1	# calculate which kernel: a0

	// load kernel size
	mv a3,a0
	slli a3,a3,1
	la a4,kn0_os_size_loc
	sub a4,a4,a3
	lh a4,(a4)

	addi a3,zero,1
	la t0,kn0_os_size_loc
sum:
	addi a0,a0,-1
	blt a0,zero,move_bootblock
	lh t0,(t0)
	add a3,a3,t0
	addi t0,t0,-2
	j sum

move_bootblock:
	// move bootblock
	la a0,kernel
	slli a5,a4,9
	la a6,kernel_stack
	blt a5,a6,normal
	add a0,a0,a5
	j mov
normal:
	mv a0,a6
	mv t0,a0
mov:
	addi a1,zero,1 #unkown block number
	addi a2,zero,0
	SBI_CALL SBI_SD_READ
	
	fence.i
	// jump to new address
	// calculate entrance
	mv a0,t0
	addi a0,a0,0xc0
	jr a0
	
read_kernel:
	// 2) task2 call BIOS read kernel in SD card and jump to kernel start
	mv a2,a3
	mv a1,a4
	la a0,kernel
	SBI_CALL SBI_SD_READ

	fence.i
	
	// Jump to kernel_main
	la a3,kernel_main
	jr a3

// while(1) --> stop here
stop:
	j stop
	nop

.data

msg: .string "It's a bootloader...\n\rPlease choose which kernel you hope to use, 0 ~ ";

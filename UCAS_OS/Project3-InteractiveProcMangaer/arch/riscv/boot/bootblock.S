#include <asm/sbidef.h>
#include <asm/sbiasm.h>
#include <csr.h>

.equ os_num, 0x502001fc
.equ kn0_os_size_loc, 0x502001fa

// 1. kernel address (move kernel to here ~)
.equ kernel, 0x50201000
// .equ kernel, 0x50201000

// 2. kernel main address (jmp here to start kernel main!)
.equ kernel_main, 0x50201000
// .equ kernel_main, 0x50201000

.equ kernel_stack, 0x50500000

// 3. move bootblock to total_os_size
// .equ bbl_new_addr, 0x5efff000
// .equ bbl_new_entrance,0x5efff026

.text
.global main

main:
	// fence on all memory and I/O
	fence

	// a0 is mhartid, a1 is the pointer of fdt
	move s0,a0
	move s1,a1

	// The core whose mhartid=0 is selected to boot the kernel.
	// Other cores should wait until the kernel's initialization
	// has been finished.
 	move a0, s0
	bnez a0, secondary

    csrw CSR_SIE, zero
	csrw CSR_SIE, zero
	csrw CSR_SSTATUS, zero

	PRINT_N

	// your code 
	lui a0,%hi(msg)
	addi a0,a0,%lo(msg)
	SBI_CALL SBI_CONSOLE_PUTSTR

	PRINT_0

	// print number of kernels
	la a0,os_num
	lh a0,(a0)
	addi a0,a0,47
	mv t0,a0
	li a1,48
	SBI_CALL SBI_CONSOLE_PUTCHAR

read_input:
	SBI_CALL SBI_CONSOLE_GETCHAR
	blt a0,a1,read_input
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
	blt a0,zero,read_kernel
	lh t1,(t0)
	add a3,a3,t1
	addi t0,t0,-2
	j sum
	
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


secondary:
	/* TODO: 
	 * 1. Mask all interrupts
	 * 2. let stvec pointer to kernel_main
	 * 3. enable software interrupt for ipi
	 */
    csrw CSR_SIE, zero
	la a0,kernel_main
	csrw CSR_STVEC, a0
	li a0, SIE_SSIE
	csrw CSR_SIE, a0
	li a0, SR_SIE
	csrw CSR_SSTATUS, a0
	la a0, msg2

    fence.i

wait_for_wakeup:
	wfi
	j wait_for_wakeup
	
// while(1) --> stop here
stop:
	j stop
	nop

.data

msg: .string "Master core activated\n\rIt's a bootloader...\n\rPlease choose which kernel you hope to use, 0 ~ ";
msg2: .string "Slave core activated\n\r"

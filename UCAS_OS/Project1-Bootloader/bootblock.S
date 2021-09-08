#include <asm/sbidef.h>
#include <asm/sbiasm.h>

// #define BOOT_LOADER_SIG_OFFSET 0x1fe
// #define OS_SIZE_LOC (BOOT_LOADER_SIG_OFFSET - 2)
// #define OS_SIZE_LOC 0x1fc

.equ os_size_loc, 0x502001fc

// 1. kernel address (move kernel to here ~)
.equ kernel, 0x50201000

// 2. kernel main address (jmp here to start kernel main!)
.equ kernel_main, 0x50201000

// 3. move bootblock to 0x5e00000
//.equ bbl_new_addr, 0x5e00000

.text
.global main

main:
	// fence on all memory and I/O
	fence
	
	// move bootblock
	/*lui a0,%hi(bbl_new_addr)
	addi a0,a0,%lo(bbl_new_addr)
	addi a1,zero,1 #unkown block number
	addi a2,zero,0
	SBI_CALL SBI_SD_READ
	
	fence.i    #refresh I-cache
	
	// jump to new address
	*/

	// print a \n to avoid system output error...
	li a0, 10
	SBI_CALL SBI_CONSOLE_PUTCHAR

	// 1) task1 call BIOS print string "It's bootblock!"
	lui a0,%hi(msg)
	addi a0,a0,%lo(msg)
	SBI_CALL SBI_CONSOLE_PUTSTR

	// 2) task2 call BIOS read kernel in SD card and jump to kernel start
	la a0,kernel
	la a1,os_size_loc
	lh a1,(a1)
	addi a2,zero,1
	SBI_CALL SBI_SD_READ
	
	// Jump to kernel_main
	la a3,kernel_main
	jr a3
	

// while(1) --> stop here
stop:
	j stop
	nop

.data

msg: .string "It's a bootloader...\n\r"


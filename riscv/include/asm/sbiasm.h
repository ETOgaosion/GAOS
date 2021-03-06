#ifndef _SBI_ASM_H
#define _SBI_ASM_H

.macro SBI_CALL which
    li a7, \which
    ecall
    nop
.endm

.macro PRINT_N which
    li a0, 10
	SBI_CALL SBI_CONSOLE_PUTCHAR
.endm

.macro PRINT_0 which
    li a0,0
	SBI_CALL SBI_CONSOLE_PUTCHAR
.endm

.macro PRINT_a0 which
    addi a0,a0,48
    SBI_CALL SBI_CONSOLE_PUTCHAR
    addi a0,a0,-48
.endm

#endif /* _SBI_ASM_H */

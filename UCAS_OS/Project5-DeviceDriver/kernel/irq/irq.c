#include <os/irq.h>
#include <os/time.h>
#include <os/sched.h>
#include <os/string.h>
#include <os/stdio.h>
#include <assert.h>
#include <sbi.h>
#include <screen.h>
#include <csr.h>
#include <ticks.h>
#include <os/smp.h>
#include <pgtable.h>
#include <io.h>
#include <plic.h>
#include <emacps/xemacps_example.h>
#include <emacps/xemacps.h>
#include <os/list.h>

handler_t irq_table[IRQC_COUNT];
handler_t exc_table[EXCC_COUNT];
uintptr_t riscv_dtb;

void reset_irq_timer()
{
    // TODO clock interrupt handler.
    // TODO: call following functions when task4
    screen_reflush();
    check_timer();

    // note: use sbi_set_timer
    // remember to reschedule
    sbi_set_timer(get_ticks() + get_time_base()/TICKS_INTERVAL);
    k_scheduler();
}

void interrupt_helper(regs_context_t *regs, uint64_t stval, uint64_t cause)
{
    // TODO interrupt handler.
    // call corresponding handler by the value of `cause`
    current_running = (get_current_cpu_id() == 0) ? &current_running_core_m : &current_running_core_s;
    if((*current_running)->pid != -1 && (*current_running)->pid != 0){
        kernel_move_cursor((*current_running)->cursor_x,(*current_running)->cursor_y);
    }
    handler_t *handle_table = (cause & SCAUSE_IRQ_FLAG) ? irq_table : exc_table;
    uint64_t exception_code = cause & ~SCAUSE_IRQ_FLAG;
    handle_table[exception_code](regs,stval,cause);
}

void handle_int(regs_context_t *regs, uint64_t interrupt, uint64_t cause)
{
    reset_irq_timer();
}

// !!! NEW: handle_irq
extern uint64_t read_sip();
void handle_irq(regs_context_t *regs, int irq)
{
    // TODO: 
    // handle external irq from network device
    uint32_t ISR_reg;
    ISR_reg = XEmacPs_ReadReg(EmacPsInstance.Config.BaseAddress, XEMACPS_ISR_OFFSET);
    if(ISR_reg & XEMACPS_IXR_FRAMERX_MASK){      //complete getting a packet
        int recieved_num=0;
        while(bd_space[recieved_num]%2){
            recieved_num++;
        }
        if((bd_space[recieved_num-1]%4)==3){   //reieve enough packet
            if (!list_is_empty(&recv_queue)) {
                k_unblock(recv_queue.prev,1);
            }
        }
        //set XEMACPS_ISR_OFFSET for complete recieve
        XEmacPs_WriteReg(EmacPsInstance.Config.BaseAddress, XEMACPS_ISR_OFFSET,ISR_reg); 
        //set RXSR for complete recieve
        XEmacPs_WriteReg(EmacPsInstance.Config.BaseAddress, XEMACPS_RXSR_OFFSET,XEMACPS_RXSR_FRAMERX_MASK);
        // NOTE: remember to flush dcache
        Xil_DCacheFlushRange(0, 64);
        // let PLIC know that handle_irq has been finished
        plic_irq_eoi(irq);
    }
    if(ISR_reg & XEMACPS_IXR_TXCOMPL_MASK){      //complete sending a packet
        if (!list_is_empty(&send_queue)) {
            k_unblock(send_queue.prev,1);
        }
        //set XEMACPS_ISR_OFFSET for complete recieve/send
        XEmacPs_WriteReg(EmacPsInstance.Config.BaseAddress, XEMACPS_ISR_OFFSET,ISR_reg);  
        //set TXSR for complete recieve
        uint32_t TXSR_reg;
        TXSR_reg = XEmacPs_ReadReg(EmacPsInstance.Config.BaseAddress, XEMACPS_TXSR_OFFSET);
        XEmacPs_WriteReg(EmacPsInstance.Config.BaseAddress, XEMACPS_TXSR_OFFSET,TXSR_reg|XEMACPS_TXSR_TXCOMPL_MASK);  
        // NOTE: remember to flush dcache
        Xil_DCacheFlushRange(0, 64);
        // let PLIC know that handle_irq has been finished
        plic_irq_eoi(irq);
    }
}

void init_exception()
{
    /* TODO: initialize irq_table and exc_table */
    /* note: handle_int, handle_syscall, handle_other, etc.*/
    int i = 0;
    for (; i<IRQC_COUNT; i++)
    {
        irq_table[i] = &handle_int;
    }
    irq_table[IRQC_S_EXT           ] = (void (*)(struct regs_context *, long unsigned int,  long unsigned int))&plic_handle_irq;
    for(i=0; i<EXCC_COUNT; i++){
        exc_table[i] = &handle_other;
    }
    exc_table[EXCC_SYSCALL] = &handle_syscall;
    exc_table[EXCC_INST_PAGE_FAULT ] = &handle_inst_pagefault;
    exc_table[EXCC_LOAD_PAGE_FAULT ] = &handle_load_store_pagefault;
    exc_table[EXCC_STORE_PAGE_FAULT] = &handle_load_store_pagefault;
    setup_exception();
}

void handle_inst_pagefault(regs_context_t *regs, uint64_t stval, uint64_t cause)
{
    uintptr_t kva;
    if ((kva = check_page_helper(stval,(*current_running)->pgdir)) != 0)
    {
        set_attribute((PTE *)kva, _PAGE_ACCESSED | _PAGE_DIRTY);
        local_flush_tlb_all();
        return ;
    } else {
        printk("> Error: Inst page fault: %lx, %lx\n\r",kva, *(PTE *)kva);
        handle_other(regs,stval,cause);
    }
}

void handle_load_store_pagefault(regs_context_t *regs, uint64_t stval, uint64_t cause)
{
    //swap
    if((*current_running)->owned_page_num >= MAX_PAGE_NUM_PER_PCB){
        swap_page_helper(1);
    } 
    uintptr_t kva;
    kva = search_last_page_helper(stval,(*current_running)->pgdir);
    if(kva == 0 || *(PTE *)kva == 0){
        kva = alloc_page_helper_user((uintptr_t)stval, (*current_running)->pgdir);
        adjust_page_list_helper(kva,stval & USER_SPACE);
    }
    else if(*(PTE *)kva & _PAGE_VALID){
        set_attribute((PTE *)kva, _PAGE_ACCESSED | ((cause == 15) ? _PAGE_DIRTY : 0));
    }
    else{
        swap_page_with_sd(kva,1,(cause == 15));
        adjust_page_list_helper(kva,stval & USER_SPACE);
    }
    local_flush_tlb_all();
}

void handle_other(regs_context_t *regs, uint64_t interrupt, uint64_t cause)
{
    char* reg_name[] = {
        "zero "," ra  "," sp  "," gp  "," tp  ",
        " t0  "," t1  "," t2  ","s0/fp"," s1  ",
        " a0  "," a1  "," a2  "," a3  "," a4  ",
        " a5  "," a6  "," a7  "," s2  "," s3  ",
        " s4  "," s5  "," s6  "," s7  "," s8  ",
        " s9  "," s10 "," s11 "," t3  "," t4  ",
        " t5  "," t6  "
    };
    for (int i = 0; i < 32; i += 3) {
        for (int j = 0; j < 3 && i + j < 32; ++j) {
            printk("%s : %016lx ",reg_name[i+j], regs->regs[i+j]);
        }
        printk("\n\r");
    }
    printk("current running pid:%d\n\r",(*current_running)->pid);
    printk("current running preempt_count:%d\n\r",(*current_running)->preempt_count);
    printk("sstatus: 0x%lx sbadaddr: 0x%lx scause: %lu\n\r",
           regs->sstatus, regs->sbadaddr, regs->scause);
    printk("sepc: 0x%lx\n\r", regs->sepc);
    printk("sie: 0x%lx\n\r", regs->sie);
    assert(0);
}

void unknown_syscall(regs_context_t *regs, uint64_t interrupt, uint64_t cause){
    printk(">[ERROR] unkonwn syscall, undefined syscall number: %d\n!",regs->regs[17]);
    handle_other(regs,interrupt,cause);
}
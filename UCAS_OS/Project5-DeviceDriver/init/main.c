#include <common.h>
#include <os/irq.h>
#include <os/mm.h>
#include <os/sched.h>
#include <screen.h>
#include <sbi.h>
#include <os/time.h>
#include <os/lock.h>
#include <os/comm.h>
#include <csr.h>
#include <tasks.h>
#include <ticks.h>
#include <os/stdio.h>
#include <os/string.h>
#include <os/smp.h>
#include <pgtable.h>
#include <os/elf.h>
#include <sys/syscall_number.h>
#include <sys/syscall.h>
#include <plic.h>
#include <emacps/xemacps_example.h>
#include <net.h>
#include <os/ioremap.h>
#include <assert.h>

extern void ret_from_exception();
extern void __global_pointer$();
extern void kp_ret_from_exception();
task_info_t **tasks;
long tasks_num;

void init_pcb_stack(
    ptr_t kernel_stack, ptr_t user_stack, ptr_t entry_point,
    pcb_t *pcb, int argc, char *argv[])
{
    regs_context_t *pt_regs =
        (regs_context_t *)(kernel_stack - sizeof(regs_context_t));
    
    /* TODO: initialization registers
     * note: sp, gp, ra, sepc, sstatus
     * gp should be __global_pointer$
     * To run the task in user mode,
     * you should set corresponding bits of sstatus(SPP, SPIE, etc.).
     */
    for(int i=0;i<32;i++){
        pt_regs->regs[i]=0;
    }
    pt_regs->regs[1] = entry_point;
    if(pcb->type == USER_PROCESS || KERNEL_PROCESS){
        pt_regs->regs[3] = (reg_t)__global_pointer$;
    }
    else{
        regs_context_t *pr_regs = (regs_context_t *)((*current_running)->kernel_sp + sizeof(switchto_context_t));
        pt_regs->regs[3] = pr_regs->regs[3];
    }
    pt_regs->regs[4] = (reg_t)pcb;
    pt_regs->regs[10] = (reg_t)argc;
    pt_regs->regs[11] = (reg_t)argv;
    pt_regs->sepc = entry_point;
    pt_regs->scause = 0;
    pt_regs->sbadaddr = 0;
    pt_regs->sie = 0;
    if(pcb->type == USER_PROCESS || pcb->type == USER_THREAD){
        pt_regs->regs[2] = user_stack;
        pt_regs->sstatus = 0;
    }
    else{
        pt_regs->regs[2] = kernel_stack - sizeof(regs_context_t) - sizeof(switchto_context_t);
        pt_regs->sstatus = pt_regs->sstatus | SR_SPP | SR_SPIE;
    }

    // set sp to simulate return from switch_to
    /* TODO: you should prepare a stack, and push some values to
     * simulate a pcb context.
     */
    pcb->kernel_sp = kernel_stack- sizeof(regs_context_t) - sizeof(switchto_context_t);
    switchto_context_t *stored_switchto_k = (switchto_context_t *) pcb->kernel_sp;
    if(pcb->type == USER_PROCESS || pcb->type == USER_THREAD){
        stored_switchto_k->regs[0] = (reg_t)&ret_from_exception;
    }
    else{
        stored_switchto_k->regs[0] = entry_point;
    }
    stored_switchto_k->regs[1] = pcb->kernel_sp;
    for(int i=2;i<4;i++){
        stored_switchto_k->regs[i] = pt_regs->regs[i+7];
    }
    for(int i=4;i<14;i++){
        stored_switchto_k->regs[i] = pt_regs->regs[i+15];
    }
}

void init_pcb_stack_pointer(pcb_t *pcb){
    if(pcb->type == USER_PROCESS || pcb->type == KERNEL_PROCESS){
        pcb->pgdir = (PTE *)(allocPage() - PAGE_SIZE);
        clear_pgdir((uintptr_t)pcb->pgdir);
        cancel_direct_map(0x50000000);
        memcpy((char *)pcb->pgdir, (char *)pa2kva(PGDIR_PA), PAGE_SIZE);
        // user stack
        pcb->user_sp_useeable = (USER_STACK_BIOS + PAGE_SIZE) & ~((((uint64_t)1) << 7) - 1);
        pcb->user_sp_kseeonly = (uint64_t)alloc_page_helper((uintptr_t)USER_STACK_BIOS, (uintptr_t)pcb->pgdir, 1, 0);
        // kernel stack
        pcb->kernel_sp = (uint64_t)alloc_page_helper((uintptr_t)KERNEL_STACK_BIOS, (uintptr_t)pcb->pgdir, 0, 0);
    }
    else{
        pcb->pgdir = (*current_running)->pgdir;
        // user stack
        pcb->user_sp_useeable = (USER_STACK_BIOS - PAGE_SIZE * ((*current_running)->thread_num + 1)) & ~((((uint64_t)1) << 7) - 1);
        pcb->user_sp_kseeonly = (uint64_t)alloc_page_helper((uintptr_t)USER_STACK_BIOS - PAGE_SIZE * ((*current_running)->thread_num + 2), (uintptr_t)pcb->pgdir, 1, 0);
        // kernel stack
        int current_running_kpage_num = 0;
        list_head *curr = (*current_running)->k_plist.next;
        while (curr != &(*current_running)->k_plist)
        {
            current_running_kpage_num++;
            curr = curr->next;
        }
        pcb->kernel_sp = (uint64_t)alloc_page_helper((uintptr_t)(KERNEL_STACK_BIOS + (current_running_kpage_num + (*current_running)->thread_num + 1) * PAGE_SIZE), (uintptr_t)pcb->pgdir, 0, 0);
    }
    pcb->user_stack_base = pcb->user_sp_kseeonly - PAGE_SIZE + 1;
    pcb->user_sp_kseeonly &= ~((((uint64_t)1) << 7) - 1);
    pcb->kernel_stack_base = pcb->kernel_sp - PAGE_SIZE + 1;
    pcb->kernel_sp &=  ~((((uint64_t)1) << 7) - 1);
    // page list
    init_list_head(&(pcb->k_plist));
    init_list_head(&(pcb->u_plist));
    // page info list
    page_t *kstack = (page_t *)kmalloc(sizeof(page_t));
    page_t *ustack = (page_t *)kmalloc(sizeof(page_t));
    kstack->pa = kva2pa(pcb->kernel_stack_base);
    kstack->va = pcb->kernel_stack_base;
    ustack->pa = kva2pa(pcb->user_stack_base);
    ustack->va = pcb->user_stack_base;
    list_add_tail(&(kstack->list), &(pcb->k_plist));
    list_add_tail(&(ustack->list), &(pcb->u_plist));
    pcb->owned_page_num++;
}

void init_pcb_block(pcb_t *pcb, task_type_t pcb_type){
    pcb->type = pcb_type;
    init_pcb_stack_pointer(pcb);
    init_list_head(&pcb->thread_list);
    int init_ticks = get_ticks();
    // user stack is below kernel stack for security
    #if !defined (USE_CLOCK_INT) // no preempt
    pcb->preempt_count = 1;
    #endif
    #if defined (USE_CLOCK_INT) // enable preempt
    pcb->preempt_count = 0;
    #endif
    init_list_head(&pcb->list);
    pcb->wait_parent = NULL;
    pcb->owned_lock_num = 0;
    pcb->owned_mbox_num = 0;
    pcb->status = TASK_READY;
    pcb->cursor_x = 0;
    pcb->cursor_y = 0;
    pcb->timer.initialized = 0;
    #if defined (INIT_WITH_PRIORITY)
    pcb->sched_prior.priority = i;
    #endif
    #ifndef INIT_WITH_PRIORITY
    pcb->sched_prior.priority = 0;
    #endif
    pcb->sched_prior.last_sched_time = init_ticks;
}

static void init_pid0_core_both(int mhartid){
    if(mhartid == 0){
        switchto_context_t *stored_switchto_k_m = (switchto_context_t *) pid0_pcb_core_m.kernel_sp;
        stored_switchto_k_m->regs[1] = pid0_pcb_core_m.kernel_sp;
        current_running_core_m = &pid0_pcb_core_m;
    }
    else{
        switchto_context_t *stored_switchto_k_s = (switchto_context_t *) pid0_pcb_core_s.kernel_sp;
        stored_switchto_k_s->regs[1] = pid0_pcb_core_s.kernel_sp;
        current_running_core_s = &pid0_pcb_core_s;
    }
}

static void init_pcb(int way)
{
    if(way == 0){
        init_list_head(&ready_queue);
        memset(pcb,0,sizeof(pcb));
        // page dir
        // load ELF
        init_pcb_block(&pcb[0],USER_PROCESS);
        ptr_t start_pos = (ptr_t)load_elf(_elf___test_test_shell_elf,_length___test_test_shell_elf,pcb[0].pgdir,alloc_page_helper_user);
        pcb[0].pid = 1;
        pcb[0].core_mask = 0b11;
        init_pcb_stack(pcb[0].kernel_sp,pcb[0].user_sp_useeable,start_pos,&pcb[0],0,NULL);
        list_add_tail(&(pcb[0].list),&ready_queue);
        // help initialize pid0
    }
    else{
        init_pcb_block(&bubble_pcb_m,USER_PROCESS);
        int elf_idx = match_elf("bubble");
        ptr_t start_pos = (ptr_t)load_elf(elf_files[elf_idx].file_content,*elf_files[elf_idx].file_length,bubble_pcb_m.pgdir,alloc_page_helper_user);
        bubble_pcb_m.pid = -1;
        bubble_pcb_m.core_mask = 0b11;
        init_pcb_stack(bubble_pcb_m.kernel_sp,bubble_pcb_m.user_sp_useeable,start_pos,&bubble_pcb_m,0,NULL);
        init_pcb_block(&bubble_pcb_s,USER_PROCESS);
        elf_idx = match_elf("bubble");
        start_pos = (ptr_t)load_elf(elf_files[elf_idx].file_content,*elf_files[elf_idx].file_length,bubble_pcb_s.pgdir,alloc_page_helper_user);
        bubble_pcb_s.pid = -1;
        bubble_pcb_s.core_mask = 0b11;
        init_pcb_stack(bubble_pcb_s.kernel_sp,bubble_pcb_s.user_sp_useeable,start_pos,&bubble_pcb_s,0,NULL);
    }
}

void setup_network()
{
    // network card
    uint32_t slcr_bade_addr = 0, ethernet_addr = 0;

    // get_prop_u32(_dtb, "/soc/slcr/reg", &slcr_bade_addr);
    slcr_bade_addr = sbi_read_fdt(SLCR_BADE_ADDR);
    printk("[slcr] phy: 0x%x\n\r", slcr_bade_addr);

    // get_prop_u32(_dtb, "/soc/ethernet/reg", &ethernet_addr);
    ethernet_addr = sbi_read_fdt(ETHERNET_ADDR);
    printk("[ethernet] phy: 0x%x\n\r", ethernet_addr);

    uint32_t plic_addr = 0;
    // get_prop_u32(_dtb, "/soc/interrupt-controller/reg", &plic_addr);
    plic_addr = sbi_read_fdt(PLIC_ADDR);
    printk("[plic] plic: 0x%x\n\r", plic_addr);

    uint32_t nr_irqs = sbi_read_fdt(NR_IRQS);
    // get_prop_u32(_dtb, "/soc/interrupt-controller/riscv,ndev", &nr_irqs);
    printk("[plic] nr_irqs: 0x%x\n\r", nr_irqs);

    XPS_SYS_CTRL_BASEADDR =
        (uintptr_t)ioremap((uint64_t)slcr_bade_addr, NORMAL_PAGE_SIZE);
    xemacps_config.BaseAddress =
    #ifdef QEMU
        (uintptr_t)ioremap((uint64_t)ethernet_addr, 9 * NORMAL_PAGE_SIZE);
        xemacps_config.BaseAddress = (uint64_t)xemacps_config.BaseAddress + 0x8000;
    #endif
    #ifndef QEMU
        (uintptr_t)ioremap((uint64_t)ethernet_addr, NORMAL_PAGE_SIZE);
    #endif
    uintptr_t _plic_addr =
        (uintptr_t)ioremap((uint64_t)plic_addr, 0x4000*NORMAL_PAGE_SIZE);
    // XPS_SYS_CTRL_BASEADDR = slcr_bade_addr;
    // xemacps_config.BaseAddress = ethernet_addr;
    xemacps_config.DeviceId        = 0;
    xemacps_config.IsCacheCoherent = 0;

    printk(
        "[slcr_bade_addr] phy:%x virt:%lx\n\r", slcr_bade_addr,
        XPS_SYS_CTRL_BASEADDR);
    printk(
        "[ethernet_addr] phy:%x virt:%lx\n\r", ethernet_addr,
        xemacps_config.BaseAddress);
    printk("[plic_addr] phy:%x virt:%lx\n\r", plic_addr, _plic_addr);
    plic_init(_plic_addr, nr_irqs);
    
    long status = EmacPsInit(&EmacPsInstance);
    if (status != XST_SUCCESS) {
        printk("Error: initialize ethernet driver failed!\n\r");
        assert(0);
    }

    // net_poll_mode = 1;
    // xemacps_example_main();
}

void boot_first_core(uintptr_t _dtb){
    smp_init(); // only done by master core
    lock_kernel();
    local_flush_tlb_all();
    init_pid0_core_both(0);
    current_running = &current_running_core_m;

    // init interrupt (^_^)
    init_exception();
    printk("\n\r> [INIT] Interrupt processing initialization succeeded.\n\r");
    
    // init system call table (0_0)
    init_syscall();
    printk("> [INIT] System call initialized successfully.\n\r");

    // read CPU frequency
    time_base = sbi_read_fdt(TIMEBASE);
    printk("time_base:%d\n\r",time_base);

    // network card
    setup_network();
    init_pcb(0);
    printk("> [INIT] PCB initialization succeeded.\n\r");

    // init screen (QAQ)
    init_screen();
    printk("> [INIT] SCREEN initialization succeeded.\n\r");

    // wake up slave
    wakeup_other_hart();

    printk("> [READY] Master core ready to launch!\n\r");

}

// jump from bootloader.
// The beginning of everything >_< ~~~~~~~~~~~~~~
int main(unsigned long mhartid, uintptr_t _dtb)
{
    // find current core

    // init Process Control Block (-_-!)
    if(mhartid == 0){
        boot_first_core(_dtb);
    }
    else{
        lock_kernel();
        cancel_direct_map(0x50000000);
        init_pcb(1);
        init_pid0_core_both(1);
        current_running = &current_running_core_s;
        setup_exception();
        printk("> [READY] Slave core ready to launch!\n\r");
        sbi_set_timer(get_ticks() + get_time_base()/TICKS_INTERVAL);
        k_schedule();
    }

    // TODO:
    // Setup timer interrupt and enable all interrupt

    #if defined (USE_CLOCK_INT)
    while(1){
        reset_irq_timer();
        __asm__ __volatile__("wfi\n\r");
    }
    #endif

    #if !defined (USE_CLOCK_INT)
    unlock_kernel();
    while(1){
        reset_irq_timer();
    }
    #endif
    return 0;
}


#include <os/list.h>
#include <os/mm.h>
#include <os/sched.h>
#include <pgtable.h>
#include <os/string.h>
#include <assert.h>
#include <os/smp.h>
#include <os/string.h>
#include <os/stdio.h>

#define STORE_TO_SD 0
#define LOAD_FROM_SD 1

ptr_t memCurr = FREEMEM;
ptr_t heapCurr = FREEHEAP;
int sd_block_id = SD_SWAP;
static LIST_HEAD(freePageList);
int freePage_num = 0;

typedef struct {
    uint8_t  valid;
    uint8_t  key;
    uint64_t paddr;
    uint64_t vaddr[16];
} shm_page_t;
shm_page_t spage[16];

void cancel_direct_map(uint64_t va)
{
    uint64_t pgdir = PGDIR_KVA;
    
    uint64_t vpn2 = 
        va >> (NORMAL_PAGE_SHIFT + PPN_BITS + PPN_BITS);
    uint64_t vpn1 = (vpn2 << PPN_BITS) ^
                    (va >> (NORMAL_PAGE_SHIFT + PPN_BITS));
    PTE *fst_pg = (PTE *)pgdir + vpn2;
    if(*fst_pg){
        PTE *snd_pte = (PTE *)pa2kva(get_pa(*fst_pg)) + vpn1;
        *fst_pg = 0;
        *snd_pte = 0;
    }
}

void swap_page_with_sd(uint64_t va, int op, int type){
    switch (op)
    {
    case STORE_TO_SD:{
        PTE *kva = (PTE *)va;
        uintptr_t paddr = get_pa(kva);
        // printk("va: %lx, kva: %lx, paddr: %lx, sd_block_id: %d\n\r",va,*kva,paddr,sd_block_id);
        sbi_sd_write(paddr, 8, sd_block_id);
        *kva = sd_block_id <<_PAGE_PFN_SHIFT;
        *kva &= ~_PAGE_VALID;
        sd_block_id += 8;
        break;
    }
    case LOAD_FROM_SD:{
        PTE *kva_old = (PTE *)va;
        int block_id = (*kva_old) >> _PAGE_PFN_SHIFT;
        uintptr_t kva_new = alloc_page_helper(va, (*current_running)->pgdir, 1, 1);
        // printk("va: %lx, old pte: %lx, new pte: %lx, block_id: %d\n\r",va,*kva_old,kva_new,block_id);
        sbi_sd_read(kva2pa(kva_new), 8, block_id);
        set_attribute(kva_new, _PAGE_ACCESSED | (type ? _PAGE_DIRTY : 0));
        *kva_old = kva_new;
    }
    default:
        break;
    }
}

ptr_t allocPage()
{
    if (freePage_num >= FREEPAGE_THRESHOLD)
    {
        page_t *new = list_entry(freePageList.next, page_t, list);
        uintptr_t kva = pa2kva(new->pa);
        memset((void *)kva,0,PAGE_SIZE);
        list_del(freePageList.next);
        freePage_num--;
        return kva;
    }else if(memCurr < TOP_USABLE_SPACE - WATER_MASK){
        memset((void *)memCurr,0,PAGE_SIZE);
        memCurr += PAGE_SIZE;
        return memCurr;
    } else{
        swap_page_helper(DELETE_PAGE_NUM_ONCE);
        allocPage();
    }
}

void freePage(ptr_t baseAddr)
{
    baseAddr &= (~0xfff);
    memset((void *)baseAddr,0,PAGE_SIZE);
    page_t *fp = (page_t *)kmalloc(sizeof(page_t));
    fp->pa = kva2pa(baseAddr);
    list_add_tail(&(fp->list), &freePageList);
    freePage_num++;
}

void *kmalloc(size_t size)
{
    ptr_t ret = ROUND(heapCurr, 4);
    heapCurr = ret + size;
    return (void*)ret;
}

uintptr_t shm_page_get(int key)
{
    current_running = (get_current_cpu_id() == 0) ? &current_running_core_m : &current_running_core_s;
    uint64_t begin_addr = 0xc00000;
    uint64_t kva;
    int i;
    // Find if exists
    for (i = 0; i < 16; i++)
    {
        if (spage[i].valid && (spage[i].key == key)){
            for (;; begin_addr += 0x1000)
            {
                if (check_page_helper(begin_addr, pa2kva((*current_running)->pgdir)) == 0)
                    break;
            }
            setup_shm_page(begin_addr, spage[i].paddr, pa2kva((*current_running)->pgdir));
            spage[i].vaddr[(*current_running)->pid] = begin_addr;
            return spage[i].vaddr[(*current_running)->pid];
        }
    }
    // Find a free pageframe
    for (;; begin_addr += 0x1000)
    {
        if ((kva = alloc_page_helper(begin_addr, pa2kva((*current_running)->pgdir),1 ,0)) != 0)
            break;
    }
    // Find a free spage control block
    for (i = 0; i < 16; i++)
    {
        if (!spage[i].valid)
            break;
    }
    // Init spage control block
    spage[i].valid = 1;
    spage[i].key = key;
    spage[i].paddr = kva2pa(kva);
    spage[i].vaddr[(*current_running)->pid] = begin_addr;
    return spage[i].vaddr[(*current_running)->pid];
}

void shm_page_dt(uintptr_t addr)
{
    current_running = (get_current_cpu_id() == 0) ? &current_running_core_m : &current_running_core_s;
    uint64_t vaddr = addr & 0xfffffffffffff000;
    int i;
    // Find spage cb
    for (i = 0; i < 16; i++)
    {
        if (spage[i].valid && vaddr == spage[i].vaddr[(*current_running)->pid])
            break;
    }
    // Invalid vaddr
    spage[i].vaddr[(*current_running)->pid] = 0;
    // Remove mirror
    free_page_helper(vaddr, (uintptr_t)pa2kva((*current_running)->pgdir));
    // Check valid
    int flag = 0;
    for (int j = 0; j < 16; j++)
    {
        if (spage[i].vaddr[j] != 0)
            flag = 1;
    }
    if (flag == 0)
    {
        spage[i].valid = 0;
        freePage(pa2kva(spage[i].paddr));
    }
}

/* this is used for mapping kernel virtual address into user page table */
void share_pgtable(uintptr_t dest_pgdir, uintptr_t src_pgdir)
{
    // TODO:
}

void setup_shm_page(uintptr_t va, uintptr_t pa, uintptr_t pgdir)
{
    uint64_t vpn[] = {(va >> 12) & ~(~0 << 9),
                      (va >> 21) & ~(~0 << 9),
                      (va >> 30) & ~(~0 << 9)};
    // Find PTE
    PTE *ptable[] = {(PTE *)pgdir + vpn[2],NULL,NULL};
    // Find level 2 pgdir
    if (((*ptable[2]) & _PAGE_VALID) == 0)
    {
        ptr_t newpage = allocPage() - PAGE_SIZE;
        *ptable[2] = (kva2pa(newpage) >> 12) << 10;
        set_attribute((PTE *)ptable[2], _PAGE_VALID);
        ptable[1] = (PTE *)newpage + vpn[1];
    }
    else
    {
        ptable[1] = (PTE *)pa2kva(((*ptable[2]) >> 10) << 12) + vpn[1];
    }
    // Find level 1 pgdir
    if (((*ptable[1]) & _PAGE_VALID) == 0)
    {
        ptr_t newpage = allocPage() - PAGE_SIZE;
        *ptable[1] = (kva2pa(newpage) >> 12) << 10;
        set_attribute((PTE *)ptable[1], _PAGE_VALID);
        ptable[0] = (PTE *)newpage + vpn[0];
    }
    else
    {
        ptable[0] = (PTE *)pa2kva(((*ptable[1]) >> 10) << 12) + vpn[0];
    }
    // Generate PTE
    // Set pfn
    set_pfn(ptable[0], pa >> 12);
    // Generate flags
    uint64_t pte_flags = _PAGE_READ     | _PAGE_WRITE    | _PAGE_EXEC 
                       | _PAGE_ACCESSED | _PAGE_VALID  | _PAGE_DIRTY 
                       | _PAGE_USER;
    set_attribute((PTE *)ptable[0], pte_flags);    
}
/* allocate physical page for `va`, mapping it into `pgdir`,
   return the kernel virtual address for the page.
   */
uintptr_t alloc_page_helper(uintptr_t va, uintptr_t pgdir, int mode, int ret_mode)
{
    uint64_t vpn_mask = ((((uint64_t)1) << 9) - 1);
    uint64_t vpn[] = {(va >> 12) & vpn_mask, (va >> 21) & vpn_mask, (va >> 30) & vpn_mask};
    // Find PTE
    PTE *ptable[] = {NULL,NULL,(PTE *)pgdir + vpn[2]};
    for(int i = 2; i > 0; i--){
        if (((*ptable[i]) & _PAGE_VALID) == 0)
        {
            ptr_t newpage = allocPage() - PAGE_SIZE;
            *ptable[i] = (kva2pa(newpage) >> 12) << 10;
            set_attribute((PTE *)ptable[i], (mode ? _PAGE_USER : 0) | _PAGE_VALID);
            ptable[i - 1] = (PTE *)newpage + vpn[i-1];
        }
        else
        {
            ptable[i - 1] = (PTE *)pa2kva((*ptable[i] >> 10) << 12) + vpn[i-1];
        }
    }
    // Generate PTE
    if ((*ptable[0] & _PAGE_VALID) != 0){
        return 0;
    }
    // Set pfn
    ptr_t newpage = allocPage() - PAGE_SIZE;
    set_pfn(ptable[0], (uint64_t)kva2pa(newpage) >> 12);
    // Generate flags
    uint64_t pte_flags = _PAGE_READ     | _PAGE_WRITE    | _PAGE_EXEC  | _PAGE_VALID  | (mode == 1 ? _PAGE_USER : 0);
    set_attribute((PTE *)ptable[0], pte_flags);
    if(ret_mode){
        return newpage;
    }
    return newpage + PAGE_SIZE;    
}

uintptr_t alloc_page_helper_user(uintptr_t kva, uintptr_t pgdir)
{
    return alloc_page_helper(kva, pgdir, 1, 1);
}

uintptr_t free_page_helper(uintptr_t va, uintptr_t pgdir)
{
    uint64_t vpn_mask = ((((uint64_t)1) << 9) - 1);
    uint64_t vpn[] = {(va >> 12) & vpn_mask, (va >> 21) & vpn_mask, (va >> 30) & vpn_mask};
    // Find PTE
    PTE *ptable[] = {(PTE *)pa2kva((*ptable[1] >> 10) << 12) + vpn[0],(PTE *)pa2kva((*ptable[2] >> 10) << 12) + vpn[1],(PTE *)pgdir + vpn[2]};
    freePage((*ptable[0] << 2) & (~0xfff));
    *ptable[0] = 0;
}

uintptr_t check_page_helper(uintptr_t va, uintptr_t pgdir)
{
    uint64_t vpn_mask = ((((uint64_t)1) << 9) - 1);
    uint64_t vpn[] = {(va >> 12) & vpn_mask, (va >> 21) & vpn_mask, (va >> 30) & vpn_mask};
    // Find PTE
    PTE *ptable[] = {NULL,NULL,(PTE *)pgdir + vpn[2]};
    if (((*ptable[2]) & _PAGE_VALID) == 0)
        return 0;
    ptable[1] = (PTE *)pa2kva((*ptable[2] >> 10) << 12) + vpn[1];
    if (((*ptable[1]) & _PAGE_VALID) == 0)
        return 0;
    ptable[0] = (PTE *)pa2kva((*ptable[1] >> 10) << 12) + vpn[0];
    if (((*ptable[0]) & _PAGE_VALID) == 0)
        return 0;
    return (uintptr_t)ptable[0];
}

uintptr_t search_last_page_helper(uintptr_t va, uintptr_t pgdir)
{
    uint64_t vpn_mask = ((((uint64_t)1) << 9) - 1);
    uint64_t vpn[] = {(va >> 12) & vpn_mask, (va >> 21) & vpn_mask, (va >> 30) & vpn_mask};
    // Find PTE
    PTE *ptable[] = {NULL,NULL,(PTE *)pgdir + vpn[2]};
    if (((*ptable[2]) & _PAGE_VALID) == 0)
        return 0;
    ptable[1] = (PTE *)pa2kva((*ptable[2] >> 10) << 12) + vpn[1];
    if (((*ptable[1]) & _PAGE_VALID) == 0)
        return 0;
    ptable[0] = (PTE *)pa2kva((*ptable[1] >> 10) << 12) + vpn[0];
    return (uintptr_t)ptable[0];
}

void swap_page_helper(int swap_page_num){
    // switch page with SD
    int pgnum = 0;
    list_node_t *curr = (*current_running)->u_plist.next->next;
    page_t *page_to_free = NULL;
    while (!list_is_empty(&(*current_running)->u_plist) && curr != &(*current_running)->u_plist && pgnum < swap_page_num)
    {
        page_to_free = list_entry(curr,page_t,list);
        swap_page_with_sd(page_to_free->va & (~0xfff),0,0);
        page_to_free->va &= (~0xfff);
        memset((void *)page_to_free->va,0,PAGE_SIZE);
        curr = curr->next;
        list_del(&page_to_free->list);
        list_add_tail(&page_to_free->list,&freePageList);
        pgnum++;
        (*current_running)->owned_page_num--;
    }
    if(pgnum == 0){
        curr = (*current_running)->k_plist.next->next;
        while (!list_is_empty(&(*current_running)->k_plist) && curr != &(*current_running)->k_plist && pgnum < swap_page_num)
        {
            page_to_free = list_entry(curr,page_t,list);
            swap_page_with_sd(page_to_free->va & (~0xfff),0,0);
            page_to_free->va &= (~0xfff);
            memset((void *)page_to_free->va,0,PAGE_SIZE);
            curr = curr->next;
            list_del(&page_to_free->list);
            list_add_tail(&page_to_free->list,&freePageList);
            pgnum++;
        }
    }
    if(pgnum == 0){
        printk("fail to swap with current pcb");
    }
}

void adjust_page_list_helper(uintptr_t kva, uintptr_t va){
    page_t *np = (page_t *)kmalloc(sizeof(page_t));
    np->pa = kva2pa(kva);
    np->va = va;
    list_add_tail(&(np->list), &((*current_running)->u_plist));
    (*current_running)->owned_page_num++;
}
#include <os/ioremap.h>
#include <os/mm.h>
#include <pgtable.h>
#include <type.h>

// maybe you can map it to IO_ADDR_START ?
static uintptr_t io_base = IO_ADDR_START;

void *ioremap(unsigned long phys_addr, unsigned long size)
{
    // map phys_addr to a virtual address
    // then return the virtual address
    uintptr_t pgdir = PGDIR_KVA;   
    int flag = 0;
    uint64_t vaddr = io_base;
    uint64_t va = io_base;
    while (size > 0)
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
                set_attribute((PTE *)ptable[i], _PAGE_VALID);
                ptable[i - 1] = (PTE *)newpage + vpn[i-1];
            }
            else
            {
                ptable[i - 1] = (PTE *)pa2kva((*ptable[i] >> 10) << 12) + vpn[i-1];
            }
        }
        // Generate PTE
        if ((*ptable[0] & _PAGE_VALID) != 0){
            break;
        }
        // Set pfn
        set_pfn(ptable[0], phys_addr >> 12);
        // Generate flags
        uint64_t pte_flags = _PAGE_READ     | _PAGE_WRITE    | _PAGE_EXEC 
                           | _PAGE_ACCESSED | _PAGE_VALID  | _PAGE_DIRTY;
        set_attribute((PTE *)ptable[0], pte_flags);
        io_base = (uint64_t)io_base + PAGE_SIZE;
        phys_addr = (uint64_t)phys_addr + PAGE_SIZE;
        va = io_base;
        size -= PAGE_SIZE;
    }
    local_flush_tlb_all();
    return (void *)vaddr;
}

void iounmap(void *io_addr)
{
    // TODO: a very naive iounmap() is OK
    // maybe no one would call this function?
    // *pte = 0;
    cancel_direct_map((uint64_t)io_addr);
}

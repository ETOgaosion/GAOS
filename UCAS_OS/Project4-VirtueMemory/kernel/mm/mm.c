#include <os/list.h>
#include <os/mm.h>
#include <os/sched.h>
#include <pgtable.h>
#include <os/string.h>
#include <assert.h>

int first_time_alloc = 1;
ptr_t memCurr[NUM_MAX_TASK*2] = {FREEMEM};

static LIST_HEAD(freePageList);

ptr_t allocPage(int numPage, int startPage)
{
    // align PAGE_SIZE
    if(first_time_alloc){
        for (int i = 0; i < 2*NUM_MAX_TASK; i++)
        {
            memCurr[i] = FREEMEM + i*PAGE_SIZE;
        }
        first_time_alloc = 0;
    }
    return memCurr[startPage] + PAGE_SIZE;
}

void* kmalloc(size_t size, int page)
{
    ptr_t ret = ROUND(memCurr[page], 4);
    memCurr[page] = ret + size;
    return (void*)ret;
}

void freePage(ptr_t baseAddr, int numPage){
    int page = (baseAddr - FREEMEM) / PAGE_SIZE;
    memCurr[page] = memCurr[page] - memCurr[page] % PAGE_SIZE;
    kmemset(memCurr[page],0,numPage * PAGE_SIZE);
}
uintptr_t shm_page_get(int key)
{
    // TODO(c-core):
}

void shm_page_dt(uintptr_t addr)
{
    // TODO(c-core):
}

/* this is used for mapping kernel virtual address into user page table */
void share_pgtable(uintptr_t dest_pgdir, uintptr_t src_pgdir)
{
    // TODO:
}

/* allocate physical page for `va`, mapping it into `pgdir`,
   return the kernel virtual address for the page.
   */
uintptr_t alloc_page_helper(uintptr_t va, uintptr_t pgdir)
{
    // TODO:
}

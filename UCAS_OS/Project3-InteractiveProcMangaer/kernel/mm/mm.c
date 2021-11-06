#include <os/mm.h>
#include <os/string.h>

int first_time_alloc = 1;
ptr_t memCurr[NUM_MAX_TASK*2] = {FREEMEM};

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
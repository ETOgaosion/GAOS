#ifndef MM_H
#define MM_H

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *            Copyright (C) 2018 Institute of Computing Technology, CAS
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *                                   Memory Management
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnisched to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * */

#include <type.h>
#include <os/list.h>

#define NUM_MAX_TASK 16
#define MEM_SIZE 32
#define PAGE_SIZE 4096 // 4K
#define INIT_KERNEL_STACK 0xffffffc050500000lu
#define TOP_USABLE_SPACE 0xffffffc05d000000lu
#define MAX_PAGE_NUM_PER_PCB 0x1000
#define PAGE_LIMIT 0x9500
#define WATER_MASK PAGE_SIZE * PAGE_LIMIT
#define DELETE_PAGE_NUM_ONCE 8
#define FREEPAGE_THRESHOLD DELETE_PAGE_NUM_ONCE
#define FREEMEM (INIT_KERNEL_STACK + 2 * PAGE_SIZE)
#define FREEHEAP 0xffffffc05d000000lu
#define USER_STACK_BIOS 0xf00010000lu
#define KERNEL_STACK_BIOS 0xffffffc000000000lu

/* Rounding; only works for n = power of two */
#define ROUND(a, n)     (((((uint64_t)(a))+(n)-1)) & ~((n)-1))
#define ROUNDDOWN(a, n) (((uint64_t)(a)) & ~((n)-1))

#define SD_SWAP 2048

extern ptr_t memCurr;
typedef struct {
    uintptr_t pa;
    uintptr_t va;
    list_node_t list;
} page_t;

void cancel_direct_map(uint64_t va);
void swap_page_with_sd(uint64_t va, int op, int type);
ptr_t allocPage();
void freePage(ptr_t baseAddr);
void* kmalloc(size_t size);
void share_pgtable(uintptr_t dest_pgdir, uintptr_t src_pgdir);
void setup_shm_page(uintptr_t va, uintptr_t pa, uintptr_t pgdir);
uintptr_t shm_page_get(int key);
void shm_page_dt(uintptr_t addr);
uintptr_t alloc_page_helper(uintptr_t va, uintptr_t pgdir, int mode, int ret_mode);
uintptr_t alloc_page_helper_user(uintptr_t kva, uintptr_t pgdir);
uintptr_t free_page_helper(uintptr_t va, uintptr_t pgdir);
uintptr_t check_page_helper(uintptr_t va, uintptr_t pgdir);
uintptr_t search_last_page_helper(uintptr_t va, uintptr_t pgdir);
void swap_page_helper(int swap_page_num);
void adjust_page_list_helper(uintptr_t kva, uintptr_t va);

#endif /* MM_H */

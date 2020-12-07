#pragma once

#include <types.h>

#define ALLOC_LOWMEM    BIT(0)
#define ALLOC_NOX64K    BIT(1)

void* kmalloc(size_t size);
void kfree(void* ptr);

/**
 * @brief page_init perform a memory scan and
 * initialize the free pages refcounter
 */
void _init page_init(boot_info_t* boot_info);

/**
 * @brief kheap_init initialize the kernel heap
 */
void _init kheap_init();

void kheap_check_corrupt();

/**
 * @brief page_alloc allocate a new physical
 * page. this function always succeeds
 * @param flags currently unused
 * @return the physical page frame number
 */
size_t page_alloc(int flags);

/**
 * @brief page_release release a physical page frame
 * and delete if no more references point to it
 * @param page
 */
void page_release(size_t page);

/**
 * @brief get_phys_pages get a region of physical memory
 * @param pages page count
 * @param flags PHYBUF_LOWMEM
 * @return
 */
void* get_phys_pages(size_t pages, int flags);

extern void* kheap_start_;
extern void* kheap_break_;

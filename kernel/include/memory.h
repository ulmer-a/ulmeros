#pragma once

#include <types.h>

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

extern void* kheap_start_;
extern void* kheap_break_;

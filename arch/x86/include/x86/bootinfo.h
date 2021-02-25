#pragma once

#include <util/types.h>

/*
 * the following struct of information is going
 * to be passed from the boot32 stage to the
 * 64bit kernel. it contains important pointers
 * and information that the kernel has to preserve.
 */
typedef struct
{
  uint64_t free_pages_ptr;
  uint64_t total_pages;
  uint64_t gdt_addr;
  uint64_t boot32_start_page;
  uint64_t boot32_page_count;
  uint64_t heap_addr;
  uint64_t kernel_addr;
  uint32_t heap_size;
  uint32_t kernel_size;
  uint64_t pml4_ppn;
  uint64_t cmdline_ptr;
} bootinfo_t;

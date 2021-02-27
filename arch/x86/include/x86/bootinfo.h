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
  uint64_t boot32_start_page;
  uint64_t boot32_page_count;
  uint64_t heap_start_page;
  uint64_t kernel_addr;
  uint64_t heap_pages;
  uint64_t kernel_size;
  uint64_t stack_start_page;
  uint64_t stack_pages;
  uint64_t pml4_ppn;
  uint64_t cmdline_ptr;         // pointer to the cmdline string
  uint64_t ramdisk_ptr;         // pointer to the initial ramdisk
  uint64_t ramdisk_size;        // size of the initial ramdisk
} bootinfo_t;

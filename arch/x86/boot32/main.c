#include <util/string.h>

#include "boot32.h"

static void clear_bss()
{
  size_t bss_length = (size_t)&_bss_end - (size_t)&_bss_start;
  memset(&_bss_start, 0, bss_length);
}

extern char _boot_start;
extern char _boot_end;

extern void* kheap_start_;
extern void* kheap_break_;

extern char* _binary_kernel_bin_start;
extern char* _binary_kernel_bin_size;

#define IDENT_OFFSET 0xffff800000000000ull

void boot32_main(multiboot_t* mb)
{
  clear_bss();

  /* setup a global descriptor table to avoid
   * that GRUB's GDT is overwritten. */
  gdt_init();

  /* perform a memory scan and setup a free page
   * bitmap, this will make alloc_page() work. */
  uint64_t bitmap_addr, total_pages;
  uint64_t highest_addr = create_mmap(mb->mmap, mb->mmap_length);
  create_pagemap(&bitmap_addr, &total_pages);

  /* setup identity mapping and long mode paging */
  size_t pml4 = paging_init(highest_addr >> PAGE_SHIFT);

  /* setup a long mode GDT */
  void* gdt_addr = gdt_long_init();

  bootinfo_t* boot_info = kmalloc(sizeof(bootinfo_t));
  boot_info->free_pages_ptr = bitmap_addr + IDENT_OFFSET;
  boot_info->total_pages = total_pages;
  boot_info->gdt_addr = (uint64_t)gdt_addr + IDENT_OFFSET;
  boot_info->boot32_start_page = (uint64_t)&_boot_start >> PAGE_SHIFT;
  boot_info->kernel_addr = KERNEL_LOAD_ADDR + IDENT_OFFSET;
  boot_info->boot32_page_count =
      (((uint64_t)&_boot_end - (uint64_t)&_boot_start) >> PAGE_SHIFT) + 1;
  boot_info->heap_addr = (uint64_t)kheap_start_ + IDENT_OFFSET;
  boot_info->heap_size = (uint64_t)kheap_break_ - (uint64_t)kheap_start_;
  boot_info->pml4_ppn = pml4;

  const size_t kernel_size = (size_t)&_binary_kernel_bin_size;
  boot_info->kernel_size = kernel_size;
  debug("loading kernel64 binary (%uk)... ", kernel_size >> 10);
  memcpy((void*)KERNEL_LOAD_ADDR,
         &_binary_kernel_bin_start, kernel_size);
  debug("done\n");

  debug("entering 64bit long mode\n");
  jmp_longmode(boot_info + IDENT_OFFSET);
}

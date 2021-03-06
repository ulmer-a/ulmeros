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

bootinfo_t boot_info;

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

  /* extract kernel command line and initrd
   * location from the multiboot structure */
  get_multiboot_info(mb);

  /* allocate the space required for boot32 code
   * and data structure as well as kernel64 code.
   * this will also load the kernel64 binary to
   * it's target location. */
  alloc_boot32_pages();

  /* setup identity mapping and long mode paging */
  size_t pml4 = paging_init(highest_addr >> PAGE_SHIFT);

  /* setup a long mode GDT */
  gdt_long_init();

  boot_info.free_pages_ptr = bitmap_addr + IDENT_OFFSET;
  boot_info.total_pages = total_pages;
  boot_info.pml4_ppn = pml4;

  debug("entering 64bit long mode\n");
  jmp_longmode(&boot_info);
}

#include <util/string.h>

#include "boot32.h"

static void clear_bss()
{
  size_t bss_length = (size_t)&_bss_end - (size_t)&_bss_start;
  memset(&_bss_start, 0, bss_length);
}

void boot32_main(multiboot_t* mb)
{
  clear_bss();

  gdt_init();

  uint64_t highest_addr = create_mmap(mb->mmap, mb->mmap_length);

  create_pagemap();

  paging_init(highest_addr >> PAGE_SHIFT);

  debug("virtual memory enabled\n");

  // jump to 64 bit space
}

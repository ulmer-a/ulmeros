#pragma once

#include <util/types.h>

typedef struct
{
  uint64_t free_pages_ptr;
  uint64_t gdt_addr;
  uint64_t boot32_start_page;
  uint64_t boot32_page_count;
  uint64_t heap_addr;
  uint64_t heap_size;
} bootinfo_t;

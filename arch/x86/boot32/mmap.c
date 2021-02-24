#include <util/types.h>
#include <util/list.h>
#include <util/bitmap.h>
#include <util/string.h>

#include "boot32.h"
#include "multiboot.h"

list_t _regions;
list_t* regions = &_regions;

typedef struct
{
  uint64_t addr;
  uint64_t size;
  int available;
} region_t;

static uint64_t phys_ram_size = 0;
static bitmap_t free_pages;

uint64_t create_mmap(multiboot_mmape_t* mmap, size_t length)
{
  list_init(regions);

  multiboot_mmape_t* end = (multiboot_mmape_t*)((char*)mmap + length);

  debug("scanning multiboot memory zones for usable memory\n");

  for (multiboot_mmape_t* entry = mmap;
       entry < end;
       entry = (multiboot_mmape_t*)((char*)entry + entry->entry_size + 4))
  {
    const uint64_t addr = entry->base_addr;
    const uint64_t size = entry->size;

    region_t* new_region = kmalloc(sizeof(region_t));
    new_region->addr = addr;
    new_region->size = size;
    new_region->available = entry->type == 1;
    list_add(regions, new_region);

    if (entry->type == 1 && addr + size > phys_ram_size)
      phys_ram_size = addr + size;
  }
  debug("highest usable memory address: %q\n", phys_ram_size - 1);
  return phys_ram_size;
}

void create_pagemap(uint64_t* addr)
{
  const size_t total_pages = phys_ram_size >> PAGE_SHIFT;
  bitmap_init(&free_pages, total_pages, 0xff);

  debug("-- usable memory zones:\n");
  uint64_t total_memory = 0;
  for (list_item_t* it = list_it_front(regions);
       it != LIST_IT_END;
       it = list_it_next(it))
  {
    region_t* region = list_it_get(it);

    if (!region->available)
      continue;

    size_t start_page = region->addr >> PAGE_SHIFT;
    if (region->addr % PAGE_SIZE != 0)
      start_page += 1;

    const uint64_t end_addr = region->addr + region->size;
    size_t end_page = end_addr >> PAGE_SHIFT;
    if (end_addr % PAGE_SIZE == 0)
      end_page -= 1;

    debug("mmap: %q: size=%lu KB\n", region->addr, region->size >> 10);
    for (size_t page = start_page; page <= end_page; page++)
      bitmap_clr(&free_pages, page);

    total_memory += region->size;
  }

  *addr = (uint64_t)&free_pages;
  debug("total usable memory: %lu MB\n", total_memory >> 20);
}

size_t alloc_page()
{
  size_t ppn = bitmap_find_free(&free_pages);
  bitmap_set(&free_pages, ppn);
  memset((void*)(ppn << PAGE_SHIFT), 0, PAGE_SIZE);
  return ppn;
}

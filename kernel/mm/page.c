#include <mm/memory.h>
#include <mm/vspace.h>
#include <util/bitmap.h>
#include <util/string.h>
#include <debug.h>
#include <init.h>

static bitmap_t free_pages;

void setup_page_bitmap(void* bitmap_addr, size_t size)
{
  free_pages.bitmap = bitmap_addr;
  free_pages.size = size;
}

size_t alloc_page()
{
  size_t ppn = bitmap_find_free(&free_pages);
  bitmap_set(&free_pages, ppn);
  memset(ppn_to_virt(ppn), 0, PAGE_SIZE);
  return ppn;
}

void free_page(size_t page)
{
  assert(page < free_pages.size, "page out of bounds");
  assert(bitmap_get(&free_pages, page) == 1, "double free page");
  bitmap_clr(&free_pages, page);
}

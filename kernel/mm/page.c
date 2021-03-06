#include <mm/memory.h>
#include <mm/vspace.h>
#include <util/bitmap.h>
#include <util/string.h>
#include <debug.h>

static bitmap_t free_pages;

void setup_page_bitmap(void* bitmap_addr, size_t size)
{
  free_pages.bitmap = bitmap_addr;
  free_pages.size = size;
}

size_t alloc_page()
{
  size_t ppn = bitmap_find_free(&free_pages);
  kpanic(ppn != (size_t)-1, "alloc_page() out of memory");
  bitmap_set(&free_pages, ppn);
  memset(ppn_to_virt(ppn), 0, PAGE_SIZE);
  return ppn;
}

void* alloc_dma_region()
{
  size_t dma_pages = 16;            // 16P = 64k
  size_t max_page = 1048576 - 1;    // 1MP = 4GB
  size_t ppn_start = bitmap_find_n_free(&free_pages, dma_pages);

  /* validate */
  if (ppn_start == (size_t)-1 || ppn_start + dma_pages > max_page)
  {
    kpanic(false, "No DMA region could be allocated!");
    return NULL;
  }

  /* mark used*/
  size_t page = ppn_start;
  while (dma_pages--)
    bitmap_set(&free_pages, page++);

  return (void*)(ppn_start << PAGE_SHIFT);
}

void free_page(size_t page)
{
  if (page >= free_pages.size)
  {
    assert(false, "page out of bounds");
    return;
  }
  assert(bitmap_get(&free_pages, page) == 1, "double free page");
  bitmap_clr(&free_pages, page);
}

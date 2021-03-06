#include "boot32.h"

extern char _boot_start;
extern char _boot_end;

extern void* kheap_start_;
extern void* kheap_break_;

extern char* _binary_kernel_bin_start;
extern char* _binary_kernel_bin_size;

static void mark_pages_used(size_t start_page, size_t count)
{
  while (count--)
    bitmap_set(&free_pages, start_page++);
}

void alloc_boot32_pages()
{
  /* pages that are used by boot32 stage code/data/BSS */
  const uint64_t boot_start_page = (uint64_t)&_boot_start >> PAGE_SHIFT;
  const uint64_t boot_page_count =
      (((uint64_t)&_boot_end - (uint64_t)&_boot_start) >> PAGE_SHIFT) + 1;
  mark_pages_used(boot_start_page, boot_page_count);

  /* pages that are used by boot32 stack */
  const uint64_t stack_start_page =
      (BOOT32_STACK_LOC >> PAGE_SHIFT) - BOOT32_STACK_PAGES;
  const uint64_t stack_page_count = BOOT32_STACK_PAGES;
  mark_pages_used(stack_start_page, stack_page_count);

  /* pages that are used by the boot32 heap */
  const size_t heap_start_page = (uint64_t)kheap_start_ >> PAGE_SHIFT;
  const size_t heap_end_page = ((uint64_t)kheap_break_ >> PAGE_SHIFT) + 1;
  mark_pages_used(heap_start_page, heap_end_page - heap_start_page);

  /* finally allocate the space used by the kernel64. */
  const size_t kernel_size = (size_t)&_binary_kernel_bin_size;
  debug("loading kernel64 binary (%uk)... ", kernel_size >> 10);
  memcpy((void*)KERNEL_LOAD_ADDR,
         &_binary_kernel_bin_start, kernel_size);
  debug("done\n");
  const size_t kernel_start_page = (uint64_t)KERNEL_LOAD_ADDR >> PAGE_SHIFT;
  const size_t kernel_page_count = (kernel_size >> PAGE_SHIFT) + 1;
  for (uint64_t page = kernel_start_page;
       page < (kernel_start_page + kernel_page_count); page++)
    bitmap_set(&free_pages, page);

  boot_info.boot32_start_page = boot_start_page;
  boot_info.boot32_page_count = boot_page_count;
  boot_info.heap_start_page = heap_start_page;
  boot_info.heap_pages = heap_end_page - heap_start_page;
  boot_info.kernel_addr = KERNEL_LOAD_ADDR + IDENT_OFFSET;
  boot_info.kernel_size = kernel_size;
  boot_info.stack_start_page = stack_start_page;
  boot_info.stack_pages = BOOT32_STACK_PAGES;
}

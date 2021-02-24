#include <util/types.h>
#include <util/bitmap.h>
#include <util/string.h>
#include <mm/memory.h>
#include <mm/vspace.h>
#include <init.h>
#include <debug.h>

#include <x86/bootinfo.h>

static void free_pages(size_t start, size_t n)
{
  for (size_t page = start; page < start + n; page++)
    free_page(page);
}

void amd64_main(bootinfo_t* boot_info)
{
  debug(INIT, "welcome to 64bit long mode!\n");
  debug(INIT, "kernel is at %p (size=%uk)\n",
        boot_info->kernel_addr, boot_info->kernel_size >> 10);

  /* since alloc_page() is needed to setup the heap,
   * it has to be setup first. */
  setup_page_bitmap((void*)boot_info->free_pages_ptr,
                    boot_info->total_pages);

  /* setup virtual memory. all the page table pages
   * are already marked used in the free pages bitmap
   * at this point. */
  vspace_setup(boot_info->pml4_ppn);

  /* copy the page bitmap into the new heap */
  const size_t bitmap_size = BITMAP_BYTE_SIZE(boot_info->total_pages);
  void* new_bitmap = kmalloc(bitmap_size);
  memcpy(new_bitmap, (void*)boot_info->free_pages_ptr, bitmap_size);
  setup_page_bitmap(new_bitmap, boot_info->total_pages);

  /* release boot32 resources */
  /*size_t old_heap_pages = boot_info->heap_size >> PAGE_SHIFT;
  if (boot_info->heap_size % PAGE_SIZE != 0)
    old_heap_pages += 1;
  free_pages(boot_info->heap_addr >> PAGE_SHIFT, old_heap_pages);
  free_pages(boot_info->boot32_start_page, boot_info->boot32_page_count);*/
}

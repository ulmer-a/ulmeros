#include <memory.h>
#include <types.h>
#include <definitions.h>
#include <vspace.h>

static void* kheap_start = ARCH_KHEAP_START;
static void* kheap_break = NULL;

void kheap_init()
{
  size_t phys_page = page_alloc(0);
  size_t heap_start_virt = (size_t)kheap_start / PAGE_SIZE;
  vspace_map(VSPACE_KERNEL, heap_start_virt, phys_page, PG_KERNEL);
}

void* kmalloc(size_t size)
{
  assert(kheap_break, "kmalloc(): uninitialized");
  return NULL;
}

void kfree(void *ptr)
{
  assert(kheap_break, "kfree(): uninitialized");

}

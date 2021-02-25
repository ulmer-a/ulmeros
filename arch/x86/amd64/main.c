#include <util/types.h>
#include <util/bitmap.h>
#include <util/string.h>
#include <mm/memory.h>
#include <mm/vspace.h>
#include <debug.h>

#include <x86/bootinfo.h>

/* the generic kernel main function */
extern void kmain(const char* cmdline);

/* initialize the free pages bitmap from an
 * existing memory location. */
void setup_page_bitmap(void* bitmap_addr, size_t size);

/* initialize a new 64bit GDT with TSS on the
 * kernel heap. */
extern void reload_gdt();

/* setup the kernel's virtual address space */
extern void vspace_setup(size_t pml4_ppn);

/* initialize interrupt controller, IDT and
 * install handlers. */
extern void x86_irq_init();

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

  /* reload the kernel command line */
  const char* cmdline = (const char*)boot_info->cmdline_ptr;
  cmdline = strcpy(kmalloc(strlen(cmdline) + 1), cmdline);

  /* copy the page bitmap into the new heap */
  const size_t bitmap_size = BITMAP_BYTE_SIZE(boot_info->total_pages);
  void* new_bitmap = kmalloc(bitmap_size);
  memcpy(new_bitmap, (void*)boot_info->free_pages_ptr, bitmap_size);
  setup_page_bitmap(new_bitmap, boot_info->total_pages);

  reload_gdt();

  x86_irq_init();

  kmain(cmdline);
}

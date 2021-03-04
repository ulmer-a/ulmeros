/*
 * UlmerOS x86_64 architecture initialization
 * Copyright (C) 2021 Alexander Ulmer
 *
 * After the boot32 stage has performed a far jump
 * to the 64bit kernel code, the 64 bit assembly startup
 * code of the 64 bit kernel will be executed. It will
 * setup a stack, enable SSE and then jump to the amd64_main()
 * function located in this file.
*/

#include <util/types.h>
#include <util/bitmap.h>
#include <util/string.h>
#include <arch/common.h>
#include <mm/memory.h>
#include <mm/vspace.h>
#include <x86/bootinfo.h>
#include <debug.h>

extern char _bss_start;
extern char _bss_end;

static size_t __init__ stack_start_page;
static size_t __init__ stack_page_count;

/* initialize the free pages bitmap from an
 * existing memory location. */
void setup_page_bitmap(void* bitmap_addr, size_t size);

/* initialize a new 64bit GDT with TSS on the
 * kernel heap. */
extern void setup_gdt();

/* setup the kernel's virtual address space */
extern void vspace_setup(size_t pml4_ppn);

/* initialize interrupt controller, IDT and
 * install handlers. */
extern void x86_irq_init();

static const char* copy_cmdline(bootinfo_t* boot_info)
{
  /* reload the kernel command line */
  const char* cmdline = (const char*)boot_info->cmdline_ptr;
  return strcpy(kmalloc(strlen(cmdline) + 1), cmdline);
}

static void copy_page_bitmap(bootinfo_t* boot_info)
{
  /* copy the page bitmap into the new heap */
  const size_t bitmap_size = BITMAP_BYTE_SIZE(boot_info->total_pages);
  void* new_bitmap = kmalloc(bitmap_size);
  memcpy(new_bitmap, (void*)boot_info->free_pages_ptr, bitmap_size);
  setup_page_bitmap(new_bitmap, boot_info->total_pages);
}

static void* copy_initrd(bootinfo_t* boot_info)
{
  if (!boot_info->ramdisk_ptr)
    return NULL;

  /* copy the initial ramdisk into the kernel64 heap */
  void* initrd = kmalloc(boot_info->ramdisk_size);
  memcpy(initrd, (void*)boot_info->ramdisk_ptr, boot_info->ramdisk_size);
  return initrd;
}

static void free_pages(size_t start, size_t count)
{
  while (count--)
    free_page(start++);
}

void delete_init_stack()
{
  debug(INIT, "deleting init stack\n");
  free_pages(stack_start_page, stack_page_count);
}

void amd64_main(bootinfo_t* boot_info)
{
  /* clear BSS segment */
  memset(&_bss_start, 0, (size_t)&_bss_end - (size_t)&_bss_start);

  debug(INIT, "welcome to 64bit long mode!\n");
  debug(INIT, "kernel is at %p (size=%uk)\n",
        boot_info->kernel_addr, boot_info->kernel_size >> 10);

  setup_gdt();

  /* since alloc_page() is needed for the heap to work,
   * it has to be setup first. this will use the free
   * pages bitmap located in the boot32 heap. */
  setup_page_bitmap((void*)boot_info->free_pages_ptr,
                    boot_info->total_pages);

  /* setup virtual memory. all the page table pages
   * are already marked used in the free pages bitmap
   * at this point. */
  vspace_setup(boot_info->pml4_ppn);

  /* copy any information that is going to be used
   * by the kernel and that is located in the boot32
   * heap over to the kernel64 heap. */
  const char* cmdline = copy_cmdline(boot_info);
  copy_page_bitmap(boot_info);
  const size_t initrd_size = boot_info->ramdisk_size;
  void* initrd = copy_initrd(boot_info);
  stack_start_page = boot_info->stack_start_page;
  stack_page_count = boot_info->stack_pages;

  /* at this point, all the data from the old heap is
   * already copied over to the kernel64 heap, so we
   * can free the pages used by boot32 code/data as well
   * as the boot32 heap. */
  free_pages(boot_info->boot32_start_page, boot_info->boot32_page_count);
  free_pages(boot_info->heap_start_page, boot_info->heap_pages);

  x86_irq_init();

  kmain(cmdline, initrd, initrd_size);
}

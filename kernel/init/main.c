#include <types.h>
#include <kstring.h>
#include <vspace.h>
#include <memory.h>
#include <interrupt.h>
#include <task.h>
#include <arch.h>

extern char _bss_start;
extern char _bss_end;

static void sysinit()
{
  debug(KMAIN, "Welcome from the first kernel task!\n");
}

void main64(boot_info_t* bootinfo)
{
  debug(KMAIN, "reached kmain()\n");

  /* clear BSS segment */
  memset(&_bss_start, 0, (size_t)&_bss_end - (size_t)&_bss_start);

  /* allocate the free pages refcounter and perform
   * a memory scan and mark the pages used by the
   * kernel and the boot32 heap as reserved. */
  page_init(bootinfo);

  /* read the kernel page tables and save pointers to
   * them. this will enable vspace_init() to map the
   * kernel above the user break. */
  vspace_init_kernel();

  /* initialize the interrupt handlers and the
   * system timer */
  irq_init();
  timer_reset(50);

  /* initialize the kernel heap. this will create a
   * mapping at the very top of the address space */
  kheap_init();

  create_ktask(sysinit);

  for (;;) __asm__ volatile("hlt");
}

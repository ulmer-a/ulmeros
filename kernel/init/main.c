#include <types.h>
#include <kstring.h>
#include <vspace.h>
#include <memory.h>

extern char _bss_start;
extern char _bss_end;

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

  /* initialize the kernel heap. this will create a
   * mapping at the very top of the address space */
  kheap_init();

  char* test = kmalloc(5000);
  strcpy(test, "Hello!");
  kfree(test);

  // start the scheduler as FAST AS POSSIBLE

  // perform memory init

  // get some debug output

  // take care of interrupts

  // start the scheduler

  // start the intitialization kernel thread
}
